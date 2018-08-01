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



static int aic3x_read_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t *data);
static int aic3x_write_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t data);
static int aic3x_mask_write_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t mask, uint8_t data);
static int aic3x_set_micbias(struct aic3x_dev *codec, enum aic3x_micbias micbias);


/**
 * @brief	Initialize AIC3x Codec
 */
int aic3x_init(struct aic3x_dev *codec, struct aic3x_cfg *cfg)
{
	int err;
	enum aic3x_reg *reg;

	if ((codec == NULL) || (cfg == NULL))
		return (-1);

	codec->cfg = cfg;					/* Attach configuration to codec */

	err = aic3x_reset(codec);
	if (err != 0)
		return err;

	/* Set the clock generation source selections to use BCLK */
	err = aic3x_configure_clock_source(codec, CLKGEN_CTRL_PLLCLK_IN, CLKGEN_SOURCE_BCLK);
	if (err != 0)
		return err;

	err = aic3x_configure_clock_source(codec, CLKGEN_CTRL_CLKDIV_IN, CLKGEN_SOURCE_BCLK);
	if (err != 0)
		return err;

	err = aic3x_mask_write_reg(codec, AIC3X_P0_PLL_PROGA, (1 << 7) | 0x07, (1 << 7) | 0x01);
	if (err != 0)
		return err;

	/* Set PLL 'J' value */
	err = aic3x_write_reg(codec, AIC3X_P0_PLL_PROGB, 0x88);
	if (err != 0)
		return err;

	/* Set PLL 'D' value */
	err = aic3x_write_reg(codec, AIC3X_P0_PLL_PROGC, 0x02);
	if (err != 0)
		return err;

	err = aic3x_write_reg(codec, AIC3X_P0_PLL_PROGD, 0x20);
	if (err != 0)
		return err;


	/* Configure inputs */
	if (cfg->in_route == AIC3X_MIC_IN_MONO) {

		/* Route MIC2R to RADC (-3dB input level control gain)*/
		err = aic3x_mask_write_reg(codec, AIC3X_P0_MIC2LR_2_RADC_CTRL, 0x0f, 0x02);
		if (err != 0)
			return err;

	} else {
		err = aic3x_mask_write_reg(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, 0x78, 0x10);
		if (err != 0)
			return err;

		err = aic3x_mask_write_reg(codec, AIC3X_P0_LINE1L_2_LADC_CTRL, 0x78, 0x10);
		if (err != 0)
			return err;
	}

	/* Power up the right ADC */
	err = aic3x_mask_write_reg(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, (1 << 2), (1 << 2));
	if (err != 0)
		return err;

	if (cfg->in_route == AIC3X_LINE_IN_STEREO) {
		err = aic3x_mask_write_reg(codec, AIC3X_P0_LINE1L_2_LADC_CTRL, (1 << 2), (1 << 2));
		if (err != 0)
			return err;
	}

//	err = aic3x_write_reg(codec, AIC3X_P0_LINE1R_2_RADC_CTRL, 0x7C);
//	if (err != 0)
//		return err;

	/* ADC default volume and unmute */
	err = aic3x_write_reg(codec, AIC3X_P0_RADC_VOL, AIC3X_DEFAULT_GAIN);
	if (err != 0)
		return err;

	err = aic3x_write_reg(codec, AIC3X_P0_LADC_VOL, AIC3X_DEFAULT_GAIN);
	if (err != 0)
		return err;

	err = aic3x_mask_write_reg(codec, AIC3X_P0_RADC_VOL, (1 << 7), 0);
	if (err != 0)
		return err;

	err = aic3x_mask_write_reg(codec, AIC3X_P0_LADC_VOL, (1 << 7), 0);
	if (err != 0)
		return err;

//	if (cfg->in_route == AIC3X_MIC_IN_MONO) {
//		err = aic3x_set_micbias(codec, AIC3X_MICBIAS_2V);
//		if (err != 0)
//			return err;
//	}

	/* Set the reference sample frequency */
	err = aic3x_configure_fref(codec, DATAPATH_FREF_44_1kHz);
	if (err != 0)
		return err;

	/* Route left channel to LDAC, route right channel to RDAC */
	err = aic3x_configure_datapath(codec, DATAPATH_RDAC, DATAPATH_STRAIGHT);
	if (err != 0)
		return err;

	err = aic3x_configure_datapath(codec, DATAPATH_LDAC, DATAPATH_STRAIGHT);
	if (err != 0)
		return err;

	/* Configure DAC connections */

	/* Power up Left and Right DACs */
	err = aic3x_enable_dac_pwr(codec, DAC_PWR_RDAC, 1);
	if (err != 0)
		return err;

	err = aic3x_enable_dac_pwr(codec, DAC_PWR_LDAC, 1);
	if (err != 0)
		return err;

	/* DAC default volume and unmute */
	err = aic3x_write_reg(codec, AIC3X_P0_LDAC_VOL, AIC3X_DEFAULT_VOL);
	if (err != 0)
		return err;

	err = aic3x_write_reg(codec, AIC3X_P0_RDAC_VOL, AIC3X_DEFAULT_VOL);
	if (err != 0)
		return err;

	/* Left Line Output Plus/Minus control registers */
	switch (cfg->out_route) {
	case AIC3X_LINE_OUT_STEREO: reg = lineout; break;
	case AIC3X_HP_OUT_STEREO: reg = headout; break;
	}

	/* DAC to output default volume and route to output mixer */
	err = aic3x_write_reg(codec, reg[0], AIC3X_DEFAULT_VOL | AIC3X_ROUTE_ON);
	if (err != 0)
		return err;

	err = aic3x_write_reg(codec, reg[1], AIC3X_DEFAULT_VOL | AIC3X_ROUTE_ON);
	if (err != 0)
		return err;

	/* Unmute outputs and power on */
	err = aic3x_mask_write_reg(codec, reg[2], AIC3X_UNMUTE | AIC3X_POWER_ON, AIC3X_UNMUTE | AIC3X_POWER_ON);
	if (err != 0)
		return err;

	err = aic3x_mask_write_reg(codec, reg[3], AIC3X_UNMUTE | AIC3X_POWER_ON, AIC3X_UNMUTE | AIC3X_POWER_ON);
	if (err != 0)
		return err;

	/* PGA to HP Bypass default volume */
	err = aic3x_write_reg(codec, reg[4], AIC3X_DEFAULT_VOL);
	if (err != 0)
		return err;

	err = aic3x_write_reg(codec, reg[5], AIC3X_DEFAULT_VOL);
	if (err != 0)
		return err;

	return (0);
}


