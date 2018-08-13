/*
 * hdmi_rx.h
 *
 *  Created on: Nov 8, 2017
 *      Author: engineering
 */

#ifndef __HDMI_RX_H
#define __HDMI_RX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tda1997x.h"

extern struct tda1997x_dev hdmi_rx;

int hdmi_rx_init(void);
int hdmi_rx_dump_timing(void);
int hdmi_rx_set_edid(uint8_t *edid, uint8_t *edid_ext);

#ifdef __cplusplus
}
#endif

#endif /* __HDMI_RX_H_*/
