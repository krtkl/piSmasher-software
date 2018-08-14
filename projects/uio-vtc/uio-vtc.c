/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    uio-vtc.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    August 6, 2018
 * @brief   Video Timing Controller Userspace Control
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

#include "vtc.h"


static void
usage(void)
{
	printf("Usage: uio-vtc [OPTIONS] UIONUM\n"
		"\n"
		"Options:\n"
		"    -g        - Configure generator (default)\n"
		"    -m MODE   - Configure generator for format MODE\n"
		"    -d        - Configure detector\n"
		"\n"
		"Modes:\n"
		"    720p\n"
		"    1080p\n"
		"    480p\n"
		"    576p\n"
		"    VGA\n"
		"    SVGA\n"
		"    XGA\n"
		"    SXGA\n"
		"    WXGA\n"
	);
}

const struct vtc_mode_str {
	const char *str;
	enum vtc_mode mode;
} modestr[] = {
	{ "720p", VTC_MODE_720p },
	{ "1080p", VTC_MODE_1080p },
	{ "480p", VTC_MODE_480p },
	{ "576p", VTC_MODE_576p },
	{ "VGA", VTC_MODE_VGA },
	{ "SVGA", VTC_MODE_SVGA },
	{ "XGA", VTC_MODE_XGA },
	{ "SXGA", VTC_MODE_SXGA },
	{ "WXGA", VTC_MODE_WXGA },
	{ /* Sentinel */ }
};

static int
str2mode(const char *str, enum vtc_mode *mode)
{
	const struct vtc_mode_str *list = &modestr[0];

	while (list && list->str[0]) {

		printf("Comparing %s and %s\n", list->str, str);

		if (strcmp(str, list->str) == 0) {

			printf("Mode found %s (%d)\n", list->str, list->mode);

			*mode = list->mode;
			return 0;
		}

		list++;
	}

	printf("Mode not found\n");

	return -1;
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	bool isgen = true;
	int devnum = -1;
	char devname[16];
	enum vtc_mode mode = VTC_MODE_WXGA;
	struct vtc_dev *vtc;

	while ((c = getopt(argc, argv, "gdm:")) != -1) {
		switch (c) {
		case 'd':
			isgen = false;
			break;

		case 'g':
			isgen = true;
			break;

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

	vtc = malloc(sizeof(struct vtc_dev));
	if (vtc == NULL)
		return EXIT_FAILURE;

	ret = vtc_init(vtc, devname);
	if (ret < 0)
		goto out;

	ret = vtc_enable_interrupts(vtc);
	if (ret < 0)
		goto out;

	if (isgen) {
		ret = vtc_set_generator_video_mode(vtc, mode);
		if (ret < 0)
			goto out;

		ret = vtc_gen_enable(vtc, true);
		if (ret < 0)
			goto out;
	} else {
		ret = vtc_det_enable(vtc, true);
		if (ret < 0)
			goto out;

		ret = vtc_det_dump(vtc);
		if (ret < 0)
			goto out;
	}
out:
	free(vtc);
	return ret;
}
