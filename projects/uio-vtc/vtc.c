/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vtc.c
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


#include <sys/mman.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "vtc.h"

#define ERR_MODE_INVALID		(0x0100U)
#define ERR_NULL_PARAM			(0x0101U)

#define VTC_REG_CONTROL			(0x00000000U)	/**< Control */
#define VTC_REG_STATUS			(0x00000004U)	/**< Core/interrupt status */
#define VTC_REG_ERROR			(0x00000008U)	/**< Status/error */
#define VTC_REG_IRQ_ENABLE		(0x0000000CU)	/**< Interrupt enable/clear */
#define VTC_REG_VERSION			(0x00000010U)	/**< Core version */

#define VTC_REG_DET_ASIZE		(0x00000020U)	/**< Frame size (without blanking) */
#define VTC_REG_DET_STATUS		(0x00000024U)	/**< Timing measurement status */
#define VTC_REG_DET_ENC			(0x00000028U)	/**< Frame encoding */
#define VTC_REG_DET_POL			(0x0000002CU)	/**< Blanking/sync polarity */
#define VTC_REG_DET_HSIZE		(0x00000030U)	/**< Horizontal frame size (with blanking) */
#define VTC_REG_DET_VSIZE		(0x00000034U)	/**< Vertical frame size (with blanking) */
#define VTC_REG_DET_HSYNC		(0x00000038U)	/**< Hsync start and end cycle index */
#define VTC_REG_DET_VBLANK_F0		(0x0000003CU)	/**< Vblank start and end cycle index (field 0) */
#define VTC_REG_DET_VSYNC_V_F0		(0x00000040U)	/**< Start and end line index of VSync for field 0 */
#define VTC_REG_DET_VSYNC_H_F0		(0x00000044U)	/**< Start and end cycle index of VSync for field 0 */
#define VTC_REG_DET_VBLANK_F1		(0x00000048U)	/**< Start and end cycle index of VBlank for field 1 */
#define VTC_REG_DET_VSYNC_V_F1		(0x0000004CU)	/**< Start and end line index of VSync for field 1 */
#define VTC_REG_DET_VSYNC_H_F1		(0x00000050U)	/**< Start and end cycle index of VSync for field 1 */

#define VTC_REG_GEN_ASIZE		(0x00000060U)	/**< Generator frame size (without blanking) (R/W) */
#define VTC_REG_GEN_STATUS		(0x00000064U)	/**< Generator timing measurement status (RO) */
#define VTC_REG_GEN_ENC			(0x00000068U)	/**< Generator frame encoding (R/W) */
#define VTC_REG_GEN_POL			(0x0000006CU)	/**< Generator blank and sync polarities (R/W) */
#define VTC_REG_GEN_HSIZE		(0x00000070U)	/**< Generator horizontal frame size (with blanking) (R/W) */
#define VTC_REG_GEN_VSIZE		(0x00000074U)	/**< Generator vertical frame size (with blanking) (R/W) */
#define VTC_REG_GEN_HSYNC		(0x00000078U)	/**< Generator hsync start and end cycle index */
#define VTC_REG_GEN_VBLANK_F0		(0x0000007CU)	/**< Generator vblank start and end cycle index of vsync for field 0 */
#define VTC_REG_GEN_VSYNC_V_F0		(0x00000080U)	/**< Generator vsync start and end line index of vsync for field 0 */
#define VTC_REG_GEN_VSYNC_H_F0		(0x00000084U)	/**< Generator vsync start and end line index of vsync for field 0 */
#define VTC_REG_GEN_VBLANK_F1		(0x00000088U)	/**< Generator vblank start and end line index of vsync for field 1 */
#define VTC_REG_GEN_VSYNC_V_F1		(0x0000008CU)	/**< Generator vsync start and end line index of vsync for field 1 */
#define VTC_REG_GEN_VSYNC_H_F1		(0x00000090U)	/**< Generator vsync start and end cycle index of vsync for field 1 */

#define CONTROL_SW_ENABLE		(1 << 0)	/**< Core enable */
#define CONTROL_REG_UPDATE		(1 << 1)	/**< Register update. Generator and Fsync registers are double-buffered. */
#define CONTROL_GEN_ENABLE		(1 << 2)	/**< Generation enable */
#define CONTROL_DET_ENABLE		(1 << 3)	/**< Detection enable */
#define CONTROL_SYNC_ENABLE		(1 << 5)	/**< Generator synchronization enable */
#define CONTROL_FRAME_HSIZE_SRC		(1 << 8)	/**< Generator horizontal frame size source select */
#define CONTROL_ACTIVE_HSIZE_SRC	(1 << 9)	/**< Generator horizontal active size source select */
#define CONTROL_HSYNC_START_SRC		(1 << 10)	/**< Generator horizontal sync start source select */
#define CONTROL_HSYNC_END_SRC		(1 << 11)	/**< Generator horizontal sync end source select */
#define CONTROL_FRAME_VSIZE_SRC		(1 << 13)	/**< Generator vertical frame size source select */
#define CONTROL_ACTIVE_VSIZE_SRC	(1 << 14)	/**< Generator vertical active size source select */
#define CONTROL_VSYNC_START_SRC		(1 << 15)	/**< Generator vertical sync start source select */
#define CONTROL_VSYNC_END_SRC		(1 << 16)	/**< Generator vertical sync end source select */
#define CONTROL_VBLANK_HOFF_SRC		(1 << 17)	/**< Generator vertical blank offset source select */
#define CONTROL_CHROMA_SRC		(1 << 18)	/**< Generator chroma polarity and encoding source select */
#define CONTROL_VBLANK_POL_SRC		(1 << 20)	/**< Vertical Blank Polarity Source Select */
#define CONTROL_HBLANK_POL_SRC		(1 << 21)	/**< Horizontal blank polarity source select */
#define CONTROL_VSYNC_POL_SRC		(1 << 22)	/**< Vertical sync polarity source select */
#define CONTROL_HSYNC_POL_SRC		(1 << 23)	/**< Horizontal sync polarity source select */
#define CONTROL_ACTIVE_VIDEO_POL_SRC	(1 << 24)	/**< Active video polarity source select */
#define CONTROL_ACTIVE_CHROMA_POL_SRC	(1 << 25)	/**< Active chroma polarity source select */
#define CONTROL_FIELD_ID_POL_SRC	(1 << 26)	/**< Field ID polarity source select */
#define CONTROL_FSYNC_RESET		(1 << 30)	/**< Frame Sync Core reset */
#define CONTROL_SW_RESET		(1 << 31)	/**< Core reset */

