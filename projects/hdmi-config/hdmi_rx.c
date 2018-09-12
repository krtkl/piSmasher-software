/*
 * hdmi_rx.c
 *
 *  Created on: Nov 8, 2017
 *      Author: Russell Bush
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "i2c.h"

#include "hdmi_rx.h"

#define HDMI_RX_DEVICE_ID			(0)
#define HDMI_RX_NCONFIGS			(1)

#define HDMI_RX_HDMI_I2C_ADDR			(0x48U)
#define HDMI_RX_CEC_I2C_ADDR			(0x34U)


/* Short Audio Descriptors */
//#define SAD_FMT_MASK			(0x78U)

//enum edid_sad_format {
//	SAD_FMT_LPCM = 0x08,		/**< Linear pulse code modulation */
//	SAD_FMT_AC3,			/**< AC-3 */
//	SAD_FMT_MPEG1,			/**< MPEG layers 1 and 2 */
//	SAD_FMT_MP3,			/**< MP3 */
//	SAD_FMT_MPEG2,			/**< MPEG2 */
//	SAD_FMT_AAC,			/**< AAC */
//	SAD_FMT_DTS,			/**< DTS */
//	SAD_FMT_ATRAC,			/**< ATRAC */
//	SAD_FMT_OBA,			/**< One-bit audio */
//	SAD_FMT_DDPLUS,			/**< DD+ */
//	SAD_FMT_DTS_HD,			/**< DTS-HD */
//	SAD_FMT_MLP,			/**< MLP/Dolby TrueHD */
//	SAD_FMT_DST_AUDIO,		/**< DST Audio */
//	SAD_FMT_WMA			/**< Microsoft WMA Pro */
//};

//#define SAD_NCHAN_MASK			(0x07U)

/* SAD Byte 2 */
//#define SAD_FREQ_192kHz			(1 << 6)
//#define SAD_FREQ_176kHz			(1 << 5)
//#define SAD_FREQ_96kHz			(1 << 4)
//#define SAD_FREQ_88kHz			(1 << 3)
//#define SAD_FREQ_48kHz			(1 << 2)
//#define SAD_FREQ_44kHz			(1 << 1)
//#define SAD_FREQ_32kHz			(1 << 0)

/* SAD Byte 3 */

//struct audio_data_block {
//	int nsads;		/**< Number of short audio descriptors (SADs) */
//};


static uint8_t edid_block[127] = {
	0x00,	0xFF,	0xFF,	0xFF,	0xFF,	0xFF,	0xFF,	0x00,	0x3B,	0x10,	0x03,	0x4E,	0x01,	0x00,	0x00,	0x00,
	0x30,	0x15,	0x01,	0x03,	0x80,	0x00,	0x00,	0x00,	0x22,	0x2a,	0xc5,	0xa4,	0x56,	0x4f,	0x9e,	0x28,
	0x0f,	0x50,	0x54,	0x20,	0x00,	0x00,	0x81,	0xc0,	0xd1,	0xc0,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,
	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,

	0x66,	0x21,	0x56,	0xaa,	0x51,	0x00,	0x1e,	0x30,	0x46,	0x8f,	0x33,	0x00,	0xd2,	0x76,	0x00,	0x00,	0x00,	0x1e,
	0x28,	0x3c,	0x80,	0xa0,	0x70,	0xb0,	0x23,	0x40,	0x30,	0x20,	0x36,	0x00,	0xa0,	0x5a,	0x00,	0x00,	0x00,	0x1e,
//	0x01,	0x1d,	0x00,	0x72,	0x51,	0xd0,	0x1e,	0x20,	0x6e,	0x28,	0x55,	0x00,	0xd2,	0x76,	0x00,	0x00,	0x00,	0x1e,

	0x00,    0x00,    0x00,    0xFC,    0x00,    0x70,    0x69,    0x53,    0x6d,    0x61,    0x73,    0x68,    0x65,    0x72,    0x0a,    0x20,    0x20,    0x20,
	0x00,    0x00,    0x00,    0xFD,    0x00,    0x32,    0x3C,    0x0F,    0x44,    0x0F,    0x00,    0x0A,    0x20,    0x20,    0x20,    0x20,    0x20,    0x20,
	0x01
};

