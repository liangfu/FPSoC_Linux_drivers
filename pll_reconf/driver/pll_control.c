/* pll_control.c - simple Linux platform driver for "Altera PLL Reconfig" IP Core.
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
 * This is a simple driver which can be used in combination with "Altera PLL
 * Reconfig" IP core to dynamically reprogram PLL. Driver uses ioctl interface.
 * API for this driver is in form of an shared library.
 *
 * For more information refer to "Implementing Fractional PLL
 * Reconfiguration with Altera PLL and Altera PLL Reconfig IP Cores"
 * Application Notes.
 *
 */

/* Linux driver related */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/uaccess.h>		/* copy to/from user */
#include <linux/idr.h> 			/* obtain id's */
#include <linux/cdev.h>

/* Platform driver related */
#include <linux/platform_device.h>  /* struct platform_driver */
#include <linux/of.h>				/* of_match_ptr() */
#include <linux/of_platform.h>

/* Command interface */
#include "pll_control.h"


/* Define default parameters */
#ifndef DRIVER_NODE_NAME
 	#define DRIVER_NODE_NAME	"pll_control"
#endif

#ifndef DCOMPATIBLE_STRING
 	#define DCOMPATIBLE_STRING	"pll_ctl"
#endif

#ifndef DEBUG_PLL_CTL 
	#define DEBUG_PLL_CTL 		0
#endif

#ifndef DPLL_IOC_MAGIC
 	#define DPLL_IOC_MAGIC 		0xf0
#endif


/* Driver private data */
struct pll_ctl_data{
	struct class 		*class;
	struct device 		*device;
	int 				minor;
	struct cdev 		cdev;
	struct resource 	*resource;

	unsigned 			base_addr;
	void 				*iomap;
};


/* MM address span of reconfig pll IP */
#define PLL_ADDR_SPAN		0xff
#define PLL_REQUEST_SPAN	0x04

#define PLL_MODE_OFFSET 				0x00
#define PLL_STATUS_OFFSET 				0x01
#define PLL_START_OFFSET 				0x02
#define PLL_N_COUNTER_OFFSET 			0x03
#define PLL_M_COUNTER_OFFSET 			0x04
#define PLL_C_COUNTER_OFFSET 			0x05
#define PLL_DYNAMIC_SHIFT_MODE_OFFSET 	0x06
#define PLL_M_COUNTER_FRACT_OFFSET 		0x07
#define PLL_BANDWIDTH_OFFSET 			0x08
#define PLL_CHARGE_PUMP_OFFSET 			0x09
#define PLL_VCO_DIV_OFFSET 				0x1C
#define PLL_MIF_BASE_OFFSET 			0x1F


/* Commonly used printk statements */
#define __ERROR(fmt,args...)		printk(KERN_ERR "PLL_CTL_ERROR: " fmt, ##args)
#define __INFO(fmt,args...)			printk(KERN_INFO "PLL_CTL_INFO: " fmt, ##args)

#if DEBUG_PLL_CTL == 1
	#define __DEBUG(fmt,args...)		printk(KERN_INFO "PLL_CTL_DEBUG: " fmt, ##args)
#else
	#define __DEBUG(fmt,args...)
#endif 


/* Global variables */
static struct class 	*pll_class;
static int 				pll_major;
static DEFINE_IDA(pll_ida);

/* platform driver functions */
static int pll_probe	(struct platform_device *pdev);
static int pll_remove	(struct platform_device *pdev);

/* character driver functions */
int pll_open	( struct inode *inode, struct file *filp );
int pll_release	( struct inode *inode, struct file *filp);
long pll_ioctl	(struct file *filp, unsigned int cmd, unsigned long arg);
long pll_ioctl_read	(struct file *filp, unsigned int cmd, unsigned long arg);
long pll_ioctl_write(struct file *filp, unsigned int cmd, unsigned long arg);


/* Platform Driver structures */
static const struct of_device_id pll_ctl_driver_id[] = {
	{.compatible = COMPATIBLE_STRING},
	{}
};
static struct platform_driver pll_driver = {
	.driver = {
		.name 	= DRIVER_NODE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(pll_ctl_driver_id)
	},
	.probe 	= pll_probe,
	.remove = pll_remove
};


/* fops */
static struct file_operations pll_fops = {
	.owner 			= THIS_MODULE,
	.open 			= pll_open,
	.release 		= pll_release,
	.unlocked_ioctl = pll_ioctl
};