#define STATUS_LOCK			(1 << 8)	/**< Lock status */
#define STATUS_LOCK_LOSS		(1 << 9)	/**< Loss-of-lock status */
#define STATUS_DET_VBLANK		(1 << 10)	/**< Detected vertival blank interrupt status */
#define STATUS_DET_ACTIVE_VIDEO		(1 << 11)	/**< Detected active video interrupt status */
#define STATUS_GEN_VBLANK		(1 << 12)	/**< Generated vertical blank interrupt status */
#define STATUS_GEN_ACTIVE_VIDEO		(1 << 13)	/**< Generated active video interrupt status */
#define STATUS_FSYNC_MASK		(0xFFFF0000U)	/**< Fsync interrupt status */

#define ERROR_VBLANK_LOCK		(1 << 16)	/**< Vertical blank lock status */
#define ERROR_HBLANK_LOCK		(1 << 17)	/**< Horizontal blank lock status */
#define ERROR_VSYNC_LOCK		(1 << 18)	/**< Vertical sync lock status */
#define ERROR_HSYNC_LOCK		(1 << 19)	/**< Horizontal sync lock status */
#define ERROR_ACTIVE_VIDEO_LOCK		(1 << 20)	/**< Active video lock status */
#define ERROR_ACTIVE_CHROMA_LOCK	(1 << 21)	/**< Active chroma lock status */

#define IRQ_ENABLE_LOCK			(1 << 8)	/**< Lock interrupt enable */
#define IRQ_ENABLE_LOCK_LOSS		(1 << 9)	/**< Loss-of-lock interrupt enable */
#define IRQ_ENABLE_DET_VBLANK		(1 << 10)	/**< Detected vertical blank interrupt enable */
#define IRQ_ENABLE_DET_ACTIVE_VIDEO	(1 << 11)	/**< Detected active video interrupt enable */
#define IRQ_ENABLE_GEN_VBLANK		(1 << 12)	/**< Generated vertical blank interrupt enable */
#define IRQ_ENABLE_GEN_ACTIVE_VIDEO	(1 << 13)	/**< Generated active video interrupt enable */
#define IRQ_ENABLE_FSYNC_MASK		(0xFFFF0000U)	/**< Frame synchronization interrupt enable */

#define GEN_ASIZE_HSIZE			(0x00001FFFU)	/**< Generated horizontal active frame size */
#define GEN_ASIZE_VSIZE			(0x1FFF0000U)	/**< Generated vertical active frame size */

#define GEN_STATUS_VBLANK		(1 << 1)	/**< Generated vertical blank interrupt status */
#define GEN_STATUS_ACTIVE_VIDEO		(1 << 2)	/**< Generated active video interrupt status */

#define GEN_ENC_VIDEO_FORMAT_MASK	(0x000000FFU)	/**< Generated video format */
#define GEN_ENC_INTERLACED		(1 << 6)	/**< Generated video interlaced format */
#define GEN_ENC_FIELD_ID_PARITY		(1 << 7)	/**< Generated field ID parity */
#define GEN_ENC_CHROMA_PARITY_MASK	(0x00000300U)	/**< Generated chroma parity */

#define GEN_POL_VBLANK			(1 << 0)	/**< Generated vertical blank polarity */
#define GEN_POL_HBLANK			(1 << 1)	/**< Generated horizontal blank polarity */
#define GEN_POL_VSYNC			(1 << 2)	/**< Generated vertical sync polarity */
#define GEN_POL_HSYNC			(1 << 3)	/**< Generated horizontal sync polarity */
#define GEN_POL_ACTIVE_VIDEO		(1 << 4)	/**< Generated active video polarity */
#define GEN_POL_ACTIVE_CHROMA		(1 << 5)	/**< Generated active chroma polarity */
#define GEN_POL_FIELD_ID		(1 << 6)	/**< Generated field ID polarity */
#define GEN_POL_ALL_MASK		(0x0000007FU)	/**< All polarity bits */

#define GEN_HSIZE_MASK			(0x00001FFFU)	/**< Generated horizontal frame size */
#define GEN_VSIZE_MASK			(0x00001FFFU)	/**< Generated vertical frame size */

