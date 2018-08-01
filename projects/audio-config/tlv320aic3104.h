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
	enum aic3x_page curr_page;

};

int aic3x_init(struct aic3x_dev *codec, struct aic3x_cfg *cfg);
int aic3x_reset(struct aic3x_dev *codec);
int aic3x_configure_fref(struct aic3x_dev *codec, enum aic3x_fref fref);
int aic3x_configure_datapath(struct aic3x_dev *codec, enum aic3x_datapath_dac dac, enum aic3x_datapath_ctrl path);
int aic3x_set_sample_rate(struct aic3x_dev *codec, enum aic3x_rate_div adc_div, enum aic3x_rate_div dac_div);
int aic3x_enable_dac_pwr(struct aic3x_dev *codec, enum aic3x_dac_pwr_dac dac, unsigned char en);
int aic3x_configure_clock_source(struct aic3x_dev *codec, enum aic3x_clkgen_ctrl clk_in, enum aic3x_clkgen_ctrl_source source);

#endif /* __TLV320AIC3104_H */