static uint8_t edid_ext[127] = {
//    0x02,    0x03,    0x30,    0x41,    0x70,    0x03,    0x0C,    0x00,    0x00,    0x00,    0x00,    0x21,    0x00,    0xA0,    0x06,    0x01,
//    0x40,    0x30,    0x40,    0x60,    0x70,    0x83,    0x01,    0x00,    0x00,    0xE3,    0x05,    0x1F,    0x01,    0x4E,    0x10,    0x1F,
//    0x20,    0x22,    0x21,    0x04,    0x13,    0x03,    0x12,    0x05,    0x14,    0x07,    0x16,    0x01,    0x23,    0x0F,    0x17,    0x07,
//
//
//    0x02,    0x3a,    0x80,    0x18,    0x71,    0x38,    0x2d,    0x40,    0x58,    0x2c,    0x45,    0x00,    0xd2,    0x76,    0x00,    0x00,    0x00,    0x1e,
//    0x01,    0x1d,    0x00,    0x72,    0x51,    0xd0,    0x1e,    0x20,    0x6e,    0x28,    0x55,    0x00,    0xd2,    0x76,    0x00,    0x00,    0x00,    0x1e,
//    0x66,    0x21,    0x56,    0xaa,    0x51,    0x00,    0x1e,    0x30,    0x46,    0x8f,    0x33,    0x00,    0xd2,    0x76,    0x00,    0x00,    0x00,    0x1e,
//    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
//    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00


//	0x02,	0x03,	0x20,	0x41,	0x42,	0x84,   0x10,	0x23,	0x0f,	0x17,	0x07,
//	0x83,	0x69,	0x00,	0x00,	0x70,	0x03,	0x0c,	0x00,	0x00,	0x00,	0x00,	0x21,   0x00,   0xa0,   0x06,   0x01,   0x40,   0x30,    0x40,    0x60,    0x70,
//
//	0x02,	0x3a,	0x80,	0x18,	0x71,	0x38,	0x2d,	0x40,	0x58,	0x2c,	0x45,	0x00,	0xd2,   0x76,   0x00,   0x00,   0x00,   0x1e,
//	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,   0x00,   0x00,
//	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,   0x00,   0x00,
//
//	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
//	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,   0x00,
//	0x00,	0x00,	0x00,	0x00,	0x00,	0x00

	0x02,	0x03,	0x1f,	0x41,	0x47,	0x84,	0x05,	0x03,	0x10,	0x20,	0x22,	0x07,	0x23,	0x0f,	0x07,	0x07,
	0x83,	0x01,	0x00,	0x00,	0xe2,	0x00,	0x0f,	0x67,	0x03,	0x0c,	0x00,	0x20,	0x00,	0x80,	0x21,

	0x01,	0x1d,	0x80,	0x18,	0x71,	0x1c,	0x16,	0x20,	0x58,	0x2c,	0x25,	0x00,	0xa0,	0x5a,	0x00,	0x00,	0x00,	0x9e,
	0x8c,	0x0a,	0xd0,	0x8a,	0x20,	0xe0,	0x2d,	0x10,	0x10,	0x3e,	0x96,	0x00,	0xa0,	0x5a,	0x00,	0x00,	0x00,	0x18,
	0x02,	0x3a,	0x80,	0x18,	0x71,	0x38,	0x2d,	0x40,	0x58,	0x2c,	0x45,	0x00,	0xa0,	0x5a,	0x00,	0x00,	0x00,	0x1e,
	0x28,   0x3c,   0x80,   0xa0,   0x70,   0xb0,   0x23,   0x40,   0x30,   0x20,   0x36,   0x00,   0xa0,   0x5a,   0x00,   0x00,   0x00,   0x1e,

	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
	0x00,	0x00,	0x00,	0x00,	0x00,	0x00
};



struct tda1997x_dev hdmi_rx;

static struct tda1997x_cfg hdmi_rx_cfg_tab[HDMI_RX_NCONFIGS] =
{
	{
		.id = HDMI_RX_DEVICE_ID,
		.i2c_addr = HDMI_RX_HDMI_I2C_ADDR,
		.cec_addr = HDMI_RX_CEC_I2C_ADDR,
		.i2c_write = i2c_write_reg,
		.i2c_read = i2c_read_reg,
		.cur_page = 0xFF
	}
};

/**
 * @brief	HDMI receiver configuration lookup by device identifier
 *
 * @dev_id	Device identifier to use for lookup
 * @return	Pointer to configuration matching device identifier if found or
 * 			NULL if not found in the lookup table
 */
static struct tda1997x_cfg *
hdmi_rx_cfg_lookup(int dev_id)
{
	struct tda1997x_cfg *cfg = NULL;
	int i;

	for (i = 0; i < HDMI_RX_NCONFIGS; i++) {
		if (hdmi_rx_cfg_tab[i].id == dev_id) {
			cfg = &hdmi_rx_cfg_tab[i];
			break;
		}
	}

	return cfg;
}

/**
 * @brief	HDMI Receiver Initialization
 */
int
hdmi_rx_init(void)
{
	int err;
	struct tda1997x_cfg *cfg;

	cfg = hdmi_rx_cfg_lookup(HDMI_RX_DEVICE_ID);
	if (cfg == NULL) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		return (-1);
	}

	err = i2c_init(cfg->i2c_addr);
	if (err < 0) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		return err;
	}

	err = i2c_init(cfg->cec_addr);
	if (err < 0) {
		printf("[ERROR] %s() %s %d\n", __func__, __FILE__, __LINE__);
		return err;
	}

	return tda1997x_init(&hdmi_rx, cfg, edid_block, edid_ext);
}