#define GEN_HSYNC_START_MASK		(0x00001FFFU)	/**< Generated horizontal sync end start cycle index */
#define GEN_HSYNC_END_MASK		(0x1FFF0000U)	/**< Generated horizontal sync end end cycle index */

#define GEN_VBLANK_HSTART_MASK		(0x00001FFFU)	/**< Generated vertical blank horizontal start start cycle index */
#define GEN_VBLANK_HEND_MASK		(0x1FFF0000U)	/**< Generated vertical blank horizontal start end cycle index */

#define GEN_VSYNC_VSTART_MASK		(0x00001FFFU)	/**< Generated vertical sync vertical start start cycle index */
#define GEN_VSYNC_VEND_MASK		(0x1FFF0000U)	/**< Generated vertical sync vertical start end cycle index */

#define GEN_VSYNC_HSTART_MASK		(0x00001FFFU)	/**< Generated vertical sync horizontal start start cycle index */
#define GEN_VSYNC_HEND_MASK		(0x1FFF0000U)	/**< Generated vertical sync horizontal start end cycle index */


#define REG_READ(__BASE__, __OFFSET__)						\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__)))

#define REG_WRITE(__BASE__, __OFFSET__, __DATA__)				\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__))) = (__DATA__)

struct vtc_polarity {
	bool act_chroma;		/**< Active Chroma Output Polarity */
	bool act_video;		/**< Active Video Output Polarity */
	bool fid;			/**< Field ID Output Polarity */
	bool vblank;			/**< Vertical Blank Output Polarity */
	bool vsync;			/**< Vertical Sync Output Polarity */
	bool hblank;			/**< Horizontal Blank Output Polarity */
	bool hsync;			/**< Horizontal Sync Output Polarity */
};

struct vtc_hoffset {
	uint16_t v0_blank_hstart;	/**< Vertical Blank Hori Offset Start (field 0) */
	uint16_t v0_blank_hend;	/**< Vertical Blank Hori Offset End (field 0) */
	uint16_t v0_sync_hstart;	/**< Vertical Sync  Hori Offset Start (field 0) */
	uint16_t v0_sync_hend;		/**< Vertical Sync  Hori Offset End (field 0) */
	uint16_t v1_blank_hstart;	/**< Vertical Blank Hori Offset Start (field 1) */
	uint16_t v1_blank_hend;	/**< Vertical Blank Hori Offset End (field 1) */
	uint16_t v1_sync_hstart;	/**< Vertical Sync  Hori Offset Start (field 1) */
	uint16_t v1_sync_hend;		/**< Vertical Sync  Hori Offset End (field 1) */
};

struct vtc_signal {
	uint16_t OriginMode;		/**< Origin Mode */
	uint16_t HTotal;		/**< Horizontal total clock cycles per Line */
	uint16_t HFrontPorchStart;	/**< Horizontal Front Porch Start Cycle Count */
	uint16_t HSyncStart;		/**< Horizontal Sync Start Cycle Count */
	uint16_t HBackPorchStart;	/**< Horizontal Back Porch Start Cycle Count */
	uint16_t HActiveStart;		/**< Horizontal Active Video Start Cycle Count */

	uint16_t V0Total;		/**< Total lines per Frame (Field 0) */
	uint16_t V0FrontPorchStart;	/**< Vertical Front Porch Start Line Count (Field 0) */
	uint16_t V0SyncStart;		/**< Vertical Sync Start Line Count (Field 0) */
	uint16_t V0BackPorchStart;	/**< Vertical Back Porch Start Line Count (Field 0) */
	uint16_t V0ActiveStart;		/**< Vertical Active Video Start Line Count (Field 0) */
	uint16_t V0ChromaStart;		/**< Active Chroma Start Line Count (Field 0) */

	uint16_t V1Total;		/**< Total lines per Frame (Field 1) */
	uint16_t V1FrontPorchStart;	/**< Vertical Front Porch Start Line Count (Field 1) */
	uint16_t V1SyncStart;		/**< Vertical Sync Start Line Count (Field 1) */
	uint16_t V1BackPorchStart;	/**< Vertical Back Porch Start Line Count (Field 1) */
	uint16_t V1ActiveStart;		/**< Vertical Active Video Start Line Count (Field 1) */
	uint16_t V1ChromaStart;		/**< Active Chroma Start Line Count (Field 1) */
	bool interlaced;		/**< Interlaced video */
};

/**
 *                                                             HSize --+
 *                                                                     |
 *                                                   HSync End --+     |
 *                                                               |     |
 *                 |                         HSync Start --+     |     |
 *                 |                                       |     |     |
 *                 |                  HBlank Start --+     |     |     |
 *          (0, 0) |                                 |     |     |     |
 *        ---------x---------------------------------+-----+-----+-----+   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |          Active Video           |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 *                 |                                 |     .     .     .   |         |
 * VBlank Start ---+---------------------------------+ . . . . . . . . . . | . . . . +----+
 *                 |                                 .     .     .     .   |              |
 *                 |                                 .     .     .     .   |              |
 *  VSync Start ---+ . . . . . . . . . . . . . . . . . . . . . . . . . . . +----+         |
 *                 |                                 .     .     .     .        |         |
 *    VSync End ---+ . . . . . . . . . . . . . . . . . . . . . . . . . . . +----+         |
 *                 |                                 .     .     .     .   |              |
 *                 |                                 .     .     .     .   |              |
 *        VSize ---+ . . . . . . . . . . . . . . . . . . . . . . . . . . . | . . . . . . .|
 *                                                   .     .     .     .
 *                                                   .     +-----+     .
 *                                                   .     |     |     .
 *                 ----------------------------------------+     +------
 *                                                   .                 .
 *                                                   +------------------
 *                                                   |
 *                 ----------------------------------+
 */