int aic3x_reset(struct aic3x_dev *codec)
{
	/* Write page 0 reset register with the soft reset */
	return aic3x_write_reg(codec, AIC3X_P0_RESET, AIC3X_SOFT_RESET);
}


int aic3x_configure_datapath(struct aic3x_dev *codec, enum aic3x_datapath_dac dac, enum aic3x_datapath_ctrl path)
{
	uint8_t mask, reg_val;

	mask = DATAPATH_CTRL_MASK << dac;
	reg_val = path << dac;

	return aic3x_mask_write_reg(codec, AIC3X_P0_CODEC_DATAPATH, mask, reg_val);
}


int aic3x_configure_clock_source(struct aic3x_dev *codec, enum aic3x_clkgen_ctrl clk_in, enum aic3x_clkgen_ctrl_source source)
{
	uint8_t mask, reg_val;

	mask = CLKGEN_CTRL_SOURCE_MASK << clk_in;
	reg_val = source << clk_in;

	return aic3x_mask_write_reg(codec, AIC3X_P0_CLKGEN_CTRL_REG, mask, reg_val);
}


int aic3x_configure_data_interface(struct aic3x_dev *codec, struct aic3x_asd_if_cfg *asd_cfg)
{
	int err;
	uint8_t mask, reg_val;

	/* Configure master/slave mode */
	mask = ASD_IFA_BCLK_MASTER | ASD_IFA_WCLK_MASTER;
	reg_val = asd_cfg->mode << ASD_IFA_MODE_SHIFT;

	err = aic3x_mask_write_reg(codec, AIC3X_P0_ASD_INTF_CTRLA, mask, reg_val);
	if (err != 0)
		return err;

	/* Configure data interface transfer mode and length */
	mask = ASD_IFB_MODE_MASK | ASD_IFB_LEN_MASK;
	reg_val = asd_cfg->mode << ASD_IFB_MODE_SHIFT;
	reg_val |= asd_cfg->data_len << ASD_IFB_LEN_SHIFT;

	return aic3x_mask_write_reg(codec, AIC3X_P0_ASD_INTF_CTRLB, mask, reg_val);
}


