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
		"    -d GAIN       - Set DAC analog output gain to GAIN\n"
		"    -p GAIN       - Set DAC PGA analog output gain to GAIN\n"
		"    -D GAIN       - Set DAC digital volume control to GAIN\n"
		"    -A GAIN       - Set ADC PGA gain setting\n"
		"\n"
		"Sources:\n"
		"    linein\n"
		"    mic\n"
		"\n"
		"Sinks:\n"
		"    lineout\n"
		"    headphones\n"
		"\n"
		"Gain Settings:\n"
		"    Setting  |  Analog Gain (dB)  |  Setting  |  Analog Gain (dB)  \n"
		"-------------|--------------------|-----------|--------------------\n"
		"       0     |          0         |    60     |      -30.1\n"
		"       1     |        -0.5        |    61     |      -30.6\n"
		"       2     |         -1         |    62     |      -31.1\n"
		"       3     |        -1.5        |    63     |      -31.6\n"
		"       4     |         -2         |    64     |      -32.1\n"
		"       5     |        -2.5        |    65     |      -32.6\n"
		"       6     |         -3         |    66     |      -33.1\n"
		"       7     |        -3.5        |    67     |      -33.6\n"
		"       8     |         -4         |    68     |      -34.1\n"
		"       9     |        -4.5        |    69     |      -34.6\n"
		"      10     |         -5         |    70     |      -35.1\n"
		"      11     |        -5.5        |    71     |      -35.7\n"
		"      12     |         -6         |    72     |      -36.1\n"
		"      13     |        -6.5        |    73     |      -36.7\n"
		"      14     |         -7         |    74     |      -37.1\n"
		"      15     |        -7.5        |    75     |      -37.7\n"
		"      16     |         -8         |    76     |      -38.2\n"
		"      17     |        -8.5        |    77     |      -38.7\n"
		"      18     |         -9         |    78     |      -39.2\n"
		"      19     |        -9.5        |    79     |      -39.7\n"
		"      20     |        -10         |    80     |      -40.2\n"
		"      21     |       -10.5        |    81     |      -40.7\n"
		"      22     |        -11         |    82     |      -41.2\n"
		"      23     |       -11.5        |    83     |      -41.7\n"
		"      24     |        -12         |    84     |      -42.2\n"
		"      25     |       -12.5        |    85     |      -42.7\n"
		"      26     |        -13         |    86     |      -43.2\n"
		"      27     |       -13.5        |    87     |      -43.8\n"
		"      28     |        -14         |    88     |      -44.3\n"
		"      29     |       -14.5        |    89     |      -44.8\n"
		"      30     |        -15         |    90     |      -45.2\n"
		"      31     |       -15.5        |    91     |      -45.8\n"
		"      32     |        -16         |    92     |      -46.2\n"
		"      33     |       -16.5        |    93     |      -46.7\n"
		"      34     |        -17         |    94     |      -47.4\n"
		"      35     |       -17.5        |    95     |      -47.9\n"
		"      36     |        -18         |    96     |      -48.2\n"
		"      37     |       -18.6        |    97     |      -48.7\n"
		"      38     |       -19.1        |    98     |      -49.3\n"
		"      39     |       -19.6        |    99     |       -50\n"
		"      40     |       -20.1        |    100    |      -50.3\n"
		"      41     |       -20.6        |    101    |       -51\n"
		"      42     |       -21.1        |    102    |      -51.4\n"
		"      43     |       -21.6        |    103    |      -51.8\n"
		"      44     |       -22.1        |    104    |      -52.2\n"
		"      45     |       -22.6        |    105    |      -52.7\n"
		"      46     |       -23.1        |    106    |      -53.7\n"
		"      47     |       -23.6        |    107    |      -54.2\n"
		"      48     |       -24.1        |    108    |      -55.3\n"
		"      49     |       -24.6        |    109    |      -56.7\n"
		"      50     |       -25.1        |    110    |      -58.3\n"
		"      51     |       -25.6        |    111    |      -60.2\n"
		"      52     |       -26.1        |    112    |      -62.7\n"
		"      53     |       -26.6        |    113    |      -64.3\n"
		"      54     |       -27.1        |    114    |      -66.2\n"
		"      55     |       -27.6        |    115    |      -68.7\n"
		"      56     |       -28.1        |    116    |      -72.2\n"
		"      57     |       -28.6        |    117    |      -78.3\n"
		"      58     |       -29.1        |  118-127  |      Mute\n"
		"      59     |       -29.6        |           |\n"
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

	struct aic3x_gain *gains = malloc(sizeof(struct aic3x_gain));
	if (!gains)
		exit(EXIT_FAILURE);

	gains->ladc_pga = 0;		/**< 0dB */
	gains->radc_pga = 0;		/**< 0dB */
	gains->ldac_dvc = 0;
	gains->rdac_dvc = 0;
	gains->ldac_vol = 0;
	gains->rdac_vol = 0;
	gains->lpga_vol = 0;
	gains->rpga_vol = 0;

	while ((c = getopt(argc, argv, "i:o:d:p:D:A:")) != -1) {
		switch (c) {
		case 'i':
			/* Set the ADC datapath */
			inpath = optarg;
			break;

		case 'o':
			/* Set the DAC datapath */
			outpath = optarg;
			break;

		case 'd':
			gains->ldac_vol = atoi(optarg);
			gains->rdac_vol = atoi(optarg);
			break;

		case 'p':
			gains->lpga_vol = atoi(optarg);
			gains->rpga_vol = atoi(optarg);
			break;

		case 'D':
			gains->ldac_dvc = atoi(optarg);
			gains->rdac_dvc = atoi(optarg);
			break;

		case 'A':
			gains->ladc_pga = atoi(optarg);
			gains->radc_pga = atoi(optarg);
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

	cfg.gains = gains;

	ret = audio_init();
	if (ret < 0) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}


