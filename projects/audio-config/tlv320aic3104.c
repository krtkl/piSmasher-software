/**
 *******************************************************************************
 *******************************************************************************
 *
/ * @file	tlv320aic3104.c
 * @author	R. Bush
 * @email	bush@krtkl.com
 * @version	v1.0
 * @date	2017 September 28
 * @brief	TLV320AIC3104 Audio Codec
 * @license	FreeBSD
 *
 *******************************************************************************
 *
 * Copyright (c) 2017, krtkl inc.
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


//#include <stdio.h>
//#include <stdint.h>
//#include <unistd.h>

//#include "i2c.h"

#include "tlv320aic3104.h"


#define PLL_PROGA_PVAL_MASK			(0x07U)
#define PLL_PROGA_QVAL_MASK			(0x78U)
#define PLL_PROGA_ENABLE			(1 << 7)

#define PLL_PROGB_JVAL_MASK			(0xFCU)

#define PLL_PROGC_DVAL_MSB_MASK			(0xFFU)

#define PLL_PROGD_DVAL_LSB_MASK			(0xFCU)





enum aic3x_page {
	AIC3X_P0 = 0x00,
	AIC3X_P1 = 0x01,
	AIC3X_PNONE = 0xFF,
};

enum aic3x_reg {
	AIC3X_P0_PAGE_SELECT =			PAGE_REG_ADDR(AIC3X_P0, 0),
	AIC3X_P0_RESET =			PAGE_REG_ADDR(AIC3X_P0, 1),
	AIC3X_P0_SAMPLE_RATE_SEL =		PAGE_REG_ADDR(AIC3X_P0, 2),

	AIC3X_P0_PLL_PROGA =			PAGE_REG_ADDR(AIC3X_P0, 3),
	AIC3X_P0_PLL_PROGB =			PAGE_REG_ADDR(AIC3X_P0, 4),
	AIC3X_P0_PLL_PROGC =			PAGE_REG_ADDR(AIC3X_P0, 5),
	AIC3X_P0_PLL_PROGD =			PAGE_REG_ADDR(AIC3X_P0, 6),

	AIC3X_P0_CODEC_DATAPATH =		PAGE_REG_ADDR(AIC3X_P0, 7),

	/* Audio serial data interface control */
	AIC3X_P0_ASD_INTF_CTRLA =		PAGE_REG_ADDR(AIC3X_P0, 8),
	AIC3X_P0_ASD_INTF_CTRLB =		PAGE_REG_ADDR(AIC3X_P0, 9),
	AIC3X_P0_ASD_INTF_CTRLC = 		PAGE_REG_ADDR(AIC3X_P0, 10),

	/* Audio overflow status and PLL R value programming register */
	AIC3X_OVRF_STATUS_AND_PLLR = 		PAGE_REG_ADDR(AIC3X_P0, 11),

	/* Audio codec digital filter control register */
	AIC3X_CODEC_DFILT_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 12),

	/* Headset/button press detection register */
	AIC3X_HEADSET_DETECT_CTRL_A = 		PAGE_REG_ADDR(AIC3X_P0, 13),
	AIC3X_HEADSET_DETECT_CTRL_B = 		PAGE_REG_ADDR(AIC3X_P0, 14),

	/* ADC PGA Gain control registers */
	AIC3X_P0_LADC_VOL = 			PAGE_REG_ADDR(AIC3X_P0, 15),
	AIC3X_P0_RADC_VOL = 			PAGE_REG_ADDR(AIC3X_P0, 16),

	/* MIC2 control registers */
	AIC3X_P0_MIC2LR_2_LADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 17),
	AIC3X_P0_MIC2LR_2_RADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 18),

	/* Line1 Input control registers */
	AIC3X_P0_LINE1L_2_LADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 19),
	AIC3X_P0_LINE1R_2_LADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 21),
	AIC3X_P0_LINE1R_2_RADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 22),
	AIC3X_P0_LINE1L_2_RADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 24),

	/* Line2 Input control registers */
	AIC3X_P0_LINE2L_2_LADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 20),
	AIC3X_P0_LINE2R_2_RADC_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 23),

	/* MICBIAS Control Register */
	AIC3X_P0_MICBIAS_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 25),

	/* AGC Control Registers A, B, C */
	AIC3X_P0_LAGC_CTRL_A = 			PAGE_REG_ADDR(AIC3X_P0, 26),
	AIC3X_P0_LAGC_CTRL_B = 			PAGE_REG_ADDR(AIC3X_P0, 27),
	AIC3X_P0_LAGC_CTRL_C = 			PAGE_REG_ADDR(AIC3X_P0, 28),
	AIC3X_P0_RAGC_CTRL_A = 			PAGE_REG_ADDR(AIC3X_P0, 29),
	AIC3X_P0_RAGC_CTRL_B = 			PAGE_REG_ADDR(AIC3X_P0, 30),
	AIC3X_P0_RAGC_CTRL_C = 			PAGE_REG_ADDR(AIC3X_P0, 31),

	/* DAC Power and Left High Power Output control registers */
	AIC3X_P0_DAC_PWR = 			PAGE_REG_ADDR(AIC3X_P0, 37),
	AIC3X_P0_HPLCOM_CFG = 			PAGE_REG_ADDR(AIC3X_P0, 37),

	/* Right High Power Output control registers */
	AIC3X_P0_HPRCOM_CFG = 			PAGE_REG_ADDR(AIC3X_P0, 38),

	/* High Power Output Stage Control Register */
	AIC3X_P0_HPOUT_SC = 			PAGE_REG_ADDR(AIC3X_P0, 40),

	/* DAC Output Switching control registers */
	AIC3X_P0_DAC_LINE_MUX = 		PAGE_REG_ADDR(AIC3X_P0, 41),

	/* High Power Output Driver Pop Reduction registers */
	AIC3X_P0_HPOUT_POP_REDUCTION = 		PAGE_REG_ADDR(AIC3X_P0, 42),

	/* DAC Digital control registers */
	AIC3X_P0_LDAC_VOL = 			PAGE_REG_ADDR(AIC3X_P0, 43),
	AIC3X_P0_RDAC_VOL = 			PAGE_REG_ADDR(AIC3X_P0, 44),

	/* Left High Power Output control registers */
