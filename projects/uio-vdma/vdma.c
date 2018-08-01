/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    vdma.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    Jan 16, 2018
 * @brief   AXI VDMA Userspace I/O Driver
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
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#include "vdma.h"

#define AXIVDMA_MM2S_CR			(0x00000000U)	/**< MM2S control register */

#define MM2S_CR_IRQ_DLY_CNT		(0xFF000000U)
#define MM2S_CR_IRQ_FRM_CNT		(0x00FF0000U)
#define MM2S_CR_REPEAT_EN		(1 << 15)	/**< Repeat or advance frame enable */
#define MM2S_CR_IRQ_ERR_EN		(1 << 14)	/**< Interrupt on error enable */
#define MM2S_CR_IRQ_DLY_CNT_EN		(1 << 13)	/**< Interrupt on delay count enable */
#define MM2S_CR_IRQ_FRM_CNT_EN		(1 << 12)	/**< Interrupt frame count complete enable */
#define MM2S_CR_RD_PTR_NUM		(0x00000F00U)
#define MM2S_CR_GEN_LOCK_SRC		(1 << 7)
#define MM2S_CR_FRM_CNT_EN		(1 << 4)
#define MM2S_CR_GEN_LOCK_EN		(1 << 3)
#define MM2S_CR_RESET			(1 << 2)
#define MM2S_CR_CIRC_PARK		(1 << 1)
#define MM2S_CR_RUN_STOP		(1 << 0)

#define AXIVDMA_MM2S_SR			(0x00000004U)	/**< MM2S status register */

#define MM2S_SR_IRQ_DLY_CNT_STS		(0xFF000000U)	/**< Interrupt delay count status */
#define MM2S_SR_IRQ_FRM_CNT_STS		(0x00FF0000U)	/**< Interrupt frame count status */
#define MM2S_SR_IRQ_ERR			(1 << 14)	/**< Interrupt on error */
#define MM2S_SR_IRQ_DLY_CNT		(1 << 13)	/**< Interrupt on delay */
#define MM2S_SR_IRQ_FRM_CNT		(1 << 12)	/**< Frame count interrupt */
#define MM2S_SR_ERR_SOF_ERLY		(1 << 7)	/**< Start of frame early error */
#define MM2S_SR_ERR_VDMA_DEC		(1 << 6)	/**< VDMA decode error */
#define MM2S_SR_ERR_VDMA_SLV		(1 << 5)	/**< VDMA slave error */
#define MM2S_SR_ERR_VDMA_INT		(1 << 4)	/**< VDMA internal error */
#define MM2S_SR_HALTED			(1 << 0)	/**< VDMA channel halted */

#define AXIVDMA_MM2S_INDEX		(0x00000014U)	/**< MM2S register index */
#define AXIVDMA_PARK_PTR		(0x00000028U)	/**< Park pointer */
#define AXIVDMA_VERSION			(0x0000002CU)	/**< Video DMA version */
#define AXIVDMA_S2MM_CR			(0x00000030U)	/**< S2MM control register */
#define AXIVDMA_S2MM_SR			(0x00000034U)	/**< S2MM status register */
#define AXIVDMA_S2MM_IRQ_MASK		(0x0000003CU)	/**< S2MM error interrupt mask */
#define AXIVDMA_S2MM_INDEX		(0x00000044U)	/**< S2MM register index */
#define AXIVDMA_MM2S_VSIZE		(0x00000050U)	/**< MM2S vertical size */
#define AXIVDMA_MM2S_HSIZE		(0x00000054U)	/**< MM2S horizontal size */
#define AXIVDMA_MM2S_FRMDLYSTRD		(0x00000058U)	/**< MM2S frame delay and stride */
#define AXIVDMA_MM2S_START_ADDRESS	(0x0000005CU)	/**< MM2S start address */
#define AXIVDMA_S2MM_VSIZE		(0x000000A0U)	/**< S2MM vertical size */
#define AXIVDMA_S2MM_HSIZE		(0x000000A4U)	/**< S2MM horizontal size */
#define AXIVDMA_S2MM_FRMDLYSTRD		(0x000000A8U)	/**< S2MM frame delay and stride */
#define AXIVDMA_S2MM_START_ADDRESS	(0x000000ACU)	/**< S2MM start address */

#define REG_READ(__BASE__, __OFFSET__)					\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__)))

#define REG_WRITE(__BASE__, __OFFSET__, __DATA__)				\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__))) = (__DATA__)


