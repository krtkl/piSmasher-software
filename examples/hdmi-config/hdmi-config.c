/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file	main.c
 * @author	R. Bush
 * @email	bush@krtkl.com
 * @version	v1.0
 * @date	2018 April 10
 * @brief	HDMI Configuration Application
 * @license	FreeBSD
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
 *	  this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "i2c.h"

#include "tmNxTypes.h"
#include "tmNxCompid.h"
#include "tmdlHdmiTx_Types.h"
#include "tmdlHdmiTx_Functions.h"

#include "hdmi_rx.h"
#include "hdmi_tx.h"

//#include "tlv320aic3104.h"
#include "audio.h"

#define ERROR_PRINT(__format__, ...)		printf("[ERROR] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)

#ifdef DEBUG_INFO
# define INFO_PRINT(__format__, ...)		printf("[INFO] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define INFO_PRINT(...)					do {} while (0)
#endif

#ifdef DEBUG
# define DEBUG_PRINT(__format__, ...)		printf("[DEBUG] %s() %s %d: " __format__ "\r\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DEBUG_PRINT(...)			do {} while (0)
#endif

#define UNIT_I2C_ADDRESS_0 0x72
#define CEC_UNIT_I2C_ADDRESS_0 0x36

static tmInstance_t HdmiTxInstance;

static tmdlHdmiTxVideoInConfig_t HdmiTxVideoInputConfig;
static tmdlHdmiTxVideoOutConfig_t HdmiTxVideoOutputConfig;
static tmdlHdmiTxAudioInConfig_t HdmiTxAudioInputConfig;

static int
hdmi_init(void)
{
	int err;

	printf("%-30s", "Initializing TDA1997x HDMI receiver\n");
	err = hdmi_rx_init();
	if (err) {
		ERROR_PRINT("%d = hdmi_rx_init()", err);
		return err;
	}

	printf("done.\n");

	printf("%-30s", "Initializing TD998x HDMI transmitter\n");
#ifndef TMDL_DRIVER
	err = hdmi_tx_init();
	if (err) {
		ERROR_PRINT("%x = hdmi_tx_init()", err);
		return err;
	}

#else
	/* Initialize I2C devices for HDMI and CEC core */
	err = i2c_init(UNIT_I2C_ADDRESS_0);
	if (err < 0)
		return err;

	err = i2c_init(CEC_UNIT_I2C_ADDRESS_0);
	if (err < 0)
		return err;

	INFO_PRINT("tmdlHdmiTxOpen()");
	err = tmdlHdmiTxOpen(&HdmiTxInstance);
	if (err) {
		ERROR_PRINT("%x = tmdlHdmiTxOpen()", err);
		return err;
	}

	INFO_PRINT("tmdlHdmiTxInstanceConfig()");
	err = tmdlHdmiTxInstanceConfig(HdmiTxInstance);
	if (err) {
		ERROR_PRINT("%x = tmdlHdmiTxInstanceConfig()", err);
		return err;
	}

	/* Configure video input */
	HdmiTxVideoInputConfig.format = TMDL_HDMITX_VFMT_04_1280x720p_60Hz;
	HdmiTxVideoInputConfig.mode = TMDL_HDMITX_VINMODE_RGB444;
	HdmiTxVideoInputConfig.syncSource = TMDL_HDMITX_SYNCSRC_EXT_VS;
	HdmiTxVideoInputConfig.pixelRate = TMDL_HDMITX_PIXRATE_SINGLE;
	HdmiTxVideoInputConfig.structure3D = TMDL_HDMITX_3D_NONE;

	/* Configure video output */
	HdmiTxVideoOutputConfig.format = TMDL_HDMITX_VFMT_04_1280x720p_60Hz;
	HdmiTxVideoOutputConfig.mode = TMDL_HDMITX_VOUTMODE_RGB444;
	HdmiTxVideoOutputConfig.colorDepth = TMDL_HDMITX_COLORDEPTH_24;
	HdmiTxVideoOutputConfig.dviVqr = TMDL_HDMITX_RGB_FULL;

	/* Configure audio input */
	HdmiTxAudioInputConfig.channelStatus.PcmIdentification = TMDL_HDMITX_AUDIO_DATA_PCM;
	HdmiTxAudioInputConfig.channelStatus.CopyrightInfo = TMDL_HDMITX_CSCOPYRIGHT_UNPROTECTED;
	HdmiTxAudioInputConfig.channelStatus.FormatInfo = TMDL_HDMITX_CSFI_PCM_2CHAN_NO_PRE;
	HdmiTxAudioInputConfig.channelStatus.categoryCode = 0;
	HdmiTxAudioInputConfig.channelStatus.clockAccuracy = TMDL_HDMITX_CSCLK_LEVEL_II;
	HdmiTxAudioInputConfig.channelStatus.maxWordLength = TMDL_HDMITX_CSMAX_LENGTH_24;
	HdmiTxAudioInputConfig.channelStatus.wordLength = TMDL_HDMITX_CSWORD_DEFAULT;
	HdmiTxAudioInputConfig.channelStatus.origSampleFreq = TMDL_HDMITX_CSOFREQ_48k;

	HdmiTxAudioInputConfig.format = TMDL_HDMITX_AFMT_I2S;
	HdmiTxAudioInputConfig.rate = TMDL_HDMITX_AFS_48K;
	HdmiTxAudioInputConfig.i2sFormat = TMDL_HDMITX_I2SFOR_PHILIPS_L;
	HdmiTxAudioInputConfig.i2sQualifier = TMDL_HDMITX_I2SQ_16BITS;
	HdmiTxAudioInputConfig.dstRate = TMDL_HDMITX_DSTRATE_SINGLE;
	HdmiTxAudioInputConfig.channelAllocation = 1;

	INFO_PRINT("tmdlHdmiTxSetInputOutput()");
	err = tmdlHdmiTxSetInputOutput(HdmiTxInstance, HdmiTxVideoInputConfig, HdmiTxVideoOutputConfig, HdmiTxAudioInputConfig, TMDL_HDMITX_SINK_HDMI);
	if (err) {
		ERROR_PRINT("%d = tmdlHdmiTxSetInputOutput()", err);
		return err;
	}
#endif

	return 0;
}


int
main(int argc, char **argv)
{
	int err;

	printf("--------------------------------------------------------\n");
	printf("piSmasher HDMI Configuration and Test Utility\n");

	printf("Compiled: %s %s\n\n", __DATE__, __TIME__);

	printf("--- Initializing HDMI interfaces\n");
	err = hdmi_init();
	if (err < 0)
		ERROR_PRINT("%x = hdmi_init()", err);

	exit(err);
}
