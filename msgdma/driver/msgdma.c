/* msgdma.c - Platform driver for Altera MSGDMA IP core on SoC devices.
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
 * This driver provides interface for multiple processes, to work with MSGDMA
 * devices (registered in DTB). To use IRQ functionality - first enable irq requests,
 * then add IRQ flag to descriptor. If flag is set then driver automatically adjusts
 * and waits for IRQ. This driver is provided together with an API library. For API
 * interface details refer to "include/msgdma_api.h" header file.
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

/* Platform driver specific includes */
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of.h>			


/* Shared include */
#include "msgdma.h"



#ifndef	MSGDMA_DEBUG
	#define MSGDMA_DEBUG	0
#endif


#define __ERROR(fmt,args...)		printk(KERN_ERR "MSGDMA_ERROR: " fmt, ##args)
#define __INFO(fmt,args...)			printk(KERN_INFO "MSGDMA_INFO: " fmt, ##args)


#if MSGDMA_DEBUG == 1
	#define __DEBUG(fmt,args...)		printk(KERN_INFO "MSGDMA_DEBUG: " fmt, ##args)
#else
	#define __DEBUG(fmt,args...)
#endif


/* Descriptor register offsets */
#define DSCR_READ_OFFSET 			0x00
#define DSCR_WRITE_OFFSET 			0x04
#define DSCR_LENGTH_OFFSET			0x08
#define DSCR_CONTROL_OFFSET 		0x0c
#define DSCR_WRITE_BURST_OFFSET		0x0c
#define DSCR_READ_BURST_OFFSET 		0x0d
#define DSCR_SEQUENCE_OFFSET 		0x0e
#define DSCR_WRITE_STRIDE_OFFSET 	0x10
#define DSCR_READ_STRIDE_OFFSET		0x12
#define DSCR_READ_HIGH_OFFSET 		0x14
#define DSCR_WRITE_HIGH_OFFSET 		0x18
#define DSCR_CONTROL_EXT_OFFSET 	0x1C

/* CSR register offsets */
#define CSR_STATUS_OFFSET 			0x00
#define CSR_CONTROL_OFFSET			0x04
#define CSR_WRITE_FILL_OFFSET 		0x08
#define CSR_READ_FILL_OFFSET 		0x0a
#define CSR_RESPONSE_FILL_OFFSET 	0x0e
#define CSR_WRITE_SEQ_NUM_OFFSET 	0x10
#define CSR_READ_SEQ_NUM_OFFSET 	0x12

/* Some useful defines */
#define CSR_STATUS_BUSY_BIT				(1<<0)
#define CSR_STATUS_IRQ_BIT 				(1<<9)
#define CSR_GLOBAL_IRQ_MASK_BIT			(1<<4)
#define CSR_RESET_DISPATCHER			(1<<1)
#define DSCR_TRANSFER_COMPLETE_IRQ_BIT	(1<<14)
#define DSCR_EARLY_TERMINATION_IRQ_BIT	(1<<15)
#define DSCR_TRANSFER_GO_BIT 			(1<<31)


#define EXTENDED_DESCRIPTOR_SPAN 		0x20
#define PROCESS_SUSPEND_TIMEOUT_MSEC	2000

struct msgdma_private_data {
	int 				minor;
	struct cdev 		cdev;
	struct device 		*device;
	struct resource 	*csr;
	struct resource 	*dscr;
	int 	 			irq_num;
	void 				*csr_iomap;
	void 				*dscr_iomap;
	int 				dscr_extended;
	wait_queue_head_t 	wait_queue;
	int 				sleeping;
};


/* Global variables */
static DEFINE_IDA(msgdma_ida);
struct class 	*msgdma_class;
int major;


/* platform device specific functions */
static int msgdma_probe(struct platform_device *pdev);
static int msgdma_remove(struct platform_device *pdev);


/* character driver functions */
int msgdma_open				( struct inode *inode, struct file *filp);
int msgdma_release			( struct inode *inode, struct file *filp);
long msgdma_ioctl			(struct file *filp, unsigned int cmd, unsigned long arg);

