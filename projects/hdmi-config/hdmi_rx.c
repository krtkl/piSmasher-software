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

struct tda1997x_dev hdmi_rx;

static struct tda1997x_cfg hdmi_rx_cfg_tab[HDMI_RX_NCONFIGS] =
{
	{
		.id = HDMI_RX_DEVICE_ID,
		.i2c_addr = HDMI_RX_HDMI_I2C_ADDR,
		.cec_addr = HDMI_RX_CEC_I2C_ADDR,
		.i2c_write = i2c_write_reg,
		.i2c_read = i2c_read_reg
	}
};

/**
 * @brief	HDMI receiver configuration lookup by device identifier
 *
 * @dev_id	Device identifier to use for lookup
 * @return	Pointer to configuration matching device identifier if found or
 * 			NULL if not found in the lookup table
 */
static struct tda1997x_cfg *hdmi_rx_cfg_lookup(int dev_id)
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
int hdmi_rx_init(void)
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

	return tda1997x_init(&hdmi_rx, cfg);
}
