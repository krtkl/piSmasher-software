/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	tlv320aic3104.h
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

#ifndef __TLV320AIC3104_H
#define __TLV320AIC3104_H

#include <stdio.h>
#include <stdint.h>


#define AIC3X_ADDR				(0x18U)		/**< Fixed I2C slave address */

#define PAGE_REG_ADDR(__PAGE__, __ADDR__)	((((uint16_t)(__PAGE__)) << 8) | ((uint16_t)(__ADDR__) & 0x00FFU))

#define AIC3X_SOFT_RESET			(0x80U)

/* Codec Datapath Setup Register - Page 0, Reg 7 */
#define DATAPATH_FREF_MASK			(1 << 7)

enum aic3x_fref {
	DATAPATH_FREF_48kHz = 0x00,
	DATAPATH_FREF_44_1kHz = 0x01
};

#define DATAPATH_ADC_DUAL_RATE			(1 << 6)
#define DATAPATH_DAC_DUAL_RATE			(1 << 5)

#define DATAPATH_RDAC_SHIFT			(1)
#define DATAPATH_LDAC_SHIFT			(3)
#define DATAPATH_CTRL_MASK			(0x03U)

enum aic3x_datapath_dac {
	DATAPATH_RDAC = 0x01,
	DATAPATH_LDAC = 0x03
};

enum aic3x_datapath_ctrl {
	DATAPATH_MUTED = 0x00,			/**< ADC data path is off (muted) */
	DATAPATH_STRAIGHT = 0x01,		/**< LADC plays left channel, RADC plays right channel */
	DATAPATH_SWAPPED = 0x02,		/**< LADC plays right channel, RADC plays left channel */
	DATAPATH_MONO_MIX = 0x03,		/**< ADC plays mono mix of both channels */
};

/* Audio Serial Data Interface Control Register A - Page 0, Reg 8 */
#define ASD_IFA_MODE_SHIFT			(6)
#define ASD_IFA_BCLK_MASTER			(1 << 7)
#define ASD_IFA_WCLK_MASTER			(1 << 6)
#define ASD_IFA_DOUT_HIZ			(1 << 5)

enum aic3x_asd_ifa_ctrl {
	ASD_IFA_SLAVE = 0x00,
	ASD_IFA_MASTER = 0x03
};


/* Audio Serial Data Interface Control Register B - Page 0, Reg 9 */
#define ASD_IFB_MODE_SHIFT			(6)
#define ASD_IFB_MODE_MASK			(0x03U << ASD_IFB_MODE_SHIFT)

enum aic3x_asd_ifb_mode {
	ASD_IFB_MODE_I2S = 0x00,
	ASD_IFB_MODE_DSP = 0x01,
	ASD_IFB_MODE_RJUST = 0x02,
	ASD_IFB_MODE_LJUST = 0x03
};

#define ASD_IFB_LEN_SHIFT			(4)
#define ASD_IFB_LEN_MASK			(0x03U << ASD_IFB_LEN_SHIFT)

enum aic3x_asd_ifb_len {
	ASD_IFB_LEN_16BIT = 0x00,
	ASD_IFB_LEN_20BIT = 0x01,
	ASD_IFB_LEN_24BIT = 0x02,
	ASD_IFB_LEN_32BIT = 0x03
};

struct aic3x_asd_if_cfg {
	enum aic3x_asd_ifa_ctrl		mode;		/**< Master or slave mode */
	enum aic3x_asd_ifb_mode		xfer_mode;	/**< Interface type */
	enum aic3x_asd_ifb_len 		data_len;	/**< Data length */
};

#define AIC3X_MICBIAS_LEVEL_SHIFT		(6)
#define AIC3X_MICBIAS_LEVEL_MASK		(0x03U << AIC3X_MICBIAS_LEVEL_SHIFT)

#define INVERT_VOL(val)   			(0x7FU - val)

/* Default output volume (inverted) */
#define AIC3X_DEFAULT_VOL     			INVERT_VOL(0x50)

/* Default input volume (16dB) */
#define AIC3X_DEFAULT_GAIN    			(0x20U)

/* Route bits */
#define AIC3X_ROUTE_ON				(0x80U)

/* Mute bits */
#define AIC3X_UNMUTE				(0x08U)
#define AIC3X_MUTE_ON				(0x80U)

#define AIC3X_POWER_ON				(0x01U)


