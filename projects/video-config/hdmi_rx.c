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


/**
 * @brief	EDID block
 */
static uint8_t edid_block[127] = {
	0x00,	0xFF,	0xFF,	0xFF,	0xFF,	0xFF,	0xFF,	0x00,			/* Fixed header */
	0x3B,	0x10,									/* Manufacturer ID */
	0x03,	0x4E,									/* Manufacturer product code */
	0x01,	0x00,	0x00,	0x00,							/* Serial number */
	0x30,	0x15,									/* Manufacture week/year */
	0x01,	0x03,									/* EDID version */
	0x80,										/* Video input parameters */
	0x00,										/* Maximum horizontal size */
	0x00,										/* Maximum vertical size */
	0x00,										/* Display gamma */
	0x1A,										/* Supported features */
	0xEE,	0x95,	0xA3,	0x54,	0x4C,	0x99,	0x26,	0x0F,	0x50,	0x54,	/* Chromaticity coordinates */
	0x20,	0x00,	0x00,								/* Established timing */
	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,			/* Standard timing information */
	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,			/* Standard timing information (cont.) */
	0x02,	0x3A,	0x80,	0x18,	0x71,	0x38,	0x2D,	0x40,	0x58,		/* Timing descriptor */
	0x2C,	0x45,	0x00,	0xC4,	0x8E,	0x21,	0x00,	0x00,	0x1E,		/* Timing descriptor (cont.) */
	0x00,	0x00,	0x00,	0xFC,	0x00,	0x70,	0x69,	0x53,	0x6d,		/* Monitor name descriptor */
	0x61,	0x73,	0x68,	0x65,	0x72,	0x0a,	0x20,	0x20,	0x20,		/* Monitor name descriptor (cont.) */
	0x00,	0x00,	0x00,	0xFD,	0x00,	0x32,	0x3C,	0x0F,	0x44,		/* Range limits descriptor */
	0x0F,	0x00,	0x0A,	0x20,	0x20,	0x20,	0x20,	0x20,	0x20,		/* Range limits descriptor (cont.) */
	0x00,	0x00,	0x00,	0xFE,	0x00,	0x31,	0x30,	0x38,	0x30,		/* Unspecified text descriptor */
	0x70,	0x44,	0x43,	0x78,	0x76,	0x43,	0x33,	0x44,	0x0A,		/* Unspecified text descriptor (cont.) */
	0x01,										/* Extensions to follow */
};


/**
 * @brief	EDID extension block
 */
static uint8_t edid_ext[127] = {
	0x02,	0x03,								/* Extension tag and version */
	0x30,									/* DTD offset */
	0xF0,									/* Global declarations */
	0x70,									/* Vendor specific data */
	0x03,	0x0C,	0x00,							/* IEEE registration identifier */
	0x00,	0x00,								/* SPA AB CD */
	0xB8,
	0x21,									/* Max TMDS frequency (165 MHz) */
	0x00,									/* Latency field present */
	0xA0,	0x06,	0x01,	0x40,						/* Latency values */
	0x30,	0x40,	0x60,	0x70,
	0x83,	0x01,	0x00,	0x00,						/* Speaker allocation */
	0xE3,	0x05,	0x1F,	0x01,						/* Colorimetry data block */
	0x4E,									/* Video data block */
	0x10,									/* 1920x1080p @ 60 Hz */
	0x1F,									/* 1920x1080p @ 50 Hz */
	0x20,									/* 1920x1080p @ 24 Hz */
	0x22,									/* 1920x1080p @ 30 Hz */
	0x21,									/* 1920x1080p @ 25 Hz */
	0x04,									/* 1280x720p @ 60 Hz */
	0x13,									/* 1280x720p @ 50 Hz */
	0x03,									/* 720x480p @ 60 Hz */
	0x12,									/* 720x576p @ 50 Hz */
	0x05,									/* 1920x1080i @ 60 Hz */
	0x14,									/* 1920x1080i @ 50 Hz */
	0x07,									/* 720x480i @ 60 Hz */
	0x16,									/* 720x576i @ 50 Hz */
	0x01,									/* 640x480p @ 60 Hz */
	0x23,	0x0F,	0x17,	0x07,						/* Audio data block */
	0x02,	0x3A,	0x80,	0xD0,	0x72,	0x38,	0x2D,	0x40,	0x10,	/* DTD (148.5 MHz) */
	0x2C,	0x45,	0x80,	0xC4,	0x8E,	0x21,	0x00,	0x00,	0x1E,
	0x01,	0x1D,	0x80,	0x3E,	0x73,	0x38,	0x2D,	0x40,	0x7E,	/* DTD (74.25 MHz) */
	0x2C,	0x45,	0x80,	0xC4,	0x8E,	0x21,	0x00,	0x00,	0x1E,
	0x01,	0x1D,	0x80,	0x18,	0x71,	0x38,	0x2D,	0x40,	0x58,	/* DTD (74.25 MHz) */
	0x2C,	0x45,	0x00,	0xC4,	0x8E,	0x21,	0x00,	0x00,	0x1E,
	0x01,	0x1D,	0x80,	0xD0,	0x72,	0x38,	0x2D,	0x40,	0x10,	/* DTD (74.25 MHz) */
	0x2C,	0x45,	0x80,	0xC4,	0x8E,	0x21,	0x00,	0x00,	0x1E,
	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,			/* Padding */
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
