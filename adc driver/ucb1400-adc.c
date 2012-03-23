/*
 *  linux/drivers/mfd/ucb1x00-assabet.c
 *
 *  Copyright (C) 2001-2003 Russell King, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 *  We handle the machine-specific bits of the UCB1x00 driver here.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>

#include "ucb1x00.h"

#define UCB1X00_ATTR(name,input)\
static ssize_t name##_show(struct class_device *dev, char *buf)	\
{								\
	struct ucb1x00 *ucb = classdev_to_ucb1x00(dev);		\
	int val;						\
	ucb1x00_adc_enable(ucb);				\
	val = ucb1x00_adc_read(ucb, input, UCB_NOSYNC);		\
	ucb1x00_adc_disable(ucb);				\
	return sprintf(buf, "%d\n", val);			\
}								\
static CLASS_DEVICE_ATTR(name,0444,name##_show,NULL)

UCB1X00_ATTR(AD0, UCB_ADC_INP_AD0);
UCB1X00_ATTR(AD1, UCB_ADC_INP_AD1);
UCB1X00_ATTR(AD2, UCB_ADC_INP_AD2);
UCB1X00_ATTR(AD3, UCB_ADC_INP_AD3);

static int ucb1x00_assabet_add(struct ucb1x00_dev *dev)
{
	class_device_create_file(&dev->ucb->cdev, &class_device_attr_AD0);
	class_device_create_file(&dev->ucb->cdev, &class_device_attr_AD1);
	class_device_create_file(&dev->ucb->cdev, &class_device_attr_AD2);
	class_device_create_file(&dev->ucb->cdev, &class_device_attr_AD3);
	return 0;
}

static void ucb1x00_assabet_remove(struct ucb1x00_dev *dev)
{
	class_device_remove_file(&dev->ucb->cdev, &class_device_attr_AD0);
	class_device_remove_file(&dev->ucb->cdev, &class_device_attr_AD1);
	class_device_remove_file(&dev->ucb->cdev, &class_device_attr_AD2);
	class_device_remove_file(&dev->ucb->cdev, &class_device_attr_AD3);
}

static struct ucb1x00_driver ucb1x00_assabet_driver = {
	.add	= ucb1x00_assabet_add,
	.remove	= ucb1x00_assabet_remove,
};

static int __init ucb1x00_assabet_init(void)
{
	return ucb1x00_register_driver(&ucb1x00_assabet_driver);
}

static void __exit ucb1x00_assabet_exit(void)
{
	ucb1x00_unregister_driver(&ucb1x00_assabet_driver);
}

module_init(ucb1x00_assabet_init);
module_exit(ucb1x00_assabet_exit);

MODULE_AUTHOR("Russell King <rmk@arm.linux.org.uk>");
MODULE_DESCRIPTION("Assabet noddy testing only example ADC driver");
MODULE_LICENSE("GPL");
