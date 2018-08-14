/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	hdmi-edid.c
 * @author	R. Bush
 * @email	bush@krtkl.com
 * @version	v1.0
 * @date	2018 April 10
 * @brief	HDMI Configuration Application
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

#define ERROR_PRINT(__format__, ...)		printf("[ERROR] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)

#ifdef DEBUG_INFO
# define INFO_PRINT(__format__, ...)		printf("[INFO] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define INFO_PRINT(...)					do {} while (0)
#endif

#ifdef DEBUG
# define DEBUG_PRINT(__format__, ...)		printf("[DEBUG] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DEBUG_PRINT(...)			do {} while (0)
#endif

#define UNIT_I2C_ADDRESS_0			(0x72)
#define CEC_UNIT_I2C_ADDRESS_0			(0x36)

static uint8_t edid_data[EDID_BLOCK_SIZE];		/**< EDID data for first block */
static uint8_t edid_ext[EDID_BLOCK_SIZE];		/**< EDID data for extension block */

static void
usage(void)
{
	printf("Usage: hdmi-edid [OPTIONS] FILE\n"
		"\n"
		"Options:\n"
		"    -r        - Read EDID from transmitter (default)\n"
		"    -w        - Write EDID to receiver\n"
	);
}

static int
hdmi_init(void)
{
	int err;

	printf("%-30s", "Initializing TDA1997x HDMI receiver\n");
	err = hdmi_rx_init();
	if (err) {
		ERROR_PRINT("%d = hdmi_rx_init()", err);
		return err;
	}

	printf("done.\n");

	printf("%-30s", "Initializing TD998x HDMI transmitter\n");
	err = hdmi_tx_init();
	if (err) {
		ERROR_PRINT("%x = hdmi_tx_init()", err);
		return err;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	int i, c, fd, ret;
	off_t fsize;
	char *fname = NULL;
	bool read_edid = true;

	printf("--------------------------------------------------------\n");
	printf("piSmasher HDMI EDID Read/Write Test Utility\n");
	printf("Compiled: %s %s\n\n", __DATE__, __TIME__);

	while ((c = getopt(argc, argv, "rw")) != -1) {
		switch (c) {
		case 'r':
			read_edid = true;
			break;

		case 'w':
			read_edid = false;
			break;

		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			usage();
			return -1;
		}
	}

	/* Get the filename after parsing the options */
	if (optind < argc) {
		fname = argv[optind];
	} else {
		usage();
		return -1;
	}

	printf("--- Initializing HDMI interfaces\n");
	ret = hdmi_init();
	if (ret < 0)
		ERROR_PRINT("%d = hdmi_init()", ret);

	if (read_edid) {
		ret = hdmi_tx_dump_edid(edid_data, edid_ext);
		if (ret < 0) {
			ERROR_PRINT("%d = hdmi_tx_dump_edid()", ret);
			exit(ret);
		}

		fd = open(fname, O_CREAT | O_RDWR | O_TRUNC, 0666);
		if (fd < 0) {
			ERROR_PRINT("%s", strerror(errno));
			exit(ret);
		}

		write(fd, edid_data, 128);
		write(fd, edid_ext, 128);

		close(fd);
	} else {
		ret = access(fname, F_OK);
		if (ret < 0) {
			ERROR_PRINT("file %s does not exist", fname);
			exit(ret);
		}

		fd = open(fname, O_RDONLY);
		if (fd < 0) {
			ERROR_PRINT("opening file %s: %s", fname, strerror(errno));
			exit(fd);
		}

		/* Check file size */
		fsize = lseek(fd, 0, SEEK_END);
		if (fsize != 256) {
			ERROR_PRINT("file size should be 256 bytes, received %ld", fsize);
			close(fd);
			exit(EXIT_FAILURE);
		}

		lseek(fd, 0, SEEK_SET);

		read(fd, edid_data, 128);
		read(fd, edid_ext, 128);

		close(fd);

		ret = hdmi_rx_set_edid(edid_data, edid_ext);
		if (ret < 0)
			ERROR_PRINT("%d = hdmi_rx_set_edid()", ret);
	}

	exit(EXIT_SUCCESS);
}
