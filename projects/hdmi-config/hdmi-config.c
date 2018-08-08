/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	main.c
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
#include <unistd.h>

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

#define UNIT_I2C_ADDRESS_0 0x72
#define CEC_UNIT_I2C_ADDRESS_0 0x36

static void
usage(void)
{
	printf("hdmi-config [-i input_format] [-o output_format]\n");
}

static int
hdmi_init(enum tda998x_vid_fmt vin_fmt, enum tda998x_vid_fmt vout_fmt)
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
	err = hdmi_tx_init(vin_fmt, vout_fmt);
	if (err) {
		ERROR_PRINT("%x = hdmi_tx_init()", err);
		return err;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	int c, err;
	enum tda998x_vid_fmt vin_fmt = VFMT_PC_1366x768p_60Hz;
	enum tda998x_vid_fmt vout_fmt = VFMT_PC_1366x768p_60Hz;

	printf("--------------------------------------------------------\n");
	printf("piSmasher HDMI Configuration and Test Utility\n");

	printf("Compiled: %s %s\n\n", __DATE__, __TIME__);

	while ((c = getopt(argc, argv, "i:o:")) != -1) {
		switch (c) {
		case 'i':
			vin_fmt = (enum tda998x_vid_fmt) atoi(optarg);
			break;

		case 'o':
			vout_fmt = (enum tda998x_vid_fmt) atoi(optarg);
			break;

		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			usage();
			return -1;
		}
	}

	printf("--- Initializing HDMI interfaces\n");
	err = hdmi_init(vin_fmt, vout_fmt);
	if (err < 0)
		ERROR_PRINT("%x = hdmi_init()", err);

	exit(err);
}