long msgdma_write_std_dscr		(struct file *filp, unsigned int cmd, unsigned long arg);
long msgdma_write_ext_dscr		(struct file *filp, unsigned int cmd, unsigned long arg);
long msgdma_enable_global_IRQ	(struct file *filp, unsigned int cmd, unsigned long arg);
long msgdma_disable_global_IRQ	(struct file *filp, unsigned int cmd, unsigned long arg);
long msgdma_is_busy 			(struct file *filp, unsigned int cmd, unsigned long arg);
long msgdma_reset_dispatcher	(struct file *filp, unsigned int cmd, unsigned long arg);


static struct file_operations msgdma_fops = {
	.owner 			= 	THIS_MODULE,
	.open 			= 	msgdma_open,
	.release 		= 	msgdma_release,
	.unlocked_ioctl = 	msgdma_ioctl
};


/* Platform Driver structures */
static const struct of_device_id msgdma_id[] = {
	{.compatible = COMPATIBLE_STRING},
	{}
};
static struct platform_driver msgdma_driver = {
	.driver = {
		.name 	= DRIVER_NODE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(msgdma_id)
	},
	.probe 	= msgdma_probe,
	.remove = msgdma_remove
};


static irqreturn_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	__DEBUG("Interrupt %d recieved!\n", irq);
	
	/* remove IRQ flag*/
	__DEBUG("Removing IRQ bit\n");
	iowrite32(CSR_STATUS_IRQ_BIT,	((struct msgdma_private_data*)dev_id)->csr_iomap + CSR_STATUS_OFFSET);

	/* TODO - find a wat to remove this flag */
	__DEBUG("Clearing sleep flag\n");
	((struct msgdma_private_data*)dev_id)->sleeping = 0;

	/* wake up waiting process */
	__DEBUG("Waking up process\n");
	wake_up_interruptible( &( ( (struct msgdma_private_data *)dev_id)->wait_queue));


	return IRQ_HANDLED;
}


int msgdma_open( struct inode *inode, struct file *filp)
{
	__DEBUG("msgdma_open called\n");

	/* Save reference to private data */
	filp->private_data = container_of(inode->i_cdev, struct msgdma_private_data, cdev);
	return 0;
}


int msgdma_release( struct inode *inode, struct file *filp)\
{
	__DEBUG("msgdma_release called\n");
	return 0;
}


long msgdma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	__DEBUG("IOCTL command issued\n");

	/* check validity of the cmd */
	if(_IOC_TYPE(cmd) != MSGDMA_IOCTL_MAGIC){
		__ERROR("IOCTL Incorrect magic number");
		return -ENOTTY;
	}
	if(_IOC_NR(cmd) > MSGDMA_IOCTL_MAXNR){
		__ERROR("IOCTL Command is not valid");
		return -ENOTTY;
	}

	/* Select command (start with control for measurement accuracy) */
	if( cmd == MSGDMA_WRITE_STD_DSCR ){
		return msgdma_write_std_dscr(filp, cmd, arg);
	}
	else if( cmd == MSGDMA_WRITE_EXT_DSCR ){
		return msgdma_write_ext_dscr(filp, cmd, arg);
	}
	else if( cmd == MSGDMA_ENABLE_IRQ_MASK){
		return msgdma_enable_global_IRQ(filp, cmd, arg);
	}
	else if( cmd == MSGDMA_DISABLE_IRQ_MASK){
		return msgdma_disable_global_IRQ(filp, cmd, arg);
	} 
	else if(cmd == MSGDMA_IS_BUSY_MASK){
		return msgdma_is_busy(filp, cmd, arg);
	}
	else if(cmd == MSGDMA_RESET_MASK){
		return msgdma_reset_dispatcher(filp, cmd, arg);
	}


	return -ENOTTY;
}


