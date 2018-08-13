/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    main.c
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
#include <unistd.h>

#include "vid_tpg.h"

static void usage(void)
{
	printf("Usage:\n"
		"  vid-tpg-config [-h height] [-w width] [-b bgpat] [-f fgpat]\n"
		"\n"
//		"    outpath -- Must be either \"lineout\" or \"headphones\"\n"
//		"    inpath -- Must be either \"linein\" or \"mic\"\n"
	);
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	int height = 720, width = 1280;
	vidtpg_t *tpg;
	char *devname;
	enum vidtpg_bgpat bgpat = BGPAT_VIDEOIN;
	enum vidtpg_fgpat fgpat = FGPAT_NOOVERLAY;

	while ((c = getopt(argc, argv, "h:w:b:f:d:")) != -1) {
		switch (c) {
		case 'h':
			height = atoi(optarg);
			break;

		case 'w':
			width = atoi(optarg);
			break;

		case 'b':
			bgpat = (enum vidtpg_bgpat)atoi(optarg);
			break;

		case 'f':
			fgpat = (enum vidtpg_fgpat)atoi(optarg);
			break;

		case 'd':
			devname = optarg;
			break;

		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			usage();
			return -1;
		}
	}

	tpg = malloc(sizeof(vidtpg_t));
	if (!tpg)
		return -9;

	ret = vidtpg_init(tpg, devname);
	if (ret < 0)
		goto out;

	vidtpg_set_format(tpg, height, width, COLORFMT_RGB);
	vidtpg_set_pattern(tpg, bgpat, fgpat);

out:
	close(tpg->fd);
	free(tpg);
	return ret;
}
