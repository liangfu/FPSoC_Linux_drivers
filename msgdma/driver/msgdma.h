/* msgdma.h
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
 */

#ifndef MSGDMA_H_
#define MSGDMA_H_


/* Import data types */
#include "msgdma_api.h"


/* Should be defined in Settings.mak file */
#ifndef MSGDMA_IOCTL_MAGIC
#define MSGDMA_IOCTL_MAGIC 	0xf1
#endif


/* Basic commands required for configuration */
#define MSGDMA_WRITE_STD_DSCR			_IOC(_IOC_WRITE,	MSGDMA_IOCTL_MAGIC,1,  sizeof(struct msgdma_dscr))
#define MSGDMA_WRITE_EXT_DSCR			_IOC(_IOC_WRITE,	MSGDMA_IOCTL_MAGIC,2,  sizeof(struct msgdma_dscr_extended))
#define MSGDMA_ENABLE_IRQ_MASK			_IOC(_IOC_WRITE,	MSGDMA_IOCTL_MAGIC,3,  0)
#define MSGDMA_DISABLE_IRQ_MASK			_IOC(_IOC_WRITE,	MSGDMA_IOCTL_MAGIC,4,  0)
#define MSGDMA_IOCTL_MAXNR 				4


#endif