long msgdma_write_std_dscr	(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	struct msgdma_dscr dscr;

	__DEBUG("msgdma_write_std_dscr called\n");

	/* access private data */
	msgdma = filp->private_data;

	if( msgdma->dscr_extended )
		return -EINVAL;

	/* TODO: checks */
	__get_user(dscr.read_addr,	(typeof(&dscr.read_addr))(arg + offsetof(struct msgdma_dscr, read_addr)));
	__get_user(dscr.write_addr,	(typeof(&dscr.write_addr))(arg + offsetof(struct msgdma_dscr, write_addr)));
	__get_user(dscr.length,		(typeof(&dscr.length))(arg + offsetof(struct msgdma_dscr, length)));
	__get_user(dscr.control,	(typeof(&dscr.control))(arg + offsetof(struct msgdma_dscr, control)));

	__DEBUG("dscr->read_addr - 0x%x\n", 	dscr.read_addr);
	__DEBUG("dscr->write_addr - 0x%x\n", 	dscr.write_addr);
	__DEBUG("dscr->length - 0x%x\n", 		dscr.length);
	__DEBUG("dscr->control - 0x%x\n", 		dscr.control);

	/* start transaction TODO: checks */
	iowrite32(dscr.read_addr, 						msgdma->dscr_iomap + DSCR_READ_OFFSET);
	iowrite32(dscr.write_addr, 						msgdma->dscr_iomap + DSCR_WRITE_OFFSET);
	iowrite32(dscr.length, 							msgdma->dscr_iomap + DSCR_LENGTH_OFFSET);
	iowrite32(dscr.control | DSCR_TRANSFER_GO_BIT, msgdma->dscr_iomap + DSCR_CONTROL_OFFSET);


	/* if IRQ enabled, put process to sleep */
	if ( (dscr.control & DSCR_EARLY_TERMINATION_IRQ_BIT) ||
		 (dscr.control & DSCR_TRANSFER_COMPLETE_IRQ_BIT) ){

		/* TODO -> atomic */
		msgdma->sleeping = 1;
		wait_event_interruptible_timeout(msgdma->wait_queue, msgdma->sleeping == 0, PROCESS_SUSPEND_TIMEOUT_MSEC*HZ/1000);
	}

	return 0;
}


long msgdma_write_ext_dscr	(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	struct msgdma_dscr_extended dscr;

	__DEBUG("msgdma_write_ext_dscr called\n");

	/* access private data */
	msgdma = filp->private_data;

	if( !msgdma->dscr_extended )
		return -EINVAL;

	/* TODO: checks */
	__get_user(dscr.read_addr,			(typeof(&dscr.read_addr))(arg + offsetof(struct msgdma_dscr_extended, read_addr)));
	__get_user(dscr.write_addr,			(typeof(&dscr.write_addr))(arg + offsetof(struct msgdma_dscr_extended, write_addr)));
	__get_user(dscr.length,				(typeof(&dscr.length))(arg + offsetof(struct msgdma_dscr_extended, length)));
	__get_user(dscr.read_burst_count,	(typeof(&dscr.read_burst_count))(arg + offsetof(struct msgdma_dscr_extended, read_burst_count)));
	__get_user(dscr.write_burst_count,	(typeof(&dscr.write_burst_count))(arg + offsetof(struct msgdma_dscr_extended, write_burst_count)));
	__get_user(dscr.seq_number,			(typeof(&dscr.seq_number))(arg + offsetof(struct msgdma_dscr_extended, seq_number)));
	__get_user(dscr.read_stride,		(typeof(&dscr.read_stride))(arg + offsetof(struct msgdma_dscr_extended, read_stride)));
	__get_user(dscr.write_stride,		(typeof(&dscr.write_stride))(arg + offsetof(struct msgdma_dscr_extended, write_stride)));
	__get_user(dscr.read_addr_high,		(typeof(&dscr.read_addr_high))(arg + offsetof(struct msgdma_dscr_extended, read_addr_high)));
	__get_user(dscr.write_addr_high,	(typeof(&dscr.write_addr_high))(arg + offsetof(struct msgdma_dscr_extended, write_addr_high)));
	__get_user(dscr.control,			(typeof(&dscr.control))(arg + offsetof(struct msgdma_dscr_extended, control)));


	__DEBUG("dscr.read_addr - 0x%x\n", dscr.read_addr);
	__DEBUG("dscr.write_addr - 0x%x\n", dscr.write_addr);
	__DEBUG("dscr.length - 0x%x\n", dscr.length);
	__DEBUG("dscr.read_burst_count - 0x%x\n", dscr.read_burst_count);
	__DEBUG("dscr.write_burst_count - 0x%x\n", dscr.write_burst_count);
	__DEBUG("dscr.seq_number - 0x%x\n", dscr.seq_number);
	__DEBUG("dscr.read_stride - 0x%x\n", dscr.read_stride);
	__DEBUG("dscr.write_stride - 0x%x\n", dscr.write_stride);
	__DEBUG("dscr.read_addr_high - 0x%x\n", dscr.read_addr_high);
	__DEBUG("dscr.write_addr_high - 0x%x\n", dscr.write_addr_high);
	__DEBUG("dscr.control - 0x%x\n", dscr.control);


	/* start transaction TODO: checks */
	iowrite32(dscr.read_addr, 			msgdma->dscr_iomap + DSCR_READ_OFFSET);
	iowrite32(dscr.write_addr, 			msgdma->dscr_iomap + DSCR_WRITE_OFFSET);
	iowrite32(dscr.length, 				msgdma->dscr_iomap + DSCR_LENGTH_OFFSET);
	iowrite8(dscr.read_burst_count, 	msgdma->dscr_iomap + DSCR_READ_BURST_OFFSET);
	iowrite8(dscr.write_burst_count,	msgdma->dscr_iomap + DSCR_WRITE_BURST_OFFSET);
	iowrite16(dscr.seq_number, 			msgdma->dscr_iomap + DSCR_SEQUENCE_OFFSET);
	iowrite16(dscr.read_stride, 		msgdma->dscr_iomap + DSCR_WRITE_STRIDE_OFFSET);
	iowrite16(dscr.write_stride, 		msgdma->dscr_iomap + DSCR_READ_STRIDE_OFFSET);
	iowrite32(dscr.read_addr_high, 		msgdma->dscr_iomap + DSCR_READ_HIGH_OFFSET);
	iowrite32(dscr.write_addr_high, 	msgdma->dscr_iomap + DSCR_WRITE_HIGH_OFFSET);
	iowrite32(dscr.control, 			msgdma->dscr_iomap + DSCR_CONTROL_EXT_OFFSET);


	/* if IRQ enabled, put process to sleep */
	if ( (dscr.control & DSCR_EARLY_TERMINATION_IRQ_BIT) ||
		 (dscr.control & DSCR_TRANSFER_COMPLETE_IRQ_BIT) ){

		/* TODO -> atomic */
		msgdma->sleeping = 1;
		wait_event_interruptible_timeout(msgdma->wait_queue, msgdma->sleeping == 0, PROCESS_SUSPEND_TIMEOUT_MSEC*HZ/1000);
	}

	return 0;
}