static struct vtc_timing {
	enum vtc_mode	mode;		/**< Video mode */
	uint16_t	hactive;	/**< Horizontal active size */
	uint16_t	hfporch;	/**< Horizontal front porch (HSync Start - HBlank Start) */
	uint16_t	hsyncwidth;	/**< Horizontal sync width (HSync Start - HSync End) */
	uint16_t	hbackporch;	/**< Horizontal back porch (HSize - HSync End) */
	uint16_t	hsyncpol;	/**< Horizontal sync polarity */

	uint16_t	vactive;	/**< Vertical active size */
	uint16_t	v0fporch;	/**< Vertical front porch size (VSync Start - VBlank Start) */
	uint16_t	v0syncwidth;	/**< Vertical sync width (VSync End - VSync Start) */
	uint16_t	v0backporch;	/**< Horizontal back porch (VSize - VSync End) */

	uint16_t	v1fporch;	/**< Vertical front porch */
	uint16_t	v1syncwidth;	/**< Vertical sync width */
	uint16_t	v1backporch;	/**< Vertical back porch */

	uint16_t	vsyncpol;	/**< Vertical sync polarity */
	bool		interlaced;	/**< Video is interlaced */
} video_timing[] = {
	{
		.mode = VTC_MODE_720p,
		.hactive = 1280,
		.hfporch = 110,
		.hsyncwidth = 40,
		.hbackporch = 220,
		.hsyncpol = 1,
		.vactive = 720,
		.v0fporch = 5,
		.v0syncwidth = 5,
		.v0backporch = 20,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_1080p,
		.hactive = 1920,
		.hfporch = 88,
		.hsyncwidth = 44,
		.hbackporch = 148,
		.hsyncpol = 1,
		.vactive = 1080,
		.v0fporch = 4,
		.v0syncwidth = 5,
		.v0backporch = 36,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_480p,
		.hactive = 720,
		.hfporch = 16,
		.hsyncwidth = 62,
		.hbackporch = 60,
		.hsyncpol = 0,
		.vactive = 480,
		.v0fporch = 9,
		.v0syncwidth = 6,
		.v0backporch = 30,
		.vsyncpol = 0,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_576p,
		.hactive = 720,
		.hfporch = 12,
		.hsyncwidth = 64,
		.hbackporch = 68,
		.hsyncpol = 0,
		.vactive = 576,
		.v0fporch = 5,
		.v0syncwidth = 5,
		.v0backporch = 39,
		.vsyncpol = 0,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_VGA,
		.hactive = 656,
		.hfporch = 8,
		.hsyncwidth = 96,
		.hbackporch = 40,
		.hsyncpol = 0,
		.vactive = 496,
		.v0fporch = 2,
		.v0syncwidth = 2,
		.v0backporch = 25,
		.vsyncpol = 0,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_SVGA,
		.hactive = 800,
		.hfporch = 40,
		.hsyncwidth = 128,
		.hbackporch = 88,
		.hsyncpol = 1,
		.vactive = 600,
		.v0fporch = 1,
		.v0syncwidth = 4,
		.v0backporch = 23,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_XGA,
		.hactive = 1024,
		.hfporch = 24,
		.hsyncwidth = 136,
		.hbackporch = 160,
		.hsyncpol = 0,
		.vactive = 768,
		.v0fporch = 3,
		.v0syncwidth = 6,
		.v0backporch = 29,
		.vsyncpol = 0,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_SXGA,
		.hactive = 1280,
		.hfporch = 48,
		.hsyncwidth = 112,
		.hbackporch = 248,
		.hsyncpol = 1,
		.vactive = 1024,
		.v0fporch = 1,
		.v0syncwidth = 3,
		.v0backporch = 38,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_WXGA,
		.hactive = 1366,
		.hsyncpol = 1,
		.vactive = 768,
		.hfporch = 70,
		.hsyncwidth = 143,
		.hbackporch = 213,
		.v0fporch = 3,
		.v0syncwidth = 3,
		.v0backporch = 24,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_WXGAPLUS,
		.hactive = 1440,
		.hfporch = 80,
		.hsyncwidth = 152,
		.hbackporch = 232,
		.hsyncpol = 0,
		.vactive = 900,
		.v0fporch = 3,
		.v0syncwidth = 6,
		.v0backporch = 25,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_WSXGAPLUS,
		.hactive = 1680,
		.hfporch = 104,
		.hsyncwidth = 176,
		.hbackporch = 280,
		.hsyncpol = 0,
		.vactive = 1050,
		.v0fporch = 3,
		.v0syncwidth = 6,
		.v0backporch = 30,
		.vsyncpol = 1,
		.interlaced = false
	},
	{
		.mode = VTC_MODE_1080i,
		.hactive = 1920,
		.hfporch = 88,
		.hsyncwidth = 44,
		.hbackporch = 148,
		.hsyncpol = 1,
		.vactive = 540,
		.v0fporch = 2,
		.v0syncwidth = 5,
		.v0backporch = 15,
		.v1fporch = 2,
		.v1syncwidth = 5,
		.v1backporch = 16,
		.vsyncpol = 1,
		.interlaced = true,
	},
	{
		.mode = VTC_MODE_NTSC,
		.hactive = 720,
		.hfporch = 19,
		.hsyncwidth = 62,
		.hbackporch = 57,
		.hsyncpol = 0,
		.vactive = 240,
		.v0fporch = 4,
		.v0syncwidth = 3,
		.v0backporch = 15,
		.v1fporch = 4,
		.v1syncwidth = 3,
		.v1backporch = 16,
		.vsyncpol = 0,
		.interlaced = true,
	},
	{
		.mode = VTC_MODE_PAL,
		.hactive = 720,
		.hfporch = 12,
		.hsyncwidth = 63,
		.hbackporch = 69,
		.hsyncpol = 0,
		.vactive = 288,
		.v0fporch = 2,
		.v0syncwidth = 3,
		.v0backporch = 19,
		.v1fporch = 2,
		.v1syncwidth = 3,
		.v1backporch = 20,
		.vsyncpol = 0,
		.interlaced = true,
	},
	{ /* Sentinel */ }
};

