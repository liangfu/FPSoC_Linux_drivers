/* pll_control.h
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

#ifndef PLL_CTL_CMD_H__
#define PLL_CTL_CMD_H__


/* Command structure */
typedef struct cmd_t{
	unsigned action;
	unsigned value;
} wr_cmd_t; 


/* Should be defined in Settings.mak file */
#ifndef PLL_IOC_MAGIC
#define PLL_IOC_MAGIC	0xf0
#endif


/* IOCTL commands interfacing configuration registers
   Although register width differs, we allways use 4 byte values */
#define PLL_CTL_MODE_WRITE 					_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,1,  4)
#define PLL_CTL_MODE_READ 					_IOC(_IOC_READ,		PLL_IOC_MAGIC,2,  4)
#define PLL_CTL_STATUS_READ	 				_IOC(_IOC_READ,		PLL_IOC_MAGIC,3,  4)
#define PLL_CTL_START_WRITE	 				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,4,  4)
#define PLL_CTL_N_COUNTER_WRITE				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,5,  4)
#define PLL_CTL_N_COUNTER_READ				_IOC(_IOC_READ,		PLL_IOC_MAGIC,6,  4)
#define PLL_CTL_M_COUNTER_WRITE				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,7,  4)
#define PLL_CTL_M_COUNTER_READ				_IOC(_IOC_READ,		PLL_IOC_MAGIC,8,  4)
#define PLL_CTL_C_COUNTER_WRITE				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,9,  4)
#define PLL_CTL_C_COUNTER_READ				_IOC(_IOC_READ,		PLL_IOC_MAGIC,10,  4)
#define PLL_CTL_DYNAMIC_SHIFT_MODE_WRITE 	_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,11,  4)
#define PLL_CTL_M_COUNTER_FRACT_WRITE	 	_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,12,  4)
#define PLL_CTL_BANDWIDTH_READ 				_IOC(_IOC_READ,		PLL_IOC_MAGIC,13,  4)
#define PLL_CTL_BANDWIDTH_WRITE				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,14,  4)
#define PLL_CTL_CHARGE_PUMP_READ 			_IOC(_IOC_READ,		PLL_IOC_MAGIC,15,  4)
#define PLL_CTL_CHARGE_PUMP_WRITE 			_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,16,  4)
#define PLL_CTL_VCO_DIV_READ 				_IOC(_IOC_READ,		PLL_IOC_MAGIC,17,  4)
#define PLL_CTL_VCO_DIV_WRITE 				_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,18,  4)
#define PLL_CTL_MIF_BASE_WRITE	 			_IOC(_IOC_WRITE,	PLL_IOC_MAGIC,19,  4)

#define PLL_CTL_MAXNR						19

/* Supported command list */
#define RCFIG_PLL_MODE_WREQ 		0	/* Mode register - write request mode */
#define RCFIG_PLL_MODE_POLL 		1	/* Mode register - polling mode */



/* Reading device file results into recieving reconfiguartion status */



#endif
