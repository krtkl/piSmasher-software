/**
 *******************************************************************************
 *******************************************************************************
 *
 * @file    main.c
 * @author  R. Bush
 * @email   bush@krtkl.com
 * @version 0.1
 * @date    July 10, 2018
 * @brief   Video Test Pattern Generator Userspace Control
 * @license FreeBSD
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
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "vdma.h"

#define VIDEO_HSIZE		(3840)		/**< Number of bytes per line */
#define VIDEO_VSIZE		(720)		/**< Number of vertical rows */
#define BUF_SIZE		(VIDEO_HSIZE * VIDEO_VSIZE)

static int
vdma_init_bufs(void)
{
	int fd;
	void *mem;

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0)
		return fd;

	mem = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x30000000);
	if (mem == MAP_FAILED) {
		close(fd);
		return -2;
	}

	memset(mem, 0, BUF_SIZE);
	munmap(mem, BUF_SIZE);
	close(fd);
	return 0;
}

int
main(int argc, char *argv[])
{
	int c;
	bool stop = false;
	int ret;
	const char *dev_name = NULL;
	struct pollfd pollfd;
	struct timeval tv;
	AxiVdma *vdma;

	while ((c = getopt(argc, argv, "pd:")) != -1) {
		switch (c) {
		case 'p': stop = true; break;
		case 'd': dev_name = optarg; break;
		case '?':
		default:
			printf("Unknown option character `\\x%x'.\n", optopt);
			return -1;
		}
	}

	if (dev_name == NULL) {
		printf("Select a device path\n");
		return -1;
	}

	vdma = malloc(sizeof(AxiVdma));
	if (!vdma)
		return -9;

	ret = vdma_init(vdma, dev_name);
	if (ret < 0)
		goto out;

//	vdma_init_bufs();
	ret = vdma_config(vdma);
	if (ret < 0)
		goto out;

	usleep(1000 * 2000);

	ret = vdma_get_status(vdma);
	if (ret < 0)
		goto out;

//	pollfd.fd = vdma->fd;
//	pollfd.events = POLLRDNORM;
//
//	tv.tv_sec = 5;
//	tv.tv_usec = 0;
//
//	ret = poll(&pollfd, 1, 5000);
//	if (ret != -1) {
//		vdma_memdump();
//	}

out:
	munmap(vdma->base, 0x10000);
	close(vdma->fd);
	free(vdma);
	return ret;
}