struct vtc_timing *
vtc_mode2tim(struct vtc_dev *dev, enum vtc_mode mode)
{
	struct vtc_timing *timing = video_timing;

	while (timing && timing->hactive) {
		if (timing->mode == mode)
			return timing;

		timing++;
	}

	return NULL;
}

static void
vtc_write_start_end(struct vtc_dev *dev, uint32_t reg, uint32_t start, uint32_t end)
{
	uint32_t reg_val;

	reg_val = start & (0x00001FFFU);
	reg_val |= (end << 16) & (0x1FFF0000U);

	REG_WRITE(dev->base, reg, reg_val);
}

void
vtc_tim2sig(struct vtc_dev *dev,
		struct vtc_timing *timing,
		struct vtc_signal *signal,
		struct vtc_hoffset *hoffset,
		struct vtc_polarity *pol)
{
	/* Setting up VTC Polarity.  */
	memset((void *) pol, 0, sizeof(struct vtc_polarity));

	pol->act_chroma = true;
	pol->act_video = true;
	pol->fid = true;

	pol->vblank = timing->vsyncpol;
	pol->vsync = timing->vsyncpol;
	pol->hblank = timing->hsyncpol;
	pol->hsync = timing->hsyncpol;

	memset((void *) signal, 0, sizeof(struct vtc_signal));
	memset((void *) hoffset, 0, sizeof(struct vtc_hoffset));

	/* Populate the VTC Signal config structure. */
	/* Active Video starts at 0 */
	signal->OriginMode = 1;
	signal->HActiveStart = 0;
	signal->HFrontPorchStart = timing->hactive;
	signal->HSyncStart = signal->HFrontPorchStart + timing->hfporch;
	signal->HBackPorchStart = signal->HSyncStart + timing->hsyncwidth;
	signal->HTotal = signal->HBackPorchStart + timing->hbackporch;

	signal->V0ChromaStart = 0;
	signal->V0ActiveStart = 0;
	signal->V0FrontPorchStart = timing->vactive;
	signal->V0SyncStart = signal->V0FrontPorchStart + timing->v0fporch - 1;
	signal->V0BackPorchStart = signal->V0SyncStart + timing->v0syncwidth;
	signal->V0Total = signal->V0BackPorchStart + timing->v0backporch + 1;

	hoffset->v0_blank_hstart = signal->HFrontPorchStart;
	hoffset->v0_blank_hend = signal->HFrontPorchStart;
	hoffset->v0_sync_hstart = signal->HSyncStart;
	hoffset->v0_sync_hend = signal->HSyncStart;

	if (timing->interlaced) {
		signal->V1ChromaStart = 0;
		signal->V1ActiveStart = 0;
		signal->V1FrontPorchStart = timing->vactive;
		signal->V1SyncStart = signal->V1FrontPorchStart + timing->v1fporch - 1;
		signal->V1BackPorchStart = signal->V1SyncStart + timing->v1syncwidth;
		signal->V1Total = signal->V1BackPorchStart + timing->v1backporch + 1;
		signal->interlaced = true;

		/* Align to H blank */
		hoffset->v1_blank_hstart = signal->HFrontPorchStart;
		/* Align to H Blank */
		hoffset->v1_blank_hend = signal->HFrontPorchStart;

		/* Align to half line */
		hoffset->v1_sync_hstart = signal->HSyncStart - (signal->HTotal / 2);
		hoffset->v1_sync_hend = signal->HSyncStart - (signal->HTotal / 2);
	} else {
		/* Set Field 1 same as Field 0 */
		signal->V1ChromaStart = signal->V0ChromaStart;
		signal->V1ActiveStart = signal->V0ActiveStart;
		signal->V1FrontPorchStart = signal->V0FrontPorchStart;
		signal->V1SyncStart = signal->V0SyncStart;
		signal->V1BackPorchStart = signal->V0BackPorchStart;
		signal->V1Total = signal->V0Total;
		signal->interlaced = false;

		hoffset->v1_blank_hstart = hoffset->v0_blank_hstart;
		hoffset->v1_blank_hend = hoffset->v0_blank_hend;
		hoffset->v1_sync_hstart = hoffset->v0_sync_hstart;
		hoffset->v1_sync_hend = hoffset->v0_sync_hend;
	}
}

