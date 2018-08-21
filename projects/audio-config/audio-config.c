/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	audio-config.c
 * @author	R. Bush
 * @email	bush@krtkl.com
 * @version	v1.0
 * @date	2018 April 10
 * @brief	piSmasher Audio Configuration Application
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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>

#include "i2c.h"

#include "tlv320aic3104.h"


static struct aic3x_dev codec;
static struct aic3x_cfg cfg = {
	.id = 0,
	.sl_addr = AIC3X_ADDR,
	.i2c_write = i2c_write_reg,
	.i2c_read = i2c_read_reg,
};

static void
usage(void)
{
	printf("Usage:\n"
		"  audio-config [OPTIONS]\n"
		"\n"
		"    -i SOURCE     - Set input to SOURCE\n"
		"    -o SINK       - Set output to SINK\n"
		"    -g GAIN       - Set gain to GAIN (0 - 127)\n"
		"\n"
		"Sources:\n"
		"    linein\n"
		"    mic\n"
		"\n"
		"Sinks:\n"
		"    lineout\n"
		"    headphones\n"
		"\n"
	);
}

static int
audio_init(void)
{
	int ret;

	ret = i2c_init(cfg.sl_addr);
	if (ret < 0) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		return ret;
	}

	ret = aic3x_init(&codec, &cfg);
	if (ret < 0) {

	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int c, ret;
	char *inpath, *outpath;
	uint8_t invol = 47;			/**< Default input volume */
	uint8_t outvol = 47;			/**< Default output volume */

	while ((c = getopt(argc, argv, "i:o:g:")) != -1) {
		switch (c) {
		case 'i':
			/* Set the ADC datapath */
			inpath = optarg;
			break;

		case 'o':
			/* Set the DAC datapath */
			outpath = optarg;
			break;

		case 'g':
			invol = outvol = atoi(optarg);
			break;

		case '?':
		default:
			printf("[ERROR]: Unknown character `-%c'\n", optopt);
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (outpath == NULL) {
		cfg.out_route = AIC3X_HP_OUT_STEREO;
	} else if (strcmp(outpath, "lineout") == 0) {
		cfg.out_route = AIC3X_LINE_OUT_STEREO;
	} else if (strcmp(outpath, "headphones") == 0) {
		cfg.out_route = AIC3X_HP_OUT_STEREO;
	} else {
		printf("[ERROR]: Invalid output path\n");
		usage();
		exit(EXIT_FAILURE);
	}

	if (inpath == NULL) {
		cfg.in_route = AIC3X_LINE_IN_STEREO;
	} else if (strcmp(inpath, "linein") == 0) {
		cfg.in_route = AIC3X_LINE_IN_STEREO;
	} else if (strcmp(inpath, "mic") == 0) {
		cfg.in_route = AIC3X_MIC_IN_MONO;
	} else {
		printf("[ERROR]: Invalid input path\n");
		usage();
		exit(EXIT_FAILURE);
	}

	cfg.in_gain = invol;
	cfg.out_gain = outvol;

	ret = audio_init();
	if (ret < 0) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}