//	AIC3X_P0_LINE2L_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 45),
	AIC3X_P0_PGAL_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 46),
	AIC3X_P0_DACL1_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 47),
	AIC3X_P0_LINE2R_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 48),
	AIC3X_P0_PGAR_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 49),
	AIC3X_P0_DACR1_2_HPLOUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 50),
	AIC3X_P0_HPLOUT_CTRL =			PAGE_REG_ADDR(AIC3X_P0, 51),

	/* Left High Power COM control registers */
	AIC3X_P0_LINE2L_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 52),
	AIC3X_P0_PGAL_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 53),
	AIC3X_P0_DACL1_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 54),
	AIC3X_P0_LINE2R_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 55),
	AIC3X_P0_PGAR_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 56),
	AIC3X_P0_DACR1_2_HPLCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 57),
	AIC3X_P0_HPLCOM_CTRL =			PAGE_REG_ADDR(AIC3X_P0, 58),

	/* Right High Power Output control registers */
	AIC3X_P0_LINE2L_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 59),
	AIC3X_P0_PGAL_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 60),
	AIC3X_P0_DACL1_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 61),
	AIC3X_P0_LINE2R_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 62),
	AIC3X_P0_PGAR_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 63),
	AIC3X_P0_DACR1_2_HPROUT_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 64),
	AIC3X_P0_HPROUT_CTRL = 			PAGE_REG_ADDR(AIC3X_P0, 65),

	/* Right High Power COM control registers */
	AIC3X_P0_LINE2L_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 66),
	AIC3X_P0_PGAL_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 67),
	AIC3X_P0_DACL1_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 68),
	AIC3X_P0_LINE2R_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 69),
	AIC3X_P0_PGAR_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 70),
	AIC3X_P0_DACR1_2_HPRCOM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 71),
	AIC3X_P0_HPRCOM_CTRL = 			PAGE_REG_ADDR(AIC3X_P0, 72),

	/* Mono Line Output Plus/Minus control registers */
	AIC3X_P0_LINE2L_2_MONOLOPM_VOL = 	PAGE_REG_ADDR(AIC3X_P0, 73),
	AIC3X_P0_PGAL_2_MONOLOPM_VOL =		PAGE_REG_ADDR(AIC3X_P0, 74),
	AIC3X_P0_DACL1_2_MONOLOPM_VOL = 	PAGE_REG_ADDR(AIC3X_P0, 75),
	AIC3X_P0_LINE2R_2_MONOLOPM_VOL = 	PAGE_REG_ADDR(AIC3X_P0, 76),
	AIC3X_P0_PGAR_2_MONOLOPM_VOL =		PAGE_REG_ADDR(AIC3X_P0, 77),
	AIC3X_P0_DACR1_2_MONOLOPM_VOL = 	PAGE_REG_ADDR(AIC3X_P0, 78),
	AIC3X_P0_MONOLOPM_CTRL = 		PAGE_REG_ADDR(AIC3X_P0, 79),

	/* Class-D speaker driver on tlv320aic3007 */