static int
vtc_reset(struct vtc_dev *dev)
{
	struct timeval start, end;
	uint32_t reg_val;

	reg_val = REG_READ(dev->base, VTC_REG_CONTROL);
	reg_val |= CONTROL_SW_RESET;
	REG_WRITE(dev->base, VTC_REG_CONTROL, reg_val);

	gettimeofday(&start, NULL);
	while (1) {
		reg_val = REG_READ(dev->base, VTC_REG_CONTROL);
		if (!(reg_val & CONTROL_SW_RESET))
			break;

		gettimeofday(&end, NULL);
		if (end.tv_sec > (start.tv_sec + 5))
			return -1;

		usleep(1000 * 10);
	}

	return 0;
}

/**
 * @brief	Enable Video Timing Controller
 *
 * @param	dev:	Video timing controller device structure pointer
 * @param	en:	Enable timing controller selection
 * @return	None
 */
int
vtc_enable(struct vtc_dev *dev, bool en)
{
	uint32_t reg_val;

	reg_val = REG_READ(dev->base, VTC_REG_CONTROL);

	if (en)
		reg_val |= CONTROL_SW_RESET;
	else
		reg_val &= ~(CONTROL_SW_RESET);

	REG_WRITE(dev->base, VTC_REG_CONTROL, reg_val);

	return 0;
}

/**
 * @brief	Enable Video Timing Generator
 *
 * @param	dev:	Video timing controller device structure pointer
 * @param	en:	Enable generator selection
 * @return	None
 */
int
vtc_gen_enable(struct vtc_dev *dev, bool en)
{
	uint32_t reg_val;

	reg_val = REG_READ(dev->base, VTC_REG_CONTROL);

	if (en)
		reg_val |= CONTROL_GEN_ENABLE;
	else
		reg_val &= ~(CONTROL_GEN_ENABLE);

	reg_val |= CONTROL_REG_UPDATE;

	REG_WRITE(dev->base, VTC_REG_CONTROL, reg_val);

	return 0;
}

/**
 * @brief	Enable Video Timing Detector
 */
int
vtc_det_enable(struct vtc_dev *dev, bool en)
{
	uint32_t reg_val;
	struct timeval start, end;

	reg_val = REG_READ(dev->base, VTC_REG_CONTROL);

	if (en)
		reg_val |= CONTROL_DET_ENABLE;
	else
		reg_val &= ~(CONTROL_DET_ENABLE);

	REG_WRITE(dev->base, VTC_REG_CONTROL, reg_val);

	gettimeofday(&start, NULL);
	while (1) {

		reg_val = REG_READ(dev->base, VTC_REG_STATUS);
		if (reg_val & STATUS_LOCK)
			break;

		gettimeofday(&end, NULL);
		if (end.tv_sec > (start.tv_sec + 5))
			return -1;

		usleep(1000 * 5);
	}


	return 0;
}

int
vtc_det_dump(struct vtc_dev *dev)
{
	uint32_t reg_val;

	printf("VTC Detection:\n");

	reg_val = REG_READ(dev->base, VTC_REG_DET_ASIZE);
	printf("  Act VSIZE: %d\n", (reg_val >> 16) & 0x1FFF);
	printf("  Act HSIZE: %d\n", reg_val & 0x1FFF);

	reg_val = REG_READ(dev->base, VTC_REG_DET_HSIZE);
	printf("  HSIZE: %d\n", reg_val);

	reg_val = REG_READ(dev->base, VTC_REG_DET_VSIZE);
	printf("  VSIZE: %d\n", reg_val);

	reg_val = REG_READ(dev->base, VTC_REG_DET_HSYNC);
	printf("  HSYNC Start: %d\n", reg_val & 0x1FFF);
	printf("  HSYNC End: %d\n", (reg_val >> 16) & 0x1FFF);

	reg_val = REG_READ(dev->base, VTC_REG_DET_VBLANK_F0);
	printf("  VBLANK Start: %d\n", reg_val & 0x1FFF);
	printf("  VBLANK End: %d\n", (reg_val >> 16) & 0x1FFF);

	reg_val = REG_READ(dev->base, VTC_REG_DET_VSYNC_V_F0);
	printf("  VSYNC_V Start: %d\n", reg_val & 0x1FFF);
	printf("  VSYNC_V End: %d\n", (reg_val >> 16) & 0x1FFF);

	reg_val = REG_READ(dev->base, VTC_REG_DET_VSYNC_H_F0);
	printf("  VSYNC_H Start: %d\n", reg_val & 0x1FFF);
	printf("  VSYNC_H End: %d\n", (reg_val >> 16) & 0x1FFF);

	return 0;
}

/**
 * @brief	Set Polarity
 */