/* MIC2/LINE2 to Right ADC Control Register - Page 0, Reg 18 */
#define MIC2LINE2_RADC_CTRL_MASK		(0x0FU)

enum aic3x_mic2line2_radc_ctrl {
	MIC2LINE2_RIGHT_RADC = 0x00,			/**< Offset of MIC2R/LINE2R to RADC bits */
	MIC2LINE2_LEFT_RADC = 0x04			/**< Offset of MIC2L/LINE2L to RADC bits */
};


/* DAC Power and Output Driver Control Register - Page 0, Reg 37 */
enum aic3x_dac_pwr_dac {
	DAC_PWR_RDAC = 0x06,
	DAC_PWR_LDAC = 0x07
};

enum aic3x_input_ctrl_gain {
	INPUT_CTRL_GAIN_0dB = 0x00,
	INPUT_CTRL_GAIN_1_5dB = 0x01,
	INPUT_CTRL_GAIN_3dB = 0x02,
	INPUT_CTRL_GAIN_4_5dB = 0x03,
	INPUT_CTRL_GAIN_6dB = 0x04,
	INPUT_CTRL_GAIN_7_5dB = 0x05,
	INPUT_CTRL_GAIN_9dB = 0x06,
	INPUT_CTRL_GAIN_10_5dB = 0x07,
	INPUT_CTRL_GAIN_12dB = 0x08,
	INPUT_CTRL_ADC_DISCONNECT = 0x0F
};

enum aic3x_rate_div {
	AIC3X_DIV_FS_1 = 0x00,
	AIC3X_DIV_FS_1_5 = 0x01,
	AIC3X_DIV_FS_2 = 0x02,
	AIC3X_DIV_FS_2_5 = 0x03,
	AIC3X_DIV_FS_3 = 0x04,
	AIC3X_DIV_FS_3_5 = 0x05,
	AIC3X_DIV_FS_4 = 0x06,
	AIC3X_DIV_FS_4_5 = 0x07,
	AIC3X_DIV_FS_5 = 0x08,
	AIC3X_DIV_FS_5_5 = 0x09,
	AIC3X_DIV_FS_6 = 0x0A
};

enum aic3x_micbias {
	AIC3X_MICBIAS_OFF = 0x00,
	AIC3X_MICBIAS_2V = 0x01,
	AIC3X_MICBIAS_2V5 = 0x02,
	AIC3X_MICBIAS_AVDD = 0x03,
};

#define CLKGEN_CTRL_SOURCE_MASK			(0x03U)

enum aic3x_clkgen_ctrl {
	CLKGEN_CTRL_PLLCLK_IN = 0x04,
	CLKGEN_CTRL_CLKDIV_IN = 0x06
};

enum aic3x_clkgen_ctrl_source {
	CLKGEN_SOURCE_MCLK = 0x00,
	CLKGEN_SOURCE_GPIO2 = 0x01,
	CLKGEN_SOURCE_BCLK = 0x02
};

enum aic3x_outroute {
	AIC3X_LINE_OUT_STEREO,
	AIC3X_HP_OUT_STEREO,
};

enum aic3x_inroute {
	AIC3X_LINE_IN_STEREO,
	AIC3X_MIC_IN_MONO,
};

struct aic3x_cfg {
	int id;
	uint16_t sl_addr;
	int (*i2c_write)(uint16_t addr, uint8_t reg, uint8_t *data);
	int (*i2c_read)(uint16_t addr, uint8_t reg,  uint8_t *data);
	enum aic3x_outroute out_route;
	enum aic3x_inroute in_route;
};


struct aic3x_dev {
	struct aic3x_cfg *cfg;
	uint8_t cur_page;
};

int aic3x_init(struct aic3x_dev *codec, struct aic3x_cfg *cfg);
int aic3x_reset(struct aic3x_dev *codec);
int aic3x_configure_fref(struct aic3x_dev *codec, enum aic3x_fref fref);
int aic3x_configure_datapath(struct aic3x_dev *codec, enum aic3x_datapath_dac dac, enum aic3x_datapath_ctrl path);
int aic3x_set_sample_rate(struct aic3x_dev *codec, enum aic3x_rate_div adc_div, enum aic3x_rate_div dac_div);
int aic3x_enable_dac_pwr(struct aic3x_dev *codec, enum aic3x_dac_pwr_dac dac, unsigned char en);

#endif /* __TLV320AIC3104_H */