int pll_open( struct inode *inode, struct file *filp )
{
	__DEBUG("pll_open called\n");

	/* save private data structure as for future reference */
	filp->private_data = container_of(inode->i_cdev, struct pll_ctl_data, cdev);

	return 0;
}

int pll_release( struct inode *inode, struct file *filp)
{
	__DEBUG("pll_release called\n");
	return 0;
}

long pll_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	__DEBUG("IOCTL command issued\n");

	/* check validity of the cmd */
	if(_IOC_TYPE(cmd) != PLL_IOC_MAGIC){
		__ERROR("IOCTL Incorrect magic number\n");
		return -ENOTTY;
	}
	if(_IOC_NR(cmd) > PLL_CTL_MAXNR){
		__ERROR("IOCTL Command is not valid\n");
		return -ENOTTY;
	}

	/* we have only read^write commands */
	if(_IOC_DIR(cmd) & _IOC_READ){
		return pll_ioctl_read(filp,cmd,arg);
	}else{
		return pll_ioctl_write(filp,cmd,arg);
	}
}


long pll_ioctl_read	(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pll_ctl_data *pll;
	unsigned offset, value;

	__DEBUG("IOCTL read command issued\n");

	/* validate pointer */
	if( !access_ok(VERIFY_READ, (void __user*) arg, _IOC_SIZE(cmd)) )
		return -EFAULT;

	switch(cmd){
		case PLL_CTL_MODE_READ:
			offset = PLL_MODE_OFFSET;
			break;
		case PLL_CTL_STATUS_READ:
			offset = PLL_STATUS_OFFSET;
			break;
		case PLL_CTL_N_COUNTER_READ:
			offset = PLL_N_COUNTER_OFFSET;
			break;
		case PLL_CTL_M_COUNTER_READ:
			offset = PLL_M_COUNTER_OFFSET;
			break;
		case PLL_CTL_C_COUNTER_READ:
			offset = PLL_C_COUNTER_OFFSET;
			break;
		case PLL_CTL_BANDWIDTH_READ:
			offset = PLL_BANDWIDTH_OFFSET;
			break;
		case PLL_CTL_CHARGE_PUMP_READ:
			offset = PLL_CHARGE_PUMP_OFFSET;
			break;
		case PLL_CTL_VCO_DIV_READ:
			offset = PLL_VCO_DIV_OFFSET;
			break;
		default:
			return -EFAULT;
	}

	/* Access private data */
	pll = filp->private_data;

	value = ioread32( (unsigned*)pll->iomap + offset);

	__DEBUG("Read address offset - 0x%x\n", offset);
	__DEBUG("Read value - 0x%x\n", value);

	/* Write read value to userspace */
	__put_user(value, (typeof(&value))arg);

	return 0;
}


long pll_ioctl_write(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pll_ctl_data *pll;
	unsigned offset, value;

	__DEBUG("IOCTL write command issued\n");

	/* validate pointer */
	if( !access_ok(VERIFY_WRITE, (void __user*) arg, _IOC_SIZE(cmd)) )
		return -EFAULT;

	__get_user(value, (typeof(&value))arg );

	switch(cmd){
		case PLL_CTL_MODE_WRITE:
			offset = PLL_MODE_OFFSET;
			break;
		case PLL_CTL_START_WRITE:
			offset = PLL_START_OFFSET;
			break;
		case PLL_CTL_N_COUNTER_WRITE:
			offset = PLL_N_COUNTER_OFFSET;
			break;
		case PLL_CTL_M_COUNTER_WRITE:
			offset = PLL_M_COUNTER_OFFSET;
			break;
		case PLL_CTL_C_COUNTER_WRITE:
			offset = PLL_C_COUNTER_OFFSET;
			break;
		case PLL_CTL_DYNAMIC_SHIFT_MODE_WRITE:
			offset = PLL_DYNAMIC_SHIFT_MODE_OFFSET;
			break;
		case PLL_CTL_M_COUNTER_FRACT_WRITE:
			offset = PLL_M_COUNTER_FRACT_OFFSET;
			break;
		case PLL_CTL_BANDWIDTH_WRITE:
			offset = PLL_BANDWIDTH_OFFSET;
			break;
		case PLL_CTL_CHARGE_PUMP_WRITE:
			offset = PLL_CHARGE_PUMP_OFFSET;
			break;
		case PLL_CTL_VCO_DIV_WRITE:
			offset = PLL_VCO_DIV_OFFSET;
			break;
		case PLL_CTL_MIF_BASE_WRITE:
			offset = PLL_MIF_BASE_OFFSET;
			break;
		default:
			return -EFAULT;
	}

	/* Access private data */
	pll = filp->private_data;

	__DEBUG("Write address offset - 0x%x\n", offset);
	__DEBUG("Write value - 0x%x\n", value);

	/* have to check */
	iowrite32(value, (unsigned*)pll->iomap + offset);

	return 0;
}