//#define CLASSD_CTRL			73

	/* Left Line Output Plus/Minus control registers */
	AIC3X_P0_LINE2L_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 80),
	AIC3X_P0_PGAL_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 81),
	AIC3X_P0_DACL1_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 82),
	AIC3X_P0_LINE2R_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 83),
	AIC3X_P0_PGAR_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 84),
	AIC3X_P0_DACR1_2_LLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 85),
	AIC3X_P0_LLOPM_CTRL = 			PAGE_REG_ADDR(AIC3X_P0, 86),

	/* Right Line Output Plus/Minus control registers */
	AIC3X_P0_LINE2L_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 87),
	AIC3X_P0_PGAL_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 88),
	AIC3X_P0_DACL1_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 89),
	AIC3X_P0_LINE2R_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 90),
	AIC3X_P0_PGAR_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 91),
	AIC3X_P0_DACR1_2_RLOPM_VOL = 		PAGE_REG_ADDR(AIC3X_P0, 92),
	AIC3X_P0_RLOPM_CTRL = 			PAGE_REG_ADDR(AIC3X_P0, 93),

	/* GPIO/IRQ registers */
	AIC3X_P0_STICKY_IRQ_FLAGS_REG = 	PAGE_REG_ADDR(AIC3X_P0, 96),
	AIC3X_P0_RT_IRQ_FLAGS_REG = 		PAGE_REG_ADDR(AIC3X_P0, 97),
	AIC3X_P0_GPIO1_REG = 			PAGE_REG_ADDR(AIC3X_P0, 98),
	AIC3X_P0_GPIO2_REG = 			PAGE_REG_ADDR(AIC3X_P0, 99),
	AIC3X_P0_GPIOA_REG = 			PAGE_REG_ADDR(AIC3X_P0, 100),
	AIC3X_P0_GPIOB_REG = 			PAGE_REG_ADDR(AIC3X_P0, 101),

	/* Clock generation control register */
	AIC3X_P0_CLKGEN_CTRL_REG = 		PAGE_REG_ADDR(AIC3X_P0, 102),

	/* New AGC registers */
	AIC3X_P0_LAGCN_ATTACK = 		PAGE_REG_ADDR(AIC3X_P0, 103),
	AIC3X_P0_LAGCN_DECAY = 			PAGE_REG_ADDR(AIC3X_P0, 104),
	AIC3X_P0_RAGCN_ATTACK = 		PAGE_REG_ADDR(AIC3X_P0, 105),
	AIC3X_P0_RAGCN_DECAY = 			PAGE_REG_ADDR(AIC3X_P0, 106),

	/* New Programmable ADC Digital Path and I2C Bus Condition Register */
	AIC3X_P0_NEW_ADC_DIGITALPATH = 		PAGE_REG_ADDR(AIC3X_P0, 107),

	/* Passive Analog Signal Bypass Selection During Powerdown Register */
	AIC3X_P0_PASSIVE_BYPASS = 		PAGE_REG_ADDR(AIC3X_P0, 108),

	/* DAC Quiescent Current Adjustment Register */
	AIC3X_P0_DAC_ICC_ADJ = 			PAGE_REG_ADDR(AIC3X_P0, 109)
};




static enum aic3x_reg lineout[] = {
	AIC3X_P0_DACL1_2_LLOPM_VOL,
	AIC3X_P0_DACR1_2_RLOPM_VOL,
	AIC3X_P0_LLOPM_CTRL,
	AIC3X_P0_RLOPM_CTRL,
	AIC3X_P0_PGAL_2_LLOPM_VOL,
	AIC3X_P0_PGAR_2_RLOPM_VOL,
};

