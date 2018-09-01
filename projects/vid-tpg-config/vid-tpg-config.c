/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vid-tpg-config.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    July 10, 2018
 * @brief   Video Test Pattern Generator Userspace Control
 * @license FreeBSD
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
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "vid_tpg.h"

#define ERROR_PRINT(__format__, ...)		fprintf(stderr, "[ERROR] %s() %s %d: %s - " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, strerror(errno), ##__VA_ARGS__)
#define INFO_PRINT(__format__, ...)		printf("[INFO]: " __format__ "\r\n", ##__VA_ARGS__)

#ifdef DEBUG
# define DEBUG_PRINT(__format__, ...)		printf("[DEBUG] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DEBUG_PRINT(...)			do {} while (0)
#endif

static void
usage(void)
{
	printf("Usage:\n"
		"  vid-tpg-config [OPTIONS] UIONUM\n"
		"\n"
		"Options:\n"
		"    -h HEIGHT - Set active video height to HEIGHT\n"
		"    -w WIDTH  - Set active video width to WIDTH\n"
		"    -b PAT    - Set background pattern ID to PAT\n"
		"\n"
	);
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	int height = 768, width = 1366;
	struct vidtpg *tpg;
	int devnum = -1;
	char devname[16];
	enum vidtpg_bgpat bgpat = BGPAT_COLORBARS;
	enum vidtpg_fgpat fgpat = FGPAT_NOOVERLAY;

	while ((c = getopt(argc, argv, "h:w:b:")) != -1) {
		switch (c) {
		case 'h':
			height = atoi(optarg);
			break;

		case 'w':
			width = atoi(optarg);
			break;

		case 'b':
			bgpat = (enum vidtpg_bgpat) atoi(optarg);
			break;

		case 'f':
			fgpat = (enum vidtpg_fgpat) atoi(optarg);
			break;

		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			usage();
			return -1;
		}
	}

	/* Get the UIO number after parsing the options */
	if (optind < argc) {
		devnum = atoi(argv[optind]);
		sprintf(devname, "/dev/uio%d", devnum);
	} else {
		usage();
		return -1;
	}

	/**
	 * Initialize the test pattern generator
	 */
	tpg = malloc(sizeof (struct vidtpg));
	if (tpg == NULL) {
		ERROR_PRINT("allocating TPG data");
		goto out;
	}

	ret = vidtpg_init(tpg, devname);
	if (ret < 0) {
		ERROR_PRINT("initializing TPG (%d)", ret);
		goto out;
	}

	ret = vidtpg_set_format(tpg, height, width, COLORFMT_RGB);
	if (ret < 0) {
		ERROR_PRINT("setting TPG format (%d)", ret);
		goto out;
	}

	ret = vidtpg_set_pattern(tpg, bgpat, fgpat);
	if (ret < 0) {
		ERROR_PRINT("setting TPG pattern (%d)", ret);
		goto out;
	}
out:
	close(tpg->fd);
	free(tpg);
	return ret;
}
