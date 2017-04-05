/* pll_api.h - api library header file.
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
 */

#ifndef PLL_API_H_
#define PLL_API_H_


/**
 * @brief Type for pll devices (there can be multiple).
 */
typedef int pll_dev_t;

/**
 * @brief Initialize pll device (basically perform open() syscall).
 *
 * @param device_name Device name to initialize ("/dev/pllX").
 * 
 * @return returns device descriptor or negative number on error.
 */
pll_dev_t 	pll_dev_init	( const char *device_name);


/**
 * @brief Release pll device (basically perform close() syscall).
 *
 * @param device Device desriptor.
 * 
 * @return returns Returns 0 on succsess.
 */
int 	pll_dev_release	( pll_dev_t device );


/**
 * @brief Calculate counter values for desired frequency by using brute force approach.
 *		  normally counter values should be passed to pll_reconfigure_basic()
 *
 * @param m,n,c Pointers to the counters.
 * @param reference_freq Reference frequency of the pll in MHz.
 * @param desired_freq Desired frequency in MHz.
 *
 * @return Frequency to which counters are configured (best match).
 */
float pll_calculate_counters_brute_force(unsigned *m, unsigned *n, unsigned *c, float reference_freq, float desired_freq);


/**
 * @brief Configure PLL with counters.
 *
 * @param device Devie descriptor. 
 * @param m,n,c Counter values.
 *
 * @return Returns 0 on succsessful configuration.
 */
int   pll_reconfigure_basic(pll_dev_t device, unsigned m, unsigned n, unsigned c);


/** @name register_write
 *  Low level register write functions.
 *
 * @param device Devie descriptor.
 * @param value Write value to register.
 * 
 * @return Returns 0 on succsessful configuration.
 */
///@{
int pll_mode_write 			(pll_dev_t device, unsigned value);
int pll_start_write 		(pll_dev_t device, unsigned value);
int pll_n_counter_write 	(pll_dev_t device, unsigned value);
int pll_m_counter_write 	(pll_dev_t device, unsigned value);
int pll_c_counter_write 	(pll_dev_t device, unsigned value);
int pll_dynamic_shift_write (pll_dev_t device, unsigned value);
int pll_m_fract_write 		(pll_dev_t device, unsigned value);
int pll_bandwidth_write		(pll_dev_t device, unsigned value);
int pll_charge_pump_write	(pll_dev_t device, unsigned value);
int pll_vco_div_write		(pll_dev_t device, unsigned value);
int pll_mif_base_write		(pll_dev_t device, unsigned value);
///@}

/** @name register_read
 *  Low level register read functions.
 *
 * @param device Devie descriptor.
 * @param value Pointer to save register value.
 * 
 * @return Returns 0 on succsessful read.
 */
///@{
int pll_mode_read 			(pll_dev_t device, unsigned *value);
int pll_status_read  		(pll_dev_t device, unsigned *value);
int pll_n_counter_read  	(pll_dev_t device, unsigned *value);
int pll_m_counter_read  	(pll_dev_t device, unsigned *value);
int pll_c_counter_read  	(pll_dev_t device, unsigned *value);
int pll_bandwidth_read 		(pll_dev_t device, unsigned *value);
int pll_charge_pump_read 	(pll_dev_t device, unsigned *value);
int pll_vco_div_read 		(pll_dev_t device, unsigned *value);
///@}

#endif