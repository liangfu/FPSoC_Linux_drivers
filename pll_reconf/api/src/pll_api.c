/* pll_api.c - simple API for "Altera PLL Reconfig" IP core driver.
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
 * This API translates register I/O methods. The only more involved functions of
 * this API are pll_calculate_counters_brute_force() and pll_reconfigure_basic()
 * which can be used to calculate and reprogram counters for specific frequency
 * when timing is not an issue.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <float.h> 		/*FLT_MAX*/

#include "pll_control.h"
#include "pll_api.h"


#ifndef DEBUG_PLL_CTL
	#define DEBUG_PLL_CTL 0
#endif

#if DEBUG_PLL_CTL == 1
	#define __DEBUG(fmt, args...)	printf("PLL_API_DEBUG: " fmt, ##args)
#else
	#define __DEBUG(fmt,args...)
#endif 



/******************** Initialize/Release functions ********************/
pll_dev_t pll_dev_init( const char *device_name)
{
	__DEBUG("Opening \"%s\" device\n", device_name);
	return (pll_dev_t)open(device_name, O_RDWR);
}

int	pll_dev_release( pll_dev_t device )
{
	__DEBUG("Releasing device\n");
	return close( (int)device );
}


/******************** High Level configuration functions ********************/
#define SYMETR_COUNTER_VALUE(x)		((x) | (x<<8))
#define SYMETR_COUNTER_BYPASS		(1<<16)
#define COUNTER_MASK				0x0003ffff


/* Works only for symtric counter values */
float pll_calculate_counters_brute_force(unsigned *m, unsigned *n, unsigned *c, float reference_freq, float desired_freq){
	int m_tmp = SYMETR_COUNTER_BYPASS;
	int n_tmp = SYMETR_COUNTER_BYPASS;
	int c_tmp = SYMETR_COUNTER_BYPASS;
	int m_best= SYMETR_COUNTER_BYPASS;
	int n_best= SYMETR_COUNTER_BYPASS;
	int c_best= SYMETR_COUNTER_BYPASS;
	float calculated_freq;
	float best_fit_freq = FLT_MAX;

	__DEBUG("pll_calculate_counters_brute_force(...) called\n");

	/* bypass n and c counters */
	for(m_tmp=1; m_tmp<255; m_tmp++){
		calculated_freq = reference_freq *  (2*(float)m_tmp);
		/* matching freuqncy found */
		if( calculated_freq == desired_freq){
			goto success_frequency_matched;
		}
		if( abs(calculated_freq - desired_freq) < abs(best_fit_freq - desired_freq) ){
			m_best = m_tmp;
			best_fit_freq = calculated_freq;
		}
	}

	/* bypass c counter */
	for(n_tmp=1; n_tmp<255; n_tmp++)
	for(m_tmp=1; m_tmp<255; m_tmp++){
		calculated_freq = reference_freq *  ( (float)m_tmp/(float)n_tmp );
		/* matching freuqncy found */
		if( calculated_freq == desired_freq){
			goto success_frequency_matched;
		}
		if( abs(calculated_freq - desired_freq) < abs(best_fit_freq - desired_freq) ){
			m_best = m_tmp;
			n_best = n_tmp;
			best_fit_freq = calculated_freq;
		}
	}

	/* use all counters */
	for(c_tmp=1; c_tmp<255; c_tmp++)
	for(n_tmp=1; n_tmp<255; n_tmp++)
	for(m_tmp=1; m_tmp<255; m_tmp++){
		calculated_freq = reference_freq *  ( (float)m_tmp/(float)(2*n_tmp*c_tmp));
		/* matching freuqncy found */
		if( calculated_freq == desired_freq){
			goto success_frequency_matched;
		}
		if( abs(calculated_freq - desired_freq) < abs(best_fit_freq - desired_freq) ){
			m_best = m_tmp;
			n_best = n_tmp;
			c_best = c_tmp;
			best_fit_freq = calculated_freq;
		}
	}

	/* no ideal match found */
	*m = SYMETR_COUNTER_VALUE(m_best) & COUNTER_MASK;
	*n = SYMETR_COUNTER_VALUE(n_best) & COUNTER_MASK;
	*c = SYMETR_COUNTER_VALUE(c_best) & COUNTER_MASK;
	return best_fit_freq;


success_frequency_matched:
	*m = SYMETR_COUNTER_VALUE(m_tmp) & COUNTER_MASK;
	*n = SYMETR_COUNTER_VALUE(n_tmp) & COUNTER_MASK;
	*c = SYMETR_COUNTER_VALUE(c_tmp) & COUNTER_MASK;
	return calculated_freq;
}