static int pll_probe(struct platform_device *pdev)
{
	int err;
	struct pll_ctl_data *pll;
	
	__DEBUG("pll_probe() called\n");

	/* allocate device related memory (freed automatially)*/
	pll = devm_kzalloc(&pdev->dev, sizeof(*pll), GFP_KERNEL);
	if( pll == NULL ){
		__DEBUG("Could not allocate devica associated memory\n");
		return -ENOMEM;
	}

	/* use ida for minor number generation */
	pll->minor = ida_simple_get(&pll_ida, 0, 0, GFP_KERNEL);
	if( pll->minor < 0 ){
		__DEBUG("Could not allocate id\n");
		return pll->minor;
	}

	/* associate cdev with file operations */
	cdev_init(&pll->cdev, &pll_fops);

	/* initialize and add device */
	pll->device = device_create(pll_class, NULL, MKDEV(pll_major, pll->minor), NULL, DRIVER_NODE_NAME"%d", pll->minor);
	if( IS_ERR(pll->device) ){
		__DEBUG("Failed to create device\n");
		err = PTR_ERR(pll->device);
		goto error_device_create;
	}

	/* make device accesable for users */
	err = cdev_add(&pll->cdev, MKDEV(pll_major, pll->minor), 1);
	if(err){
		__DEBUG("Could not add character device\n");
		goto error_cdev_add;
	}

	/* dtb info  */
	pll->resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(pll->resource == NULL){
		__DEBUG("Could not get platform resource\n");
		err = -ENXIO;
		goto error_platform_get_resource;
	}

	/* it might be logical, to put this into open file operation */
	pll->iomap = devm_ioremap_resource(&pdev->dev, pll->resource);
	if( IS_ERR(pll->iomap) ){
		__DEBUG("Could not map I/O region\n");
		err = PTR_ERR(pll->iomap);
		goto error_devm_ioremap_resource;
	}
	
	/* set private data reference */
    platform_set_drvdata(pdev, pll);

	return 0;


error_devm_ioremap_resource:
error_platform_get_resource:
	cdev_del(&pll->cdev);	

error_cdev_add:
	device_destroy(pll_class,  MKDEV(pll_major, pll->minor));

error_device_create:
	ida_simple_remove(&pll_ida, pll->minor);

	return err;
}


static int pll_remove(struct platform_device *pdev)
{
	struct pll_ctl_data *pll;

	__DEBUG("pll_remove() called\n");

	/* get private data */
	pll = platform_get_drvdata(pdev);

	devm_iounmap(&pdev->dev, pll->iomap);

	/* remove character device */
	cdev_del(&pll->cdev);

	/* release allocated id */
	ida_simple_remove(&pll_ida, pll->minor);

	/* remove char dev */
	device_destroy(pll_class,  MKDEV(pll_major, pll->minor));

	return 0;
}

static int pll_init(void)
{
	int err;
	__INFO("Initializing pll platform device driver\n");

	/* get major number for all pll control devices */
	pll_major = register_chrdev(0, DRIVER_NODE_NAME, &pll_fops);
	if( pll_major < 0 ){
		__ERROR("Failed to allocate major number\n");
		return pll_major;
	}

	/* create one class for all devices */
	pll_class = class_create(THIS_MODULE, DRIVER_NODE_NAME);
	if( IS_ERR(pll_class) ){
		__ERROR("Failed to create device class\n");
		err = IS_ERR(pll_class);
		goto error_class_create;
	}

	/* probe hardware */
	err = platform_driver_register(&pll_driver);
	if(err){
		__ERROR("Failed to register platform device\n");
		goto error_platform_driver_register;
	}

	return 0;


error_platform_driver_register:
	class_destroy(pll_class);

error_class_create:
	unregister_chrdev(pll_major, DRIVER_NODE_NAME);

	return err;
}


static void pll_exit(void)
{
	__INFO("Releasing pll platform device driver\n");

	platform_driver_unregister(&pll_driver);

	class_destroy(pll_class);

	unregister_chrdev(pll_major, DRIVER_NODE_NAME);
}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple driver for controlling reconfigurable altera pll IP core.");
module_init(pll_init);
module_exit(pll_exit);
