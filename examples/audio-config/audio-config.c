/*
 * main.c
 *
 *  Created on: May 8, 2018
 *      Author: bush
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>


#include "i2c-dev.h"
#include "i2c.h"

#include "tlv320aic3104.h"


static struct aic3x_dev codec;
static struct aic3x_cfg cfg = {
	.id = 0,
	.sl_addr = AIC3X_ADDR,
	.i2c_write = i2c_write_reg,
	.i2c_read = i2c_read_reg,
};

static void usage(void)
{
	printf("Usage:\n"
		"  audio-config [-i inpath] [-o outpath]\n"
		"\n"
		"    outpath -- Must be either \"lineout\" or \"headphones\"\n"
		"    inpath -- Must be either \"linein\" or \"mic\"\n"
	);
}


int main(int argc, char *argv[])
{
	int fd, error;
	int opt;
	char buf[2];
	char *inpath, *outpath;

	while ((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch (opt) {
		case 'i':
			/* Set the ADC datapath */
			inpath = optarg;
			break;
		case 'o':
			/* Set the DAC datapath */
			outpath = optarg;
			break;
		case '?':
		default:
			printf("[ERROR]: Unknown character `-%c'\n", optopt);
			usage();
			exit(EXIT_FAILURE);
			break;

		}
	}

	printf("test\n");

	if (outpath == NULL) {
		cfg.out_route = AIC3X_HP_OUT_STEREO;
	} else if (strcmp(outpath, "lineout") == 0) {
		cfg.out_route = AIC3X_LINE_OUT_STEREO;
	} else if (strcmp(outpath, "headphones") == 0) {
		cfg.out_route = AIC3X_HP_OUT_STEREO;
	} else {
		printf("[ERROR]: Invalid output path\n");
		usage();
		exit(EXIT_FAILURE);
	}

	if (inpath == NULL) {
		cfg.in_route = AIC3X_LINE_IN_STEREO;
	} else if (strcmp(inpath, "linein") == 0) {
		cfg.in_route = AIC3X_LINE_IN_STEREO;
	} else if (strcmp(inpath, "mic") == 0) {
		cfg.in_route = AIC3X_MIC_IN_MONO;
	} else {
		printf("[ERROR]: Invalid input path\n");
		usage();
		exit(EXIT_FAILURE);
	}

	i2c_init(cfg.sl_addr);

	aic3x_init(&codec, &cfg);

	exit(EXIT_SUCCESS);

	fd = open("/dev/i2c-0", O_RDWR);
	if (fd < 0)
		err(fd, "opening I2C device");

	error = ioctl(fd, I2C_SLAVE, 0x18U);
	if (error < 0)
		err(error, "setting I2C slave address");

	/* Reset */
	/* Page 0/Register 1 */
	buf[0] = 0x00;
	buf[1] = 0x00;
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	if ((error = read(fd, buf, 2)) < 0)
		err(error, "reading I2C device");

	buf[0] = 0x01;
	buf[1] = (1 << 7);		/* Self-clearing software reset */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	usleep(1000 * 5);

	/* Set the audio serial data interface configuration */

	/* Page 0/Register 8 */
	buf[0] = 0x08;
	buf[1] = (1 << 7) |		/* BCLK output (master mode) */
		 (1 << 6);		/* WCLK output (master mode) */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 0x03;
	buf[1] = (1 << 7);
	if ((error = write(fd, buf, 2)) < 0)
			err(error, "writing I2C device");

	/* Page 0/Register 9 */
	/* Default of 0x00 is sufficient here (I2S, 16-bit) */

	/* Configure the ADCs */
	buf[0] = 0x13;
	buf[1] = (1 << 2);		/* Power up left ADC */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 0x16;
	buf[1] = (1 << 2);		/* Power up right ADC */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	/* Left and right ADC PGA mute and control registers */
	buf[0] = 0x0f;
	buf[1] = 0x00;			/* Unmute left ADC and set 0dB gain */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 0x10;
	buf[1] = 0x00;			/* Unmute right ADC and set 0dB gain */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	/* Data-path register setup */
	buf[0] = 0x07;
	buf[1] = (1 << 7) |		/* fsref = 44.1kHz */
		 (1 << 3) |		/* left DAC plays left channel */
		 (1 << 1);		/* right DAC plays right channel */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 0x25;
	buf[1] = (1 << 7) |		/* Left DAC is powered up */
		 (1 << 6);		/* Right DAC is powered up */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	/* DAC default volume and unmute */
	buf[0] = 0x2b;
	buf[1] = 0x00;
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 0x2c;
	buf[1] = 0x00;
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

//	buf[0] = 0x52;
//	buf[1] = 0x80;
//	if ((error = write(fd, buf, 2)) < 0)
//			err(error, "writing I2C device");
//
//	buf[0] = 0x5c;
//	buf[1] = 0x80;
//	if ((error = write(fd, buf, 2)) < 0)
//			err(error, "writing I2C device");
//
//	buf[0] = 0x56;
//	buf[1] = 0x09;
//	if ((error = write(fd, buf, 2)) < 0)
//			err(error, "writing I2C device");
//
//	buf[0] = 0x5d;
//	buf[1] = 0x09;
//	if ((error = write(fd, buf, 2)) < 0)
//			err(error, "writing I2C device");

//	/* DAC routing enablement */
//	buf[0] = 0x2f;
//	buf[1] = (1 << 7);		/* DAC L1 is routed to HPLOUT */
//	if ((error = write(fd, buf, 2)) < 0)]
//		err(error, "writing I2C device");
//
//	buf[0] = 0x40;
//	buf[1] = (1 << 7);		/* DAC R1 is routed to HPROUT */
//	if ((error = write(fd, buf, 2)) < 0)
//		err(error, "writing I2C device");
//
	/* DAC_L1 to HPLOUT */
	buf[0] = 47;
	buf[1] = (1 << 7);
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	/* DAC_R1 to HPROUT */
	buf[0] = 64;
	buf[1] = (1 << 7);
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	/* HPLOUT output level control register */
	buf[0] = 51;
	buf[1] = (1 << 3) |		/* Unmute HPLOUT output */
		 (1 << 0);		/* HPLOUT is fully powered up */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	buf[0] = 65;
	buf[1] = (1 << 3) |		/* Unmute HPROUT output */
		 (1 << 0);		/* HPROUT is fully powered up */
	if ((error = write(fd, buf, 2)) < 0)
		err(error, "writing I2C device");

	exit(EXIT_SUCCESS);
}