static enum aic3x_reg headout[] = {
	AIC3X_P0_DACL1_2_HPLOUT_VOL,
	AIC3X_P0_DACR1_2_HPROUT_VOL,
	AIC3X_P0_HPLOUT_CTRL,
	AIC3X_P0_HPROUT_CTRL,
	AIC3X_P0_PGAL_2_HPLOUT_VOL,
	AIC3X_P0_PGAR_2_HPROUT_VOL
};

static int
write_page(struct aic3x_dev *codec, enum aic3x_page page)
{
	int ret;
	uint8_t reg_val;

	reg_val = (uint8_t) page;
	ret = codec->cfg->i2c_write(codec->cfg->sl_addr, 0x00, &reg_val);
	if (ret != 0)
		return ret;

	codec->cur_page = (uint8_t) page;

	return ret;
}

static int
read_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t *data)
{
	enum aic3x_page page = (reg >> 8) & 0x00ff;

	if (codec->cur_page != (uint8_t) page) {
		write_page(codec, page);
	}

	return codec->cfg->i2c_read(codec->cfg->sl_addr, (uint8_t)(reg & 0x00ff), data);
}

static int
write_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t data)
{
	int ret;
	enum aic3x_page page = (reg >> 8) & 0x00ff;
	uint8_t tmp = data;

	if (codec->cur_page != (uint8_t) page) {
		write_page(codec, page);
	}

	ret = codec->cfg->i2c_write(codec->cfg->sl_addr, (uint8_t)(reg & 0x00ff), &tmp);

	return ret;
}

static int
write_reg_mask(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t mask, uint8_t data)
{
	int err;
	uint8_t tmp;

	err = read_reg(codec, reg, &tmp);
	if (err != 0)
		return err;

	tmp &= ~(mask);
	tmp |= (data & mask);

	return write_reg(codec, reg, tmp);
}

