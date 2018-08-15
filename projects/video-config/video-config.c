/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	video-config.c
 * @author	R. Bush
 * @email	bush@krtkl.com
 * @version	v1.0
 * @date	2018 April 10
 * @brief	piSmasher Video Configuration Application
 * @license	FreeBSD
 *
 *******************************************************************************
 *
 * Copyright (c) 2018, krtkl inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *	  this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "i2c.h"

#include "hdmi_tx.h"
#include "hdmi_rx.h"
#include "vtc.h"
#include "vid_tpg.h"

#define ERROR_PRINT(__format__, ...)		fprintf(stderr, "[ERROR] %s() %s %d: %s - " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, strerror(errno), ##__VA_ARGS__)
#define INFO_PRINT(__format__, ...)		printf("[INFO]: " __format__ "\r\n", ##__VA_ARGS__)

#ifdef DEBUG
# define DEBUG_PRINT(__format__, ...)		printf("[DEBUG] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DEBUG_PRINT(...)			do {} while (0)
#endif

#define UNIT_I2C_ADDRESS_0			(0x72)
#define CEC_UNIT_I2C_ADDRESS_0			(0x36)

static void
usage(void)
{
	printf("Usage: video-config [OPTIONS]\n"
		"\n"
		"Options:\n"
		"    -m MODE   - Read EDID from transmitter (default)\n"
		"\n"
	);
}

static const struct video_mode {
	enum tda998x_vid_fmt fmt;
	enum vtc_mode mode;
	int width;
	int height;
} video_modes[] = {
	{ VFMT_04_1280x720p_60Hz, VTC_MODE_720p, 1260, 720 },
	{ VFMT_16_1920x1080p_60Hz, VTC_MODE_1080p, 1920, 1080 },
	{ VFMT_PC_1366x768p_60Hz, VTC_MODE_WXGA, 1366, 768 },
	{ /* Sentinel */ }
};

static const struct vid_mode_str {
	const char *str;
	const struct video_mode *mode;
} video_mode_str[] = {
	{ "720p", &video_modes[0] },
	{ "1080p", &video_modes[1] },
	{ "WXGA", &video_modes[2] },
	{ /* Sentinel */ }
};

static const struct video_mode *
str2mode(const char *str)
{
	const struct vid_mode_str *list = &video_mode_str[0];

	while (list && list->str) {
		if (strcmp(str, list->str) == 0)
			return list->mode;

		list++;
	}

	return NULL;
}

static int
hdmi_init(enum tda998x_vid_fmt vin_fmt, enum tda998x_vid_fmt vout_fmt)
{
	int ret;

	INFO_PRINT("Initializing TDA1997x HDMI receiver");
	ret = hdmi_rx_init();
	if (ret) {
		ERROR_PRINT("%d = hdmi_rx_init()", ret);
		return ret;
	}

	printf("done.\n");

	INFO_PRINT("Initializing TD998x HDMI transmitter");
	ret = hdmi_tx_init(vin_fmt, vout_fmt);
	if (ret) {
		ERROR_PRINT("%x = hdmi_tx_init()", ret);
		return ret;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	struct vidtpg *tpg;
	struct vtc_dev *vtc;
	const struct video_mode *mode;
	enum vidtpg_bgpat bgpat = BGPAT_COLORBARS;

	while ((c = getopt(argc, argv, "m:")) != -1) {
		switch (c) {
		case 'm':
			mode = str2mode(optarg);
			if (mode == NULL) {
				printf("Unknown mode option %s\n", optarg);
				usage();
				return EXIT_FAILURE;
			}
			break;

		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			usage();
			return -1;
		}
	}

	/**
	 * Initialize HDMI input/output
	 */
	ret = hdmi_init(mode->fmt, mode->fmt);
	if (ret < 0) {
		ERROR_PRINT("initializing HDMI (%d)", ret);
		goto out_hdmi;
	}

	/**
	 * Initialize video timing controller
	 */
	vtc = malloc(sizeof (struct vtc_dev));
	if (vtc == NULL) {
		ERROR_PRINT("allocating VTC data");
		goto out_hdmi;
	}

	ret = vtc_init(vtc, "/dev/uio3");
	if (ret < 0) {
		ERROR_PRINT("initializing VTC (%d)", ret);
		goto out_vtc;
	}

	ret = vtc_set_generator_video_mode(vtc, mode->mode);
	if (ret < 0) {
		ERROR_PRINT("setting VTC generator mode (%d)", ret);
		goto out_vtc;
	}

	ret = vtc_gen_enable(vtc, true);
	if (ret < 0) {
		ERROR_PRINT("enabling VTC generator (%d)", ret);
		goto out_vtc;
	}

	/**
	 * Initialize the test pattern generator
	 */
	tpg = malloc(sizeof (struct vidtpg));
	if (tpg == NULL) {
		ERROR_PRINT("allocating TPG data");
		goto out_vtc;
	}

	ret = vidtpg_init(tpg, "/dev/uio2");
	if (ret < 0) {
		ERROR_PRINT("initializing TPG (%d)", ret);
		goto out;
	}

	ret = vidtpg_set_format(tpg, mode->height, mode->width, COLORFMT_RGB);
	if (ret < 0) {
		ERROR_PRINT("setting TPG format (%d)", ret);
		goto out;
	}

	ret = vidtpg_set_pattern(tpg, bgpat, FGPAT_NOOVERLAY);
	if (ret < 0) {
		ERROR_PRINT("setting TPG pattern (%d)", ret);
		goto out;
	}
out:
	close(tpg->fd);
	free(tpg);

out_vtc:
	close(vtc->fd);
	free(vtc);

out_hdmi:
	exit(ret);
}