/**
 * From AXI Video Direct Memory Access v6.2 - PG020
 *
 * Programming Sequence
 *
 * AXI VDMA operations begin with the setup of the video parameter and start address
 * registers and the VDMA control registers. The following lists the minimum steps, in order,
 * required to start AXI VDMA operations:
 *
 *   1. Write control information to the channel VDMACR register (Offset 0x00 for MM2S and
 *      0x30 for S2MM) to set interrupt enables if desired, and set VDMACR.RS=1 to start the
 *      AXI VDMA channel running.
 *
 *   2. Write a valid video frame buffer start address to the channel START_ADDRESS register 1
 *      to N where N equals Frame Buffers (Offset 0x5C up to 0x98 for MM2S and 0xAC up to
 *      0xE8 for S2MM). Set the REG_INDEX register if required.
 *
 *      When AXI VDMA is configured for an address space greater than 32, each start address
 *      is to be programmed as a combination of two registers wherein the first register is used
 *      to specify LSB 32 bits of address while the next register is used to specify MSB 32 bits.
 *
 *   3. Write a valid Frame Delay (valid only for Genlock Slave) and Stride to the channel
 *      FRMDLY_STRIDE register (Offset 0x58 for MM2S and 0xA8 for S2MM).
 *
 *   4. Write a valid Horizontal Size to the channel HSIZE register (Offset 0x54 for MM2S and
 *      0xA4 for S2MM).
 *
 *   5. Write a valid Vertical Size to the channel VSIZE register (Offset 0x50 for MM2S and
 *      0xA0 for S2MM). This starts the channel transferring video data.
 *
 * You should be able to update video parameter settings at any time while the engine is
 * running by writing new video parameters and a video start address through the AXI4-Lite
 * control interface. The newly written video transfer values take effect on the next frame
 * boundary after you write the vertical size register for the respective channel.
 * To update video parameters dynamically while AXI VDMA operations are ongoing, a similar
 * process to the start steps is needed.
 *
 *   1. Write the Frame Delay, Stride, and Horizontal Size in any order for the associated
 *      channel.
 *
 *   2. Write the Vertical Size. When VSize is written, the video register values are transferred to
 *      an internal register block. On the next frame boundary the VDMA controller for the
 *      associated channel starts transfers using the newly updated values.
 *
 * Interrupts
 *
 * An interrupt output is provided for each channel (MM2S and S2MM). This output drives
 * High when there is an error if the error interrupt is enabled.
 *
 */

int
vdma_config(AxiVdma *vdma)
{
	int en;
	uint32_t cr;

	/* Set up MM2S control register */
//	cr = REG_READ(vdma->base, AXIVDMA_MM2S_CR);
//	cr |=  MM2S_CR_RUN_STOP |		/* Run channel */
//		MM2S_CR_GEN_LOCK_EN |		/* Enable GenLock */
//		MM2S_CR_GEN_LOCK_SRC |		/* Internal GenLock */
//		MM2S_CR_CIRC_PARK |		/* Circular mode */
//		MM2S_CR_REPEAT_EN;		/* Repeat on error */
//	cr &= ~(MM2S_CR_FRM_CNT_EN |		/* Don't limit to number of frames */
//		MM2S_CR_IRQ_FRM_CNT_EN);
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_CR, cr);

	/* Set up S2MM control register */
	cr = REG_READ(vdma->base, AXIVDMA_S2MM_CR);
	cr |=  MM2S_CR_RUN_STOP |		/* Run channel */
//		MM2S_CR_GEN_LOCK_EN |		/* Enable GenLock */
//		MM2S_CR_GEN_LOCK_SRC |		/* Internal GenLock */
//		MM2S_CR_CIRC_PARK |		/* Circular mode */
		MM2S_CR_FRM_CNT_EN |		/* Frame count on */
		MM2S_CR_IRQ_FRM_CNT_EN |	/* Interrupt on frame count */
		(3 << 16) |			/* Number of frames to count */
		MM2S_CR_REPEAT_EN;		/* Repeat on error */
//	cr &= ~(MM2S_CR_FRM_CNT_EN |		/* Don't limit to number of frames */
//		MM2S_CR_IRQ_FRM_CNT_EN);
	REG_WRITE(vdma->base, AXIVDMA_S2MM_CR, cr);

	/* Write frame buffer addresses */
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_START_ADDRESS, 0x30000000);
	REG_WRITE(vdma->base, AXIVDMA_S2MM_START_ADDRESS, 0x30000000);
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_START_ADDRESS + 4, 0x31000000);
	REG_WRITE(vdma->base, AXIVDMA_S2MM_START_ADDRESS + 4, 0x31000000);
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_START_ADDRESS + 8, 0x32000000);
	REG_WRITE(vdma->base, AXIVDMA_S2MM_START_ADDRESS + 8, 0x32000000);

	/* Write horizontal size (in bytes) */
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_HSIZE, 3840);
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_FRMDLYSTRD, 3840);

	REG_WRITE(vdma->base, AXIVDMA_S2MM_HSIZE, 3840);
	REG_WRITE(vdma->base, AXIVDMA_S2MM_FRMDLYSTRD, 3840);

	/* Arm the interrupt */
	en = 1;
	write(vdma->fd, &en, sizeof(int));

	/* This starts the transfer */
	REG_WRITE(vdma->base, AXIVDMA_S2MM_VSIZE, 720);
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_VSIZE, 720);

	return 0;
}


int
vdma_init(AxiVdma *vdma)
{
	int fd, ret;
	void *base;
	uint32_t cr;

	if (vdma == NULL)
		return -1;

	fd = open("/dev/uio1", O_RDWR);
	if (fd < 0)
		return fd;

	base = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (base == MAP_FAILED) {
		ret = -2;
		goto out;
	}

	vdma->base = base;
	vdma->fd = fd;

	/* Set up MM2S control register */
//	cr = REG_READ(vdma->base, AXIVDMA_MM2S_CR);
//	cr &= ~MM2S_CR_RUN_STOP;
//	cr |= MM2S_CR_RESET;
//	REG_WRITE(vdma->base, AXIVDMA_MM2S_CR, cr);

	cr = REG_READ(vdma->base, AXIVDMA_S2MM_CR);
	cr &= ~MM2S_CR_RUN_STOP;
	cr |= MM2S_CR_RESET;
	REG_WRITE(vdma->base, AXIVDMA_S2MM_CR, cr);

	usleep(1000 * 5);
out:
	return ret;
}