long msgdma_enable_global_IRQ	(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	unsigned value;

	__DEBUG("msgdma_enable_global_IRQ called\n");

	/* access private data */
	msgdma = filp->private_data;

	value = ioread32(msgdma->csr_iomap + CSR_CONTROL_OFFSET);
	iowrite32(value | CSR_GLOBAL_IRQ_MASK_BIT, msgdma->csr_iomap + CSR_CONTROL_OFFSET);

	return 0;
}


long msgdma_disable_global_IRQ	(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	unsigned value;

	__DEBUG("msgdma_disable_global_IRQ called\n");

	/* access private data */
	msgdma = filp->private_data;

	value = ioread32(msgdma->csr_iomap + CSR_CONTROL_OFFSET);
	iowrite32(value & (~CSR_GLOBAL_IRQ_MASK_BIT), msgdma->csr_iomap + CSR_CONTROL_OFFSET);

	return 0;
}

long msgdma_is_busy(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	int value;

	__DEBUG("msgdma_is_busy called\n");

	/* access private data */
	msgdma = filp->private_data;

	value = ioread32(msgdma->csr_iomap + CSR_STATUS_OFFSET) & CSR_STATUS_BUSY_BIT;
	__put_user(value, (int*)arg);

	return 0;
}

long msgdma_reset_dispatcher(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct msgdma_private_data *msgdma;
	unsigned value;

	__DEBUG("msgdma_is_busy called\n");

	/* access private data */
	msgdma = filp->private_data;

	value = ioread32(msgdma->csr_iomap + CSR_CONTROL_OFFSET);
	iowrite32(value | CSR_RESET_DISPATCHER, msgdma->csr_iomap + CSR_CONTROL_OFFSET);

	return 0;
}


