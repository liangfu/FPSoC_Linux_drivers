/* msgdma_api.h - Modular Scatter-Gather Direct Memory Access control API header file.
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
 * Refer to this file for interface documentation.
 */

#ifndef MSGDMA_API_H_
#define MSGDMA_API_H_

#ifndef __KERNEL__
	#include <stdint.h>
#endif

/** Descriptor Control bitfield */
#define MSGDMA_DSCR_TRANSMIT_CHANNEL_MASK		(0xff)
#define MSGDMA_DSCR_GENERATE_SOP				(1<<8)
#define MSGDMA_DSCR_GENERATE_EOP				(1<<9)
#define MSGDMA_DSCR_PARK_READS					(1<<10)
#define MSGDMA_DSCR_PARK_WRITES					(1<<11)
#define MSGDMA_DSCR_END_ON_EOP					(1<<12)
#define MSGDMA_DSCR_TRANSFER_COMPLETE_IRQ_MASK 	(1<<14)
#define MSGDMA_DSCR_EARLY_TERMINATION_IRQ_MASK	(1<<15)
#define MSGDMA_DSCR_TRANSMIT_ERROR_IRQ_MASK		(0xff<<16)
#define MSGDMA_DSCR_EARLY_DONE_ENABLE			(1<<24)
#define MSGDMA_DSCR_GO 							(1<<31)

#define TRANSMIT_CHANNEL_MASK


/**
 * @brief Type for msgdma devices (there can be multiple).
 */
typedef int msgdma_device_t;


/**
 * @brief Structure describing minimal transfer desciptor.
 */
struct msgdma_dscr{
	uint32_t	read_addr;
	uint32_t	write_addr;
	uint32_t	length;
	uint32_t	control;
};


/**
 * @brief Structure describing extended transfer desciptor. Remeber to initialize to 0.
 */
struct msgdma_dscr_extended{
	uint32_t	read_addr;
	uint32_t	write_addr;
	uint32_t	length;
	uint8_t 	read_burst_count;
	uint8_t 	write_burst_count;
	uint16_t 	seq_number;
	uint16_t	read_stride;
	uint16_t	write_stride;
	uint32_t	read_addr_high;
	uint32_t	write_addr_high;
	uint32_t	control;
};


	#ifndef __KERNEL__
/**
 * @brief Initialize msgdma device (basically perform open() syscall).
 *
 * @param device_name Device name to initialize ("/dev/msgdmaX").
 * 
 * @return returns device descriptor or negative number on error.
 */
msgdma_device_t msgdma_init(char *device_name);

/**
 * @brief Release msgdma device (basically perform close() syscall).
 *
 * @param device Device desriptor.
 * 
 * @return returns Returns 0 on succsess.
 */
int	msgdma_release(msgdma_device_t device);

/**
 * @brief Sends a fully formed standard descriptor to the dispatcher module. 
 * If IRQ flag is set, then process is suspended until driver IRQ is recieved.
 *
 * @param device Devie descriptor.
 * @param dscr 	 Pointer to desriptor structure.
 *
 * @return Returns 0 on succsess.
 */
int write_standard_descriptor(msgdma_device_t device, struct msgdma_dscr *dscr);

/**
 * @brief Sends a fully formed extended descriptor to the dispatcher module. 
 * If IRQ flag is set, then process is suspended until driver IRQ is recieved.
 *
 * @param device Devie descriptor.
 * @param dscr 	 Pointer to extended desriptor structure.
 *
 * @return Returns 0 on succsess.
 */
int write_standard_descriptor_extended(msgdma_device_t device, struct msgdma_dscr_extended *dscr);

/**
 * @brief Enable interrupts
 * 
 * @param device Devie descriptor.
 * 
 * @return Returns 0 on succsess.
 */
int enable_global_interrupt_mask(msgdma_device_t device);

/**
 * @brief Enable interrupts
 * 
 * @param device Devie descriptor.
 * 
 * @return Returns 0 on succsess.
 */
int disable_global_interrupt_mask(msgdma_device_t device);


/**
 * @brief Check if MSGDMA is in busy state
 *
 * @param device Devie descriptor.
 * @param busy   Destination to save busy state
 *
 * @return Returns 0 on succsess.
 */
int read_busy(msgdma_device_t device, int *busy);


/**
 * @brief Reset dispatcher
 *
 * @param device Devie descriptor.
 *
 * @return Returns 0 on succsess.
 */
int reset_dispatcher(msgdma_device_t device);

/*
TODO:
read_mm_response();
read_csr_status();
read_csr_control();
read_csr_read_descriptor_buffer_fill_level();
read_csr_write_descriptor_buffer_fill_level();
read_csr_response_buffer_fill_level();
read_csr_read_sequence_number();
read_csr_write_sequence_number();
read_descriptor_buffer_empty();
read_descriptor_buffer_full();
read_response_buffer_empty();
read_response_buffer_full();
read_stopped();
read_resetting();
read_stopped_on_error();
read_stopped_on_early_termination();
read_irq();
stop_dispatcher();
start_dispatcher();
enable_stop_on_error();
disable_stop_on_error();
enable_stop_on_early_termination();
disable_stop_on_early_termination();
stop_descriptors();
start_descriptors();
*/
	#endif
#endif
