/*
 * i2c.c
 *
 *  Created on: Jul 11, 2017
 *      Author: Russell Bush
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include "i2c-dev.h"
#include "i2c.h"

#define I2C_NDEVS		(16)

struct i2c_addr_fd {
	uint8_t	addr;
	int	fd;
};

static struct i2c_addr_fd i2c_fd[I2C_NDEVS];
static int i2c_ndevs = 0;


static int i2c_addr_fd_lookup(struct i2c_addr_fd *tab, int len, uint8_t addr)
{
	int i;
	int fd = -1;

	for (i = 0; i < len; i++, tab++)
		if (tab->addr == addr)
			fd = tab->fd;

	return fd;
}




/**
 * @brief	Initialize I2C device for communication
 */
int i2c_init(uint8_t addr)
{
	int err;
	int fd;

	/* Perform filename parsing here */
	fd = open("/dev/i2c-0", O_RDWR);
	if (fd < 0)
		return fd;

	/* Add the file descriptor to the list */
	i2c_fd[i2c_ndevs].fd = fd;
	i2c_fd[i2c_ndevs].addr = addr;

	err = ioctl(i2c_fd[i2c_ndevs].fd, I2C_SLAVE, addr);
	if (err < 0)
		return err;

	i2c_ndevs++;

	printf("[I2C]: New I2C device registered %d - 0x%02x\n", fd, addr);

	return fd;
}


/**
 * @brief	Write a byte value to a register on an I2C slave device
 *
 * @sl_addr
 * @reg_addr
 * @data
 * @return
 */
int i2c_write_reg(uint16_t sl_addr, uint8_t reg_addr, uint8_t *data)
{
	int fd, error;
//	unsigned char buf[2];

	fd = i2c_addr_fd_lookup(i2c_fd, i2c_ndevs, sl_addr);
	if (fd < 0) {
		perror("Failed to acquire bus access and/or talk to slave");
		return fd;
	}

//	buf[0] = reg_addr;
//	buf[1] = *data;

	error = i2c_smbus_write_byte_data(fd, reg_addr, *data);
//	error = write(fd, buf, 2);
	if (error < 0) {
		warn("writing I2C bus %d", error);
		return error;
	}

	return 0;
}


/**
 * @brief	Read a byte value from a register on an I2C slave device
 *
 * @sl_addr
 * @reg_addr
 * @data
 * @return
 */
int i2c_read_reg(uint16_t sl_addr, uint8_t reg_addr, uint8_t *data)
{
	int tmp, fd;
//	unsigned char buf[2];

	fd = i2c_addr_fd_lookup(i2c_fd, i2c_ndevs, sl_addr);
	if (fd < 0) {
		perror("Failed to acquire bus access and/or talk to slave");
		return fd;
	}

	tmp = i2c_smbus_read_byte_data(fd, reg_addr);
	if (tmp < 0)
		return tmp;

	*data = tmp;
	return (0);
}


int i2c_write_block(uint16_t sl_addr, uint8_t reg_addr, uint8_t len, uint8_t *data)
{
	int tmp, fd;

	fd = i2c_addr_fd_lookup(i2c_fd, i2c_ndevs, sl_addr);
	if (fd < 0) {
		perror("Failed to acquire bus access and/or talk to slave");
		return fd;
	}

	tmp = i2c_smbus_write_block_data(fd, reg_addr, len, data);
	if (tmp < 0)
		return tmp;

	return (0);
}


int i2c_read_block(uint16_t sl_addr, uint8_t reg_addr, uint8_t len, uint8_t *data)
{
	int i, fd, err;
	union i2c_smbus_data bus_data;

	fd = i2c_addr_fd_lookup(i2c_fd, i2c_ndevs, sl_addr);
	if (fd < 0) {
		perror("Failed to acquire bus access and/or talk to slave");
		return fd;
	}

	err = i2c_smbus_read_i2c_block_data(fd, reg_addr, len, data);
	if (err < 0)
		return err;

//	//read bytes
//	err = i2c_smbus_access(fd, I2C_SMBUS_READ, reg_addr, I2C_SMBUS_BLOCK_DATA, &bus_data);
//	if (err < 0)
//		return err;
//
//	//copy data
//	for (i = 0; i < bus_data.block[0]; i++, data++) {
//		*data = bus_data.block[i];
//	}

	return (0);
}