int   pll_reconfigure_basic(pll_dev_t device, unsigned m, unsigned n, unsigned c)
{
	int reconfigure = 0;

	__DEBUG("pll_reconfigure_basic(...) called\n");

	/* set counters */
	if (ioctl(device, PLL_CTL_M_COUNTER_WRITE, &m) == -1)
		return -1;

	if (ioctl(device, PLL_CTL_N_COUNTER_WRITE, &n) == -1)
		return -1;

	if (ioctl(device, PLL_CTL_C_COUNTER_WRITE, &c) == -1)
		return -1;

	if (ioctl(device, PLL_CTL_START_WRITE, &reconfigure) == -1)
		return -1;

	return 0;
}


/******************** Low Level write functions ********************/
int pll_mode_write 			(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_mode_write(...) called\n");
	return ioctl(device, PLL_CTL_MODE_WRITE, &value);
}

int pll_start_write 		(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_start_write(...) called\n");
	return ioctl(device, PLL_CTL_START_WRITE, &value);
}

int pll_n_counter_write 	(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_n_counter_write(...) called\n");
	return ioctl(device, PLL_CTL_N_COUNTER_WRITE, &value);
}

int pll_m_counter_write 	(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_m_counter_write(...) called\n");
	return ioctl(device, PLL_CTL_M_COUNTER_WRITE, &value);
}

int pll_c_counter_write 	(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_c_counter_write(...) called\n");
	return ioctl(device, PLL_CTL_C_COUNTER_WRITE, &value);
}

int pll_dynamic_shift_write (pll_dev_t device, unsigned value)
{
	__DEBUG("pll_dynamic_shift_write(...) called\n");
	return ioctl(device, PLL_CTL_DYNAMIC_SHIFT_MODE_WRITE, &value);
}

int pll_m_fract_write 		(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_m_fract_write(...) called\n");
	return ioctl(device, PLL_CTL_M_COUNTER_FRACT_WRITE, &value);
}

int pll_bandwidth_write		(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_bandwidth_write(...) called\n");
	return ioctl(device, PLL_CTL_BANDWIDTH_WRITE, &value);
}

int pll_charge_pump_write	(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_charge_pump_write(...) called\n");
	return ioctl(device, PLL_CTL_CHARGE_PUMP_WRITE, &value);
}

int pll_vco_div_write		(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_vco_div_write(...) called\n");
	return ioctl(device, PLL_CTL_VCO_DIV_WRITE, &value);
}

int pll_mif_base_write		(pll_dev_t device, unsigned value)
{
	__DEBUG("pll_mif_base_write(...) called\n");
	return ioctl(device, PLL_CTL_MIF_BASE_WRITE, &value);
}


/******************** Low Level write functions ********************/
int pll_mode_read 			(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_mode_read(...) called\n");
	return ioctl(device, PLL_CTL_MODE_READ, value);
}

int pll_status_read  		(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_status_read(...) called\n");
	return ioctl(device, PLL_CTL_STATUS_READ, value);
}

int pll_n_counter_read  	(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_n_counter_read(...) called\n");
	return ioctl(device, PLL_CTL_N_COUNTER_READ, value);
}

int pll_m_counter_read  	(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_m_counter_read(...) called\n");
	return ioctl(device, PLL_CTL_M_COUNTER_READ, value);
}

int pll_c_counter_read  	(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_c_counter_read(...) called\n");
	return ioctl(device, PLL_CTL_C_COUNTER_READ, value);
}

int pll_bandwidth_read 		(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_bandwidth_read(...) called\n");
	return ioctl(device, PLL_CTL_BANDWIDTH_READ, value);
}

int pll_charge_pump_read 	(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_charge_pump_read(...) called\n");
	return ioctl(device, PLL_CTL_CHARGE_PUMP_READ, value);
}

int pll_vco_div_read 		(pll_dev_t device, unsigned *value)
{
	__DEBUG("pll_vco_div_read(...) called\n");
	return ioctl(device, PLL_CTL_VCO_DIV_READ, value);
}

