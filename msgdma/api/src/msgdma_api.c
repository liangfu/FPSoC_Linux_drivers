/* msgdma_api.c - Modular Scatter-Gather Direct Memory Access driver control API.
 *
 *
 * The MIT License (MIT)
 *
 * COPYRIGHT (C) 2017 Institute of Electronics and Computer Science (EDI), Latvia.
 * AUTHOR: Rihards Novickis (rihards.novickis@edi.lv)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * DESCRIPTION:
 * For API interface documentation refer to "include/msgdma_api.h" header file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


#include "msgdma.h"
#include "msgdma_api.h"


#ifndef	MSGDMA_DEBUG
	#define MSGDMA_DEBUG	0
#endif


#if MSGDMA_DEBUG == 1
	#define __DEBUG(fmt,args...)		printf("MSGDMA_API_DEBUG: " fmt, ##args)
#else
	#define __DEBUG(fmt,args...)
#endif



msgdma_device_t msgdma_init(char* device_name)
{
	__DEBUG("Initializing msgdma device\n");
	return (msgdma_device_t)open(device_name, O_RDWR);
}


int	msgdma_release( msgdma_device_t device )
{
	__DEBUG("Releasing msgdma device\n");
	return close( (int)device );
}


int write_standard_descriptor(msgdma_device_t device, struct msgdma_dscr *dscr)
{
	__DEBUG("write_standard_descriptor()\n");
	return ioctl(device, MSGDMA_WRITE_STD_DSCR, dscr);
}


int write_standard_descriptor_extended(msgdma_device_t device, struct msgdma_dscr_extended *dscr)
{
	__DEBUG("write_standard_descriptor_extended()\n");
	return ioctl(device, MSGDMA_WRITE_EXT_DSCR, dscr);
}


int enable_global_interrupt_mask(msgdma_device_t device)
{
	__DEBUG("enable_global_interrupt_mask()\n");
	return ioctl(device, MSGDMA_ENABLE_IRQ_MASK, NULL);
}


int disable_global_interrupt_mask(msgdma_device_t device)
{
	__DEBUG("disable_global_interrupt_mask()\n");
	return ioctl(device, MSGDMA_DISABLE_IRQ_MASK, NULL);
}


int read_busy(msgdma_device_t device, int*busy)
{
	__DEBUG("read_busy()\n");
	return ioctl(device, MSGDMA_IS_BUSY_MASK, busy);
}

int reset_dispatcher(msgdma_device_t device)
{
	__DEBUG("reset_dispatcher()\n");
	return ioctl(device, MSGDMA_RESET_MASK, NULL);	
}