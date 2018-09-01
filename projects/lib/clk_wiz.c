/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    clk_wiz.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    August 6, 2018
 * @brief   Clocking Wizard Userspace I/O Driver
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

#include "clk_wiz.h"

#define ERR_MODE_INVALID		(0x0100U)
#define ERR_NULL_PARAM			(0x0101U)

#define CWIZ_REG_SOFT_RESET		(0x00000000U)	/**< Software reset */
#define CWIZ_REG_STATUS			(0x00000004U)	/**< Status */
#define CWIZ_REG_MON_ERR		(0x00000008U)	/**< Clock monitor error status */
#define CWIZ_REG_INT_STATUS		(0x0000000CU)	/**< Interrupt status */
#define CWIZ_REG_INT_ENABLE		(0x00000010U)	/**< Interrupt enable */

#define CWIZ_REG_CLK_CONFIG		(0x00000200U)	/**< Clock configuration */
#define CWIZ_REG_CLK_FBPHASE		(0x00000204U)	/**< Clock phase */

#define CWIZ_REG_CLKOUT0_DIVIDE		(0x00000208U)	/**< Clock out 0 divide */
#define CWIZ_REG_CLKOUT0_PHASE		(0x0000020CU)	/**< Clock out 0 phase */
#define CWIZ_REG_CLKOUT0_DUTY		(0x00000210U)	/**< Clock out 0 duty cycle */

#define CWIZ_REG_CONFIG_LOAD		(0x0000025CU)	/**< Clock configuration register 23 */

#define SOFT_RESET_VAL			(0x0000000AU)	/**< Software reset value */

#define STATUS_LOCKED			(1 << 0)	/**< MMCM/PLL locked status */

#define DIVCLK_DIVIDE_MASK		(0x000000FFU)	/**< Divide value for all output clocks */
#define CLKFBOUT_MULT_MASK		(0x0000FF00U)	/**< Multiplier integer value */
#define CLKFBOUT_FRAC_MASK		(0x03FF0000U)	/**< Multiplier fractional value */

#define CLKOUT_DIVIDE_MASK		(0x000000FFU)	/**< Clock out integer divide value */
#define CLKOUT_FRAC_SHIFT		(8)
#define CLKOUT_FRAC_MASK		(0x0003FF00U)	/**< Clock out fractional divide value */

#define CONFIG_LOAD_SEN			(1 << 0)
#define CONFIG_SADDR			(1 << 1)

#define REG_READ(__BASE__, __OFFSET__)						\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__)))

#define REG_WRITE(__BASE__, __OFFSET__, __DATA__)				\
		*((volatile uint32_t *)(((uint8_t *)(__BASE__)) + (__OFFSET__))) = (__DATA__)


//	/* Configuring Multiply and Divide values */
//	*(u32 *)(CfgPtr_Dynamic->BaseAddr + CWIZ_REG_CLK_DIVIDE) = Frac_en | (CLKOUT_FRAC_SHIFT << 8) | (Divide);
//	*(u32 *)(CfgPtr_Dynamic->BaseAddr + CWIZ_REG_CLKOUT0_PHASE) = 0x00;
//
//	/* Load Clock Configuration Register values */
//	*(u32 *)(CfgPtr_Dynamic->BaseAddr + CWIZ_REG_CONFIG_LOAD) = 0x07;
//
//	if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + CWIZ_REG_STATUS) & STATUS_LOCKED) {
//		Error++;
//		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "
//			"0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
//	}
//
//	/* Clock Configuration Registers are used for dynamic reconfiguration */
//	*(u32 *)(CfgPtr_Dynamic->BaseAddr + CWIZ_REG_CONFIG_LOAD) = 0x02;

struct clk_cfg {
	uint8_t divide;
	uint8_t fbout_mult;
	uint16_t fbout_frac;
	uint8_t clkout_divide;
	uint16_t clkout_frac;
};


int
clk_wiz_config(struct clk_wiz_dev *dev, enum clk_wiz_mode mode)
{
	uint32_t mult, mult_frac, div, outdiv, outdiv_frac;
	uint32_t reg_val;

	switch (mode) {
	case CLK_WIZ_MODE_720p:
		mult = 50;
		mult_frac = 375;
		div = 5;
		outdiv = 20;
		outdiv_frac = 875;
		break;

	case CLK_WIZ_MODE_1080p:
		mult = 31;
		mult_frac = 250;
		div = 7;
		outdiv = 4;
		outdiv_frac = 625;
		break;

	case CLK_WIZ_MODE_WXGA:
		mult = 35;
		mult_frac = 500;
		div = 7;
		outdiv = 9;
		outdiv_frac = 125;
		break;

	default:
		printf("Unsupported clock mode %d\n", mode);
		return -1;
	}

	reg_val = (mult_frac << 16) | (mult << 8) | div;
	REG_WRITE(dev->base, 0x200, reg_val);

	reg_val = (outdiv_frac << 8) | outdiv;
	REG_WRITE(dev->base, 0x208, reg_val);

	reg_val = (1 << 0) | (1 << 1);
	REG_WRITE(dev->base, 0x25c, reg_val);

	return 0;
}

/**
 * @brief	Video Timing Controller Initialization
 */
int
clk_wiz_init(struct clk_wiz_dev *dev, const char *devname)
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
out:
	return ret;
}