static int msgdma_probe(struct platform_device *pdev)
{
	int err;
	struct msgdma_private_data *private_data;

	__DEBUG("msgdma_probe() called\n");

	/* allocate private data structure (freed automatically) */
	private_data = devm_kzalloc(&pdev->dev, sizeof(*private_data), GFP_KERNEL);

	private_data->minor = ida_simple_get(&msgdma_ida, 0, 0, GFP_KERNEL);

	/* cdev interface (used to get private references from struct file) */
	cdev_init(&private_data->cdev, &msgdma_fops);

	/* initialize and add device */
	private_data->device = device_create(msgdma_class, NULL, MKDEV(major, private_data->minor), NULL, DRIVER_NODE_NAME"%d", private_data->minor);
	if( private_data->device == (struct device*)ERR_PTR ){
		__ERROR("Failed to create device\n");
		return -EIO;
	}

	cdev_add(&private_data->cdev, MKDEV(major, private_data->minor), 1);

	/* get resources resources */
	private_data->csr = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	private_data->dscr = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	private_data->irq_num = platform_get_irq(pdev, 0);

	/* dscr resource */
	private_data->csr_iomap = devm_ioremap_resource(&pdev->dev, private_data->csr);
	private_data->dscr_iomap = devm_ioremap_resource(&pdev->dev, private_data->dscr);

	/* set irq handler */
	err = request_irq(private_data->irq_num, (irq_handler_t)interrupt_handler, 0, dev_name(private_data->device), (void*)private_data );
	if(err){
		__ERROR("Could not register interrupt");
		goto error_request_irq;
	}

	init_waitqueue_head( &private_data->wait_queue );

	private_data->dscr_extended = 
		( resource_size(private_data->dscr) == EXTENDED_DESCRIPTOR_SPAN ) ? 1 : 0;

	__DEBUG("Extended descriptor: %d\n", private_data->dscr_extended);

	/* set private data reference */
    platform_set_drvdata(pdev, private_data);

	return 0;

error_request_irq:
	device_destroy(msgdma_class,  MKDEV(major, private_data->minor));

	return err;
}


static int msgdma_remove(struct platform_device *pdev)
{
	struct msgdma_private_data *private_data;

	__DEBUG("msgdma_remove() called\n");

	private_data = platform_get_drvdata(pdev);

	free_irq(private_data->irq_num, (void*)private_data );

	devm_iounmap(&pdev->dev, private_data->csr_iomap);
	devm_iounmap(&pdev->dev, private_data->dscr_iomap);

	cdev_del( &private_data->cdev );

	device_destroy(msgdma_class,  MKDEV(major, private_data->minor));

	ida_simple_remove(&msgdma_ida, private_data->minor);

	return 0;
}


static int msgdma_init(void)
{
	int err;

	__INFO("Initializing MSGDMA driver\n");

	/* register character driver to get Major number for all devices */
	major = register_chrdev(0, DRIVER_NODE_NAME, &msgdma_fops);
	if( major < 0 ){
		__ERROR("Failed to allocate major number\n");
		return  major;
	}

	/* create class for all msgdma devices */
	msgdma_class = class_create(THIS_MODULE, DRIVER_NODE_NAME);
	if( IS_ERR(msgdma_class) ){
		__ERROR("Failed to create class");
		err = IS_ERR(msgdma_class);
		goto error_class_create;
	}


	/* register platform device */
	err = platform_driver_register(&msgdma_driver);
	if( err ){
		__ERROR("Failed to register platform driver");
		goto error_platform_driver_register;
	}

	return 0;

error_platform_driver_register:
	class_destroy(msgdma_class);

error_class_create:
	unregister_chrdev(major, DRIVER_NODE_NAME);

	return err;
}


static void msgdma_exit(void)
{

	__INFO("Exiting MSGDMA driver\n");

	/* unregister character driver */
	platform_driver_unregister(&msgdma_driver);

	/* destroy class */
	class_destroy(msgdma_class);

	/* unregister platform device */
	unregister_chrdev(major, DRIVER_NODE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver provides Altera's MSGDMA IP core dispatcher control interface");
module_init(msgdma_init);
module_exit(msgdma_exit);



