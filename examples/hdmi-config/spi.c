/*
 * spi.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Russell Bush
 */


#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "spi.h"


#ifdef DEBUG
# define SPI_DEBUG_PRINT				DEBUG_PRINT
#else
# ifdef SPI_DEBUG
#  define SPI_DEBUG_PRINT				INFO_PRINT
# else
#  define SPI_DEBUG_PRINT(...)			do {} while (0)
# endif
#endif

#define SPI_NDEVS			(2)
#define SPIDEV_MAX_LEN			(32)

int spidev_fd[SPI_NDEVS];


int spi_init(void)
{
	int i;
	char fname[SPIDEV_MAX_LEN];

	/* Open SPI devices */
	for (i = 0; i < SPI_NDEVS; i++) {

		sprintf(fname, "/dev/spidev2.%d", i);

		spidev_fd[i] = open(fname, O_RDWR);
		if (spidev_fd[i] < 0)
			return spidev_fd[i];
	}

	return (0);
}



int spi_read_reg16(int ss, uint16_t addr, uint16_t *data)
{
	int err;
	struct spi_ioc_transfer xfer;
	unsigned char tx_buf[4];
	unsigned char rx_buf[4];

	tx_buf[0] = (unsigned char)(addr >> 8);
	tx_buf[1] = addr & 0xFFU;

	memset(&xfer, 0, sizeof(struct spi_ioc_transfer));

	xfer.tx_buf = (unsigned long)tx_buf;
	xfer.rx_buf = (unsigned long)rx_buf;
	xfer.speed_hz = 500000;
	xfer.len = 4;

	err = ioctl(spidev_fd[ss], SPI_IOC_MESSAGE(1), &xfer);
	if (err < 0) {
		perror("Reading SPI register");
		return err;
	}

	*data = (((uint16_t)rx_buf[2] << 8) | (rx_buf[3] & 0xFF));

	return (0);
}



int spi_write_reg16(int ss, uint16_t addr, uint16_t data)
{
	int err;
	struct spi_ioc_transfer xfer;
	unsigned char tx_buf[4];
	unsigned char rx_buf[4];

	tx_buf[0] = (unsigned char)(addr >> 8);
	tx_buf[1] = (unsigned char)addr;
	tx_buf[2] = (unsigned char)(data >> 8);
	tx_buf[3] = (unsigned char)data;

	memset(&xfer, 0, sizeof(struct spi_ioc_transfer));

	xfer.tx_buf = (unsigned long)tx_buf;
	xfer.rx_buf = (unsigned long)rx_buf;
	xfer.speed_hz = 500000;
	xfer.len = 4;

	err = ioctl(spidev_fd[ss], SPI_IOC_MESSAGE(1), &xfer);
	if (err < 0) {
		perror("Writing SPI register");
		return err;
	}

	return (0);
}