int aic3x_configure_fref(struct aic3x_dev *codec, enum aic3x_fref fref)
{
	return aic3x_mask_write_reg(codec,
					AIC3X_P0_CODEC_DATAPATH,
					DATAPATH_FREF_MASK,
					(fref << 7));
}


int aic3x_enable_dac_pwr(struct aic3x_dev *codec, enum aic3x_dac_pwr_dac dac, uint8_t en)
{
	return aic3x_mask_write_reg(codec, AIC3X_P0_DAC_PWR, (uint8_t)(1 << dac), en ? (uint8_t)(1 << dac) : 0);
}


int aic3x_set_sample_rate(struct aic3x_dev *codec, enum aic3x_rate_div adc_div, enum aic3x_rate_div dac_div)
{
	return aic3x_write_reg(codec, AIC3X_P0_SAMPLE_RATE_SEL, ((uint8_t)adc_div << 4) | (uint8_t)dac_div);
}


static int aic3x_set_micbias(struct aic3x_dev *codec, enum aic3x_micbias micbias)
{
	uint8_t reg_val;

	reg_val = ((uint8_t)(micbias) << AIC3X_MICBIAS_LEVEL_SHIFT) & AIC3X_MICBIAS_LEVEL_MASK;

	return aic3x_write_reg(codec, AIC3X_P0_MICBIAS_CTRL, reg_val);
}


static int aic3x_write_page(struct aic3x_dev *codec, enum aic3x_page page)
{
	int ret;
	uint8_t reg_val;

	reg_val = (uint8_t)page;
	ret = codec->cfg->i2c_write(codec->cfg->sl_addr, 0x00, &reg_val);
	if (ret != 0)
		return ret;

	codec->curr_page = page;

//	usleep(1000);

	return ret;
}


static int aic3x_read_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t *data)
{
	enum aic3x_page page = (reg >> 8) & 0x00ff;

	if (codec->curr_page != page) {
		aic3x_write_page(codec, page);
	}

	return codec->cfg->i2c_read(codec->cfg->sl_addr, (uint8_t)(reg & 0x00ff), data);
}


static int aic3x_write_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t data)
{
	int ret;
	enum aic3x_page page = (reg >> 8) & 0x00ff;
	uint8_t tmp = data;

	if (codec->curr_page != page) {
		aic3x_write_page(codec, page);
	}

	ret = codec->cfg->i2c_write(codec->cfg->sl_addr, (uint8_t)(reg & 0x00ff), &tmp);

	return ret;
}


static int aic3x_mask_write_reg(struct aic3x_dev *codec, enum aic3x_reg reg, uint8_t mask, uint8_t data)
{
	int err;
	uint8_t tmp;

	err = aic3x_read_reg(codec, reg, &tmp);
	if (err != 0)
		return err;

	tmp &= ~(mask);
	tmp |= (data & mask);

	return aic3x_write_reg(codec, reg, tmp);
}
