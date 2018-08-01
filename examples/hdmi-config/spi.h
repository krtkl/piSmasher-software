/*
 * spi.h
 *
 *  Created on: Aug 14, 2017
 *      Author: Russell Bush
 */

#ifndef __SPI_H
#define __SPI_H


int spi_init(void);
int spi_read_reg16(int ss, uint16_t addr, uint16_t *data);
int spi_write_reg16(int ss, uint16_t addr, uint16_t data);

#endif /* __SPI_H */
