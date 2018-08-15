/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vid_tpg.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    July 10, 2018
 * @brief   Video Test Pattern Generator Userspace I/O Driver
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


#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#include "vid_tpg.h"

#define VIDTPG_REG_CONTROL		(0x00000000U)	/**< Control */
#define CONTROL_AP_START		(1 << 0)
#define CONTROL_AP_DONE			(1 << 1)
#define CONTROL_AP_IDLE			(1 << 2)
#define CONTROL_AP_READY		(1 << 3)
#define CONTROL_AUTO_RESTART		(1 << 7)

#define VIDTPG_REG_GINTEN		(0x00000004U)	/**< Global interrupt enable */
#define VIDTPG_REG_IPINTEN		(0x00000008U)	/**< IP interrupt enable */
#define VIDTPG_REG_ACTHEIGHT		(0x00000010U)	/**< Active height */
#define VIDTPG_REG_ACTWIDTH		(0x00000018U)	/**< Active width */
#define VIDTPG_REG_BGPAT_ID		(0x00000020U)	/**< Background pattern ID */
#define VIDTPG_REG_FGPAT_ID		(0x00000028U)	/**< Foreground pattern ID */
#define VIDTPG_REG_RGB_MASK_ID		(0x00000030U)	/**< Color mask selection for RGB formats */
#define VIDTPG_REG_MOTION_SPEED		(0x00000038U)	/**< Motion speed */
#define VIDTPG_REG_COLOR_FMT		(0x00000040U)	/**< Color format */
#define VIDTPG_REG_ENABLE_INPUT		(0x00000098U)	/**< Enable input */
#define VIDTPG_REG_PASS_THRU_X_START	(0x000000a0)	/**< Video in horizontal start */
#define VIDTPG_REG_PASS_THRU_Y_START	(0x000000a8)	/**< Video in vertical start */
#define VIDTPG_REG_PASS_THRU_X_END	(0x000000b0)	/**< Video in horizontal end */
#define VIDTPG_REG_PASS_THRU_Y_END	(0x000000b8)	/**< Video in vertical end */


#define REG_READ(__BASE__, __OFFSET__)					\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__)))

#define REG_WRITE(__BASE__, __OFFSET__, __DATA__)				\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__))) = (__DATA__)


int
vidtpg_set_format(struct vidtpg *tpg, int height, int width, enum vidtpg_color_fmt color_fmt)
{
	REG_WRITE(tpg->base, VIDTPG_REG_ACTHEIGHT, height);
	REG_WRITE(tpg->base, VIDTPG_REG_ACTWIDTH, width);
	REG_WRITE(tpg->base, VIDTPG_REG_COLOR_FMT, color_fmt);
	REG_WRITE(tpg->base, VIDTPG_REG_PASS_THRU_X_START, 0);
	REG_WRITE(tpg->base, VIDTPG_REG_PASS_THRU_Y_START, 0);
	REG_WRITE(tpg->base, VIDTPG_REG_PASS_THRU_X_END, width);
	REG_WRITE(tpg->base, VIDTPG_REG_PASS_THRU_Y_END, height);

	return 0;
}

int
vidtpg_set_pattern(struct vidtpg *tpg, enum vidtpg_bgpat bgpat, enum vidtpg_fgpat fgpat)
{
	enum vidtpg_en_input en_input = ENINPUT_DISABLE;

	REG_WRITE(tpg->base, VIDTPG_REG_BGPAT_ID, bgpat);
	REG_WRITE(tpg->base, VIDTPG_REG_FGPAT_ID, fgpat);

	if (bgpat == BGPAT_VIDEOIN)
		en_input = ENINPUT_ENABLE;

	REG_WRITE(tpg->base, VIDTPG_REG_ENABLE_INPUT, en_input);

	/* Commit update */
	REG_WRITE(tpg->base, VIDTPG_REG_CONTROL, CONTROL_AP_START | CONTROL_AUTO_RESTART);

	return 0;
}

int
vidtpg_init(struct vidtpg *tpg, const char *devname)
{
	int fd, ret;
	void *base;

	if ((tpg == NULL) || (devname == NULL))
		return -1;

	fd = open(devname, O_RDWR);
	if (fd < 0)
		return fd;

	base = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (base == MAP_FAILED) {
		ret = -2;
		goto out;
	}

	tpg->base = base;
	tpg->fd = fd;
out:
	return ret;
}
