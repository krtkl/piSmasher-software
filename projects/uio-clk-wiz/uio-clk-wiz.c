/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    uio-clk-wiz.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    August 6, 2018
 * @brief   Clocking Wizard Userspace Control
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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "clk_wiz.h"


static void
usage(void)
{
	printf("Usage: uio-clk-wiz [OPTIONS] UIONUM\n"
		"\n"
		"Options:\n"
		"    -m MODE   - Configure generator for format MODE\n"
		"\n"
		"Modes:\n"
		"    720p\n"
		"    1080p\n"
		"    WXGA\n"
		"\n"
	);
}

const struct clk_wiz_mode_str {
	const char *str;
	enum clk_wiz_mode mode;
} modestr[] = {
	{ "720p", CLK_WIZ_MODE_720p },
	{ "1080p", CLK_WIZ_MODE_1080p },
	{ "WXGA", CLK_WIZ_MODE_WXGA },
	{ /* Sentinel */ }
};

static int
str2mode(const char *str, enum clk_wiz_mode *mode)
{
	const struct clk_wiz_mode_str *list = &modestr[0];

	while (list && list->str[0]) {
		if (strcmp(str, list->str) == 0) {
			*mode = list->mode;
			return 0;
		}

		list++;
	}

	return -1;
}


int
main(int argc, char *argv[])
{
	int c, ret;
	int devnum = -1;
	enum clk_wiz_mode mode;
	struct clk_wiz_dev *clk_wiz;
	char devname[16];

	while ((c = getopt(argc, argv, "m:")) != -1) {
		switch (c) {
		case 'm':
			ret = str2mode(optarg, &mode);
			if (ret < 0) {
				printf("Unknown mode option %s\n", optarg);
				usage();
				return -1;
			}
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

	clk_wiz = malloc(sizeof (struct clk_wiz_dev));
	if (clk_wiz == NULL)
		return EXIT_FAILURE;

	ret = clk_wiz_init(clk_wiz, devname);
	if (ret < 0)
		goto out;

	ret = clk_wiz_config(clk_wiz, mode);
out:
	free(clk_wiz);
	return ret;
}
