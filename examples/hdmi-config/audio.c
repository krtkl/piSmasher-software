/*
 * audio.c
 *
 *  Created on: Sep 30, 2017
 *      Author: Russell Bush
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "i2c.h"

#include "tlv320aic3104.h"

#include "audio.h"

#define AUDIO_DEVICE_ID				(0)
#define AUDIO_NCONFIGS				(1)

static struct aic3x_dev codec;

static struct aic3x_cfg aud_cfg_tab[AUDIO_NCONFIGS] =
{
	{
		.id = AUDIO_DEVICE_ID,
		.sl_addr = AIC3X_ADDR,
		.i2c_write = i2c_write_reg,
		.i2c_read = i2c_read_reg
	}
};


/**
 * @brief	Audio configuration lookup by device identifier
 *
 * @dev_id	Device identifier to use for lookup
 * @return	Pointer to configuration matching device identifier if found or
 * 			NULL if not found in the lookup table
 */
static struct aic3x_cfg *aud_cfg_lookup(int dev_id)
{
	struct aic3x_cfg *pcfg = NULL;
	int i;

	for (i = 0; i < AUDIO_NCONFIGS; i++) {
		if (aud_cfg_tab[i].id == dev_id) {
			pcfg = &aud_cfg_tab[i];
			break;
		}
	}

	return pcfg;
}


/**
 * @brief	Audio Initialization
 */
int aud_init(void)
{
	int err;
	struct aic3x_cfg *cfg;

	cfg = aud_cfg_lookup(AUDIO_DEVICE_ID);
	if (cfg == NULL)
		return (-1);

	err = i2c_init(cfg->sl_addr);
	if (err < 0)
		return err;

	return aic3x_init(&codec, cfg);
}
