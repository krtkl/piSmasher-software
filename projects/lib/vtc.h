/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vtc.h
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    August 6, 2018
 * @brief   Video Timing Controller Userspace I/O Driver
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

#ifndef __VTC_H
#define __VTC_H

#ifdef __cplusplus
extern "C" {
#endif

enum vtc_mode {
	VTC_MODE_720p = 0,	/**< Video mode 720P */
	VTC_MODE_1080p,		/**< Video mode 1080P */
	VTC_MODE_1920x1200,	/**< Video mode 1920x1200 */
	VTC_MODE_480p,		/**< Video mode 480P */
	VTC_MODE_576p,		/**< Video mode 576P */
	VTC_MODE_VGA,		/**< Video mode VGA */
	VTC_MODE_SVGA,		/**< Video mode SVGA */
	VTC_MODE_XGA,		/**< Video mode XGA */
	VTC_MODE_SXGA,		/**< Video mode SXGA */
	VTC_MODE_WXGA,		/**< Video mode WXGA */
	VTC_MODE_WXGAPLUS,	/**< Video mode WXGAPlus */
	VTC_MODE_WSXGAPLUS,	/**< Video mode WSXGAPlus */
	VTC_MODE_1080i,		/**< Video mode 1080I */
	VTC_MODE_NTSC,		/**< Video mode NTSC */
	VTC_MODE_PAL,		/**< Video mode PAL */
};

struct vtc_dev {
	void	*base;
	int	fd;
};

int vtc_init(struct vtc_dev *dev, const char *devname);
int vtc_gen_enable(struct vtc_dev *dev, bool en);
int vtc_enable(struct vtc_dev *dev, bool en);
int vtc_set_generator_video_mode(struct vtc_dev *dev, enum vtc_mode mode);
int vtc_enable_interrupts(struct vtc_dev *dev);
int vtc_det_enable(struct vtc_dev *dev, bool en);
int vtc_det_dump(struct vtc_dev *dev);

#ifdef __cplusplus
}
#endif

#endif /* __VTC_H */