void
vtc_set_polarity(struct vtc_dev *dev, struct vtc_polarity *pol)
{
	uint32_t reg_val;

	/*
	 * Read Control register value back and clear all polarity
	 * bits first
	 */
	reg_val = REG_READ(dev->base, VTC_REG_GEN_POL);
	reg_val &= ~(GEN_POL_ALL_MASK);

	if (pol->act_chroma)
		reg_val |= GEN_POL_ACTIVE_CHROMA;

	if (pol->act_video)
		reg_val |= GEN_POL_ACTIVE_VIDEO;

	if (pol->fid)
		reg_val |= GEN_POL_FIELD_ID;

	if (pol->vblank)
		reg_val |= GEN_POL_VBLANK;

	if (pol->vsync)
		reg_val |= GEN_POL_VSYNC;

	if (pol->hblank)
		reg_val |= GEN_POL_HBLANK;

	if (pol->hsync)
		reg_val |= GEN_POL_HSYNC;

	REG_WRITE(dev->base, VTC_REG_GEN_POL, reg_val);
}

/**
 * @brief	Get Version
 */
uint32_t
vtc_get_version(struct vtc_dev *dev)
{
	uint32_t version;

	version = REG_READ(dev->base, VTC_REG_VERSION);

	return version;
}

static void
vtc_set_genenerator_hoffset(struct vtc_dev *dev, struct vtc_hoffset *hoffset)
{
	uint32_t start, end;

	/*
	 * Generator VBlank Hori. Offset 0 register value
	 */
	start = hoffset->v0_blank_hstart;
	end = hoffset->v0_blank_hend;
	vtc_write_start_end(dev, VTC_REG_GEN_VBLANK_F0, start, end);

	/*
	 * Generator VSync Hori. Offset 0 register value
	 */
	start = hoffset->v0_sync_hstart;
	end = hoffset->v0_sync_hend;
	vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_H_F0, start, end);

	/*
	 * Generator VBlank Hori. Offset 1 register value
	 */
	start = hoffset->v1_blank_hstart;
	end = hoffset->v1_blank_hend;
	vtc_write_start_end(dev, VTC_REG_GEN_VBLANK_F1, start, end);

	/*
	 * Generator VSync Hori. Offset 1 register value
	 */
	start = hoffset->v1_sync_hstart;
	end = hoffset->v1_sync_hend;
	vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_H_F1, start, end);
}

static void
vtc_set_generator(struct vtc_dev *dev, struct vtc_signal *signal)
{
	uint32_t reg_val;
	uint32_t r_htotal, r_vtotal, r_hactive, r_vactive;
	uint32_t start, end;
	struct vtc_signal *SCPtr;
	struct vtc_hoffset hoffset;

	SCPtr = signal;
	if (SCPtr->OriginMode == 0) {
		r_htotal = SCPtr->HTotal + 1;
		r_vtotal = SCPtr->V0Total + 1;

		r_hactive = SCPtr->HTotal - SCPtr->HActiveStart + 1;
		r_vactive = SCPtr->V0Total - SCPtr->V0ActiveStart + 1;
	} else {
		r_htotal = SCPtr->HTotal;			/* Total in mode=1 is the line width */
		r_vtotal = SCPtr->V1Total;			/* Total in mode=1 is the frame height */

		r_hactive = SCPtr->HFrontPorchStart;
		r_vactive = SCPtr->V0FrontPorchStart;
	}

	/* Set Horizontal frame size */
	reg_val = r_htotal & (0x00001FFFU);
	REG_WRITE(dev->base, VTC_REG_GEN_HSIZE, reg_val);

	/* Set Vertical frame size */
	start = r_htotal;
	end = r_vtotal;
	vtc_write_start_end(dev, VTC_REG_GEN_VSIZE, start, end);

	/* Set Active frame size */
	start = r_hactive;
	end = r_vactive;
	vtc_write_start_end(dev, VTC_REG_GEN_ASIZE, start, end);

	if (SCPtr->OriginMode == 0) {
		/* Update the Generator Horizontal 1 Register */
		start = SCPtr->HSyncStart + r_hactive;
		end = SCPtr->HBackPorchStart + r_hactive;
		vtc_write_start_end(dev, VTC_REG_GEN_HSYNC, start, end);

		/* Update the Generator Vertical 1 Register (field 0) */
		start = SCPtr->V0SyncStart + SCPtr->V0Total - SCPtr->V0ActiveStart;
		end = SCPtr->V0BackPorchStart + SCPtr->V0Total - SCPtr->V0ActiveStart;
		vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_V_F0, start, end);

		/* Update the Generator Vertical Sync Register (field 1) */
		start = SCPtr->V1SyncStart + r_vactive - 1;
		end = SCPtr->V1BackPorchStart + r_vactive - 1;
		vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_V_F1, start, end);
	} else {
		/* Update the Generator Horizontal 1 Register */
		start = SCPtr->HSyncStart;
		end = SCPtr->HBackPorchStart;
		vtc_write_start_end(dev, VTC_REG_GEN_HSYNC, start, end);

		/* Update the Generator Vertical Sync Register (field 0) */
		start = SCPtr->V0SyncStart;
		end = SCPtr->V0BackPorchStart;
		vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_V_F0, start, end);

		/* Update the Generator Vertical Sync Register (field 1) */
		start = SCPtr->V1SyncStart;
		end = SCPtr->V1BackPorchStart;
		vtc_write_start_end(dev, VTC_REG_GEN_VSYNC_V_F1, start, end);
	}

	/* Chroma Start */
	reg_val = REG_READ(dev->base, VTC_REG_GEN_ENC);
	reg_val &= ~(GEN_ENC_CHROMA_PARITY_MASK);
	reg_val |= ((SCPtr->V0ChromaStart - SCPtr->V0ActiveStart) << 8) & GEN_ENC_CHROMA_PARITY_MASK;

	if (SCPtr->interlaced)
		reg_val |= GEN_ENC_INTERLACED;
	else
		reg_val &= ~(GEN_ENC_INTERLACED);

	REG_WRITE(dev->base, VTC_REG_GEN_ENC, reg_val);

	hoffset.v0_blank_hstart = r_hactive;
	hoffset.v0_blank_hend = r_hactive;

	hoffset.v1_blank_hstart = r_hactive;
	hoffset.v1_blank_hend = r_hactive;

	if (SCPtr->OriginMode == 0) {
		hoffset.v0_sync_hstart = SCPtr->HSyncStart + r_hactive;
		hoffset.v0_sync_hend = SCPtr->HSyncStart + r_hactive;

		hoffset.v1_sync_hstart = SCPtr->HSyncStart + r_hactive;
		hoffset.v1_sync_hend = SCPtr->HSyncStart + r_hactive;
	} else {
		hoffset.v0_sync_hstart = SCPtr->HSyncStart;
		hoffset.v0_sync_hend = SCPtr->HSyncStart;

		hoffset.v1_sync_hstart = SCPtr->HSyncStart;
		hoffset.v1_sync_hend = SCPtr->HSyncStart;
	}

	vtc_set_genenerator_hoffset(dev, &hoffset);
}

