/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vid_tpg.h
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

#ifndef __VIDTPG_H
#define __VIDTPG_H

#ifdef __cplusplus
extern "C" {
#endif
    
    enum vidtpg_bgpat {
        BGPAT_VIDEOIN = 0x00,        /**< Pass the video input straight through the video output */
        BGPAT_HORZRAMP = 0x01,        /**< Horizontal Ramp which increases each component (RGB or Y) horizontally by 1 */
        BGPAT_VERTRAMP = 0x02,        /**< Vertical Ramp which increases each component (RGB or Y) vertically by 1 */
        BGPAT_TEMPRAMP = 0x03,        /**< Temporal Ramp which increases every pixel by a value set in the motion_speed register for every frame. */
        BGPAT_SLDRED = 0x04,        /**< Solid red output */
        BGPAT_SLDGREEN = 0x05,        /**<  Solid green output */
        BGPAT_SLDBLUE = 0x6,        /**< Solid blue output */
        BGPAT_SLDBLACK = 0x07,        /**< Solid black output */
        BGPAT_SLDWHITE = 0x08,        /**< Solid white output */
        BGPAT_COLORBARS = 0x09,        /**< Color bars */
        BGPAT_ZONEPLATE = 0x0A,        /**< Zone Plate output produces a ROM based sinusoidal pattern. */
        BGPAT_TARTANBARS = 0x0B,    /**< Tartan Color Bars */
        BGPAT_CROSSHATCH = 0x0C,    /**<  Draws a cross hatch pattern. */
        BGPAT_COLORSWEEP = 0x0D,    /**< Color sweep pattern */
        BGPAT_HORZVERTRAMP = 0x0E,    /**< A combined vertical and horizontal ramp */
        BGPAT_CHECKER = 0x0F,        /**< Black and white checker board */
        BGPAT_PSEUDORAND = 0x10,    /**< Pseudorandom pattern */
        BGPAT_DPCOLORRAMP = 0x11,    /**< DisplayPort color ramp */
        BGPAT_DPBWVERT = 0x12,        /**< DisplayPort black and white vertical lines */
        BGPAT_DPCOLORSQ = 0x13,        /**< DisplayPort color square */
    };
    
    enum vidtpg_fgpat {
        FGPAT_NOOVERLAY = 0x00,        /**< No overlay pattern. */
        FGPAT_MOVINGBOX = 0x01,        /**< Enabled moving box drawn over video output */
        FGPAT_XHAIR = 0x02,        /**< Draws crosshairs one pixel in width on the output video */
    };
    
    enum vidtpg_rgb_mask {
        RGBMASK_NONE = 0x00,        /**< No masking */
        RGBMASK_RED = 0x01,        /**< Mask out the red component */
        RGBMASK_GREEN = 0x02,        /**< Mask out the green component */
        RGBMASK_BLUE = 0x04,        /**< Mask out the blue component */
    };
    
    enum vidtpg_color_fmt {
        COLORFMT_RGB = 0x00,        /**< RGB format */
        COLORFMT_YUV444 = 0x01,        /**< YUV 4:4:4 format */
        COLORFMT_YUV422 = 0x02,        /**< YUV 4:2:2 format */
        COLORFMT_YUV420 = 0x03,        /**< YUV 4:2:0 format */
    };
    
    enum vidtpg_en_input {
        ENINPUT_DISABLE = 0x00,        /**< Ignore input video stream */
        ENINPUT_ENABLE = 0x01,        /**< Use input video stream */
    };
    
    struct vidtpg {
        void *base;
        int fd;
    };
    
    int vidtpg_init(struct vidtpg *tpg, const char *devname);
    int vidtpg_set_pattern(struct vidtpg *tpg, enum vidtpg_bgpat bgpat, enum vidtpg_fgpat fgpat);
    int vidtpg_set_format(struct vidtpg *tpg, int height, int width, enum vidtpg_color_fmt color_fmt);
    
#ifdef __cplusplus
}
#endif

#endif /* __VIDTPG_H */