static int
aic3x_set_micbias(struct aic3x_dev *codec, enum aic3x_micbias micbias)
{
	int ret;
	uint8_t reg_val;

	reg_val = ((uint8_t)(micbias) << AIC3X_MICBIAS_LEVEL_SHIFT) & AIC3X_MICBIAS_LEVEL_MASK;

	ret = write_reg(codec, AIC3X_P0_MICBIAS_CTRL, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static int
aic3x_configure_clock_source(struct aic3x_dev *codec, enum aic3x_clkgen_ctrl clk_in, enum aic3x_clkgen_ctrl_source source)
{
	uint8_t mask, reg_val;

	mask = CLKGEN_CTRL_SOURCE_MASK << clk_in;
	reg_val = source << clk_in;

	return write_reg_mask(codec, AIC3X_P0_CLKGEN_CTRL_REG, mask, reg_val);
}

/**
 * @brief	Calculate and Configure PLL Values
 */
static int
aic3x_configure_pll(struct aic3x_dev *codec)
{
	int ret;
	uint8_t reg_val;
	uint16_t pll_p, pll_r, pll_j, pll_d;

	pll_p = 1;
	pll_r = 2;
	pll_j = 8;
	pll_d = 1920;

	ret = write_reg_mask(codec,
			AIC3X_P0_PLL_PROGA,
			PLL_PROGA_ENABLE | PLL_PROGA_PVAL_MASK,
			PLL_PROGA_ENABLE | pll_p);
	if (ret < 0)
		return ret;

	/* Set PLL 'J' value */
	reg_val = (uint8_t) (pll_j << 2);
	ret = write_reg(codec, AIC3X_P0_PLL_PROGB, reg_val);
	if (ret < 0)
		return ret;

	/* Set D value MSBs and LSBs in PROGC and PROGD registers, respectively */
	reg_val = (uint8_t) ((pll_d >> 6) & PLL_PROGC_DVAL_MSB_MASK);
	ret = write_reg(codec, AIC3X_P0_PLL_PROGC, reg_val);
	if (ret < 0)
		return ret;

	reg_val = (uint8_t) ((pll_d << 2) & PLL_PROGD_DVAL_LSB_MASK);
	ret = write_reg(codec, AIC3X_P0_PLL_PROGD, 0x20);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * @brief	Initialize AIC3x Codec
 */
int
aic3x_init(struct aic3x_dev *codec, struct aic3x_cfg *cfg)
{
	int ret;
	uint8_t reg_val;
	enum aic3x_reg *reg;

	if ((codec == NULL) || (cfg == NULL))
		return (-1);

	codec->cfg = cfg;					/* Attach configuration to codec */

	ret = aic3x_reset(codec);
	if (ret != 0)
		return ret;

//	ret = aic3x_configure_clock_source(codec, CLKGEN_CTRL_PLLCLK_IN, CLKGEN_SOURCE_BCLK);
//	if (ret != 0)
//		return ret;
//
//	ret = aic3x_configure_clock_source(codec, CLKGEN_CTRL_CLKDIV_IN, CLKGEN_SOURCE_BCLK);
//	if (ret != 0)
//		return ret;

	/* Configure clock source */
	reg_val = 0x02U;
	ret = write_reg(codec, AIC3X_P0_CLKGEN_CTRL_REG, reg_val);
	if (ret < 0)
		return ret;

	ret = aic3x_configure_pll(codec);
	if (ret < 0)
		return ret;

	/* Configure master/slave mode */
	ret = write_reg(codec,
			AIC3X_P0_ASD_INTF_CTRLA,
			0xC0U);
	if (ret < 0)
		return ret;

	/* Configure inputs */
	if (cfg->in_route == AIC3X_MIC_IN_MONO) {

		/* Route MIC2R to RADC (-3dB input level control gain)*/
		ret = write_reg_mask(codec, AIC3X_P0_MIC2LR_2_RADC_CTRL, 0x0f, 0x00);
		if (ret != 0)
			return ret;

		ret = aic3x_set_micbias(codec, AIC3X_MICBIAS_2V);
		if (ret < 0)
			return ret;

	} else {
		ret = write_reg_mask(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, 0x78, 0x10);
		if (ret != 0)
			return ret;

		ret = write_reg_mask(codec, AIC3X_P0_LINE1L_2_LADC_CTRL, 0x78, 0x10);
		if (ret != 0)
			return ret;
	}

	/* Power up the right ADC */
	ret = write_reg_mask(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, (1 << 2), (1 << 2));
	if (ret != 0)
		return ret;

	if (cfg->in_route == AIC3X_LINE_IN_STEREO) {
		ret = write_reg_mask(codec, AIC3X_P0_LINE1L_2_LADC_CTRL, (1 << 2), (1 << 2));
		if (ret != 0)
			return ret;
	}

//	ret = write_reg(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, 0x7C);
//	if (ret != 0)
//		return ret;

	/* ADC default volume and unmute */
	ret = write_reg(codec, AIC3X_P0_RADC_VOL, cfg->in_gain);
	if (ret != 0)
		return ret;

	ret = write_reg_mask(codec, AIC3X_P0_RADC_VOL, (1 << 7), 0);
	if (ret != 0)
		return ret;

	if (cfg->in_route == AIC3X_LINE_IN_STEREO) {

		ret = write_reg(codec, AIC3X_P0_LADC_VOL, cfg->in_gain);
		if (ret != 0)
			return ret;

		ret = write_reg_mask(codec, AIC3X_P0_LADC_VOL, (1 << 7), 0);
		if (ret != 0)
			return ret;
	}


//	if (cfg->in_route == AIC3X_MIC_IN_MONO) {
//		ret = aic3x_set_micbias(codec, AIC3X_MICBIAS_2V);
//		if (ret != 0)
//			return ret;
//	}

	/* Set the reference sample frequency */
	ret = aic3x_configure_fref(codec, DATAPATH_FREF_48kHz);
	if (ret != 0)
		return ret;

	/* Route left channel to LDAC, route right channel to RDAC */
	ret = aic3x_configure_datapath(codec, DATAPATH_RDAC, DATAPATH_STRAIGHT);
	if (ret != 0)
		return ret;

	ret = aic3x_configure_datapath(codec, DATAPATH_LDAC, DATAPATH_STRAIGHT);
	if (ret != 0)
		return ret;

	/* Configure DAC connections */

	/* Power up Left and Right DACs */
	ret = aic3x_enable_dac_pwr(codec, DAC_PWR_RDAC, 1);
	if (ret != 0)
		return ret;

	ret = aic3x_enable_dac_pwr(codec, DAC_PWR_LDAC, 1);
	if (ret != 0)
		return ret;

	/* DAC default volume and unmute */
	ret = write_reg(codec, AIC3X_P0_LDAC_VOL, cfg->out_gain);
	if (ret != 0)
		return ret;

	ret = write_reg(codec, AIC3X_P0_RDAC_VOL, cfg->out_gain);
	if (ret != 0)
		return ret;

	/* Left Line Output Plus/Minus control registers */
	switch (cfg->out_route) {
	case AIC3X_LINE_OUT_STEREO: reg = lineout; break;
	case AIC3X_HP_OUT_STEREO: reg = headout; break;
	}

	/* DAC to output default volume and route to output mixer */
	ret = write_reg(codec, reg[0], cfg->out_gain | AIC3X_ROUTE_ON);
	if (ret != 0)
		return ret;

	ret = write_reg(codec, reg[1], cfg->out_gain | AIC3X_ROUTE_ON);
	if (ret != 0)
		return ret;

	/* Unmute outputs and power on */
	ret = write_reg_mask(codec, reg[2], AIC3X_UNMUTE | AIC3X_POWER_ON, AIC3X_UNMUTE | AIC3X_POWER_ON);
	if (ret != 0)
		return ret;

	ret = write_reg_mask(codec, reg[3], AIC3X_UNMUTE | AIC3X_POWER_ON, AIC3X_UNMUTE | AIC3X_POWER_ON);
	if (ret != 0)
		return ret;

	/* PGA to HP Bypass default volume */
	ret = write_reg(codec, reg[4], cfg->out_gain);
	if (ret != 0)
		return ret;

	ret = write_reg(codec, reg[5], cfg->out_gain);
	if (ret != 0)
		return ret;

	return (0);
}

int
aic3x_reset(struct aic3x_dev *codec)
{
	/* Write page 0 reset register with the soft reset */
	return write_reg(codec, AIC3X_P0_RESET, AIC3X_SOFT_RESET);
}

int
aic3x_configure_datapath(struct aic3x_dev *codec, enum aic3x_datapath_dac dac, enum aic3x_datapath_ctrl path)
{
	uint8_t mask, reg_val;

	mask = DATAPATH_CTRL_MASK << dac;
	reg_val = path << dac;

	return write_reg_mask(codec, AIC3X_P0_CODEC_DATAPATH, mask, reg_val);
}

int
aic3x_configure_data_interface(struct aic3x_dev *codec, struct aic3x_asd_if_cfg *asd_cfg)
{
	int err;
	uint8_t mask, reg_val;

	/* Configure master/slave mode */
	mask = ASD_IFA_BCLK_MASTER | ASD_IFA_WCLK_MASTER;
	reg_val = asd_cfg->mode << ASD_IFA_MODE_SHIFT;

	err = write_reg_mask(codec, AIC3X_P0_ASD_INTF_CTRLA, mask, reg_val);
	if (err != 0)
		return err;

	/* Configure data interface transfer mode and length */
	mask = ASD_IFB_MODE_MASK | ASD_IFB_LEN_MASK;
	reg_val = asd_cfg->mode << ASD_IFB_MODE_SHIFT;
	reg_val |= asd_cfg->data_len << ASD_IFB_LEN_SHIFT;

	return write_reg_mask(codec, AIC3X_P0_ASD_INTF_CTRLB, mask, reg_val);
}

int
aic3x_configure_fref(struct aic3x_dev *codec, enum aic3x_fref fref)
{
	return write_reg_mask(codec,
				AIC3X_P0_CODEC_DATAPATH,
				DATAPATH_FREF_MASK,
				(fref << 7));
}

int
aic3x_enable_dac_pwr(struct aic3x_dev *codec, enum aic3x_dac_pwr_dac dac, uint8_t en)
{
	return write_reg_mask(codec, AIC3X_P0_DAC_PWR, (uint8_t)(1 << dac), en ? (uint8_t)(1 << dac) : 0);
}

int
aic3x_set_sample_rate(struct aic3x_dev *codec, enum aic3x_rate_div adc_div, enum aic3x_rate_div dac_div)
{
	return write_reg(codec, AIC3X_P0_SAMPLE_RATE_SEL, ((uint8_t)adc_div << 4) | (uint8_t)dac_div);
}