/**
 * @brief	Set Video Timing Generator Timing
 */
static int
vtc_set_generator_timing(struct vtc_dev *dev, struct vtc_timing *timing)
{
	if ((dev == NULL) || (timing == NULL))
		return ERR_NULL_PARAM;

	struct vtc_polarity polarity;
	struct vtc_signal signal;
	struct vtc_hoffset hoffset;

	vtc_tim2sig(dev, timing, &signal, &hoffset, &polarity);

	vtc_set_polarity(dev, &polarity);
	vtc_set_generator(dev, &signal);

	return 0;
}

/**
 * @brief	Set Video Generator Video Mode
 */
int
vtc_set_generator_video_mode(struct vtc_dev *dev, enum vtc_mode mode)
{
	uint32_t reg_val;
	int hact, vact, htotal, vtotal, hsync_st, hsync_end, vsync_st, vsync_end;

	switch (mode) {
	case VTC_MODE_720p:
		hact = 1280;
		vact = 720;
		htotal = 1650;
		vtotal = 750;
		hsync_st = 1390;
		hsync_end = 1430;
		vsync_st = 724;
		vsync_end = 729;
		break;

	case VTC_MODE_1080p:
		hact = 1920;
		vact = 1080;
		htotal = 2200;
		vtotal = 1125;
		hsync_st = 2008;
		hsync_end = 2052;
		vsync_st = 1083;
		vsync_end = 1088;
		break;

	case VTC_MODE_WXGA:
		hact = 1366;
		vact = 768;
		htotal = 1792;
		vtotal = 798;
		hsync_st = 1436;
		hsync_end = 1579;
		vsync_st = 770;
		vsync_end = 773;
		break;

	default:
		printf("Unsupported video mode %d\n", mode);
		return -1;
	}

	REG_WRITE(dev->base, VTC_REG_GEN_POL, 0x7FU);

	reg_val = (vact << 16) | (hact);
	REG_WRITE(dev->base, VTC_REG_GEN_ASIZE, reg_val);
	REG_WRITE(dev->base, VTC_REG_GEN_HSIZE, htotal);
	REG_WRITE(dev->base, VTC_REG_GEN_VSIZE, vtotal);

	reg_val = (hsync_end << 16) | hsync_st;
	REG_WRITE(dev->base, VTC_REG_GEN_HSYNC, reg_val);

	reg_val = (vsync_end << 16) | vsync_st;
	REG_WRITE(dev->base, VTC_REG_GEN_VSYNC_V_F0, reg_val);

	reg_val = (hact << 16) | hact;
	REG_WRITE(dev->base, VTC_REG_GEN_VBLANK_F0, reg_val);
	REG_WRITE(dev->base, VTC_REG_GEN_VSYNC_H_F0, reg_val);

	return 0;
}

int
vtc_enable_interrupts(struct vtc_dev *dev)
{
	uint32_t reg_val;

	reg_val = REG_READ(dev->base, VTC_REG_IRQ_ENABLE);

	reg_val |= (IRQ_ENABLE_LOCK |
			IRQ_ENABLE_LOCK_LOSS |
			IRQ_ENABLE_DET_VBLANK |
			IRQ_ENABLE_DET_ACTIVE_VIDEO);

	REG_WRITE(dev->base, VTC_REG_IRQ_ENABLE, reg_val);

	return 0;
}

/**
 * @brief	Video Timing Controller Initialization
 */
int
vtc_init(struct vtc_dev *dev, const char *devname)
{
	int fd, ret;
	void *base;

	if ((dev == NULL) || (devname == NULL))
		return -1;

	fd = open(devname, O_RDWR);
	if (fd < 0)
		return fd;

	base = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (base == MAP_FAILED) {
		ret = -2;
		goto out;
	}

	dev->base = base;
	dev->fd = fd;

	/* Perform a reset */
	ret = vtc_reset(dev);
out:
	return ret;
}
