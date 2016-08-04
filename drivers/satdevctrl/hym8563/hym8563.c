/* drivers/rtc/rtc-HYM8563.c - driver for HYM8563
 *
 * Copyright (C) 2010 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <mach/gpio.h>
//#include <linux/platform_device.h>
#include <plat/sys_config.h>
#include "hym8563.h"

#define I2C_HYM8563_NAME "hym8563"
#define I2C_HYM8563_ADDR 81

//static int clk_rtc_used = 1;
static int clk_rtc_twi_id = 2;
//static int clk_rtc_twi_addr = 81;

static struct i2c_client *hym8563_client = NULL;

static int hym8563_i2c_read_regs(struct i2c_client *client, u8 reg, u8 buf[], unsigned len)
{
	int ret;
	ret = i2c_smbus_read_i2c_block_data(client, reg, len, buf);
	return ret;
}

static int hym8563_i2c_set_regs(struct i2c_client *client, u8 reg, u8 const buf[], __u16 len)
{
	int ret;
	ret = i2c_smbus_write_i2c_block_data(client, reg, len, buf);
	return ret;
}

/*the init of the hym8563 at first time */
static int hym8563_init_device(struct i2c_client *client)
{
	u8 regs[2];
	int sr;
	//printk("[HYM-8563] %s info: I2C addr = %4x\n",__func__,client->addr);

	/* Clear stop flag if present */
	/*
	regs[0]=0;
	sr = hym8563_i2c_set_regs(client, RTC_CTL1, regs, 1);
	if (sr < 0){
	    printk("[HYM-8563] %s clear stop flag failed\n",__func__);
		goto exit;
	}
	*/

	//enable clkout
	regs[0] = 0x0;
	sr = hym8563_i2c_set_regs(client, RTC_CLKOUT, regs, 1);
	if (sr < 0){
	    printk("[HYM-8563] %s enable clkout failed\n",__func__);
		goto exit;
	}

	/*disable alarm && count interrupt*/
	sr = hym8563_i2c_read_regs(client, RTC_CTL2, regs, 1);
	if (sr < 0){
	    printk("[HYM-8563] %s disable alarm and count interrupt failed (read)\n",__func__);
		goto exit;
	}

	regs[0] = 0x0;
	regs[0] &= AIE;
	regs[0] &= TIE;
	sr = hym8563_i2c_set_regs(client, RTC_CTL2, regs, 1);
	if (sr < 0) {
	    printk("[HYM-8563] %s disable alarm and count interrupt failed (set)\n",__func__);
		goto exit;
	}

	sr = hym8563_i2c_read_regs(client, RTC_CTL2, regs, 1);
	if (sr < 0){
        printk("[HYM-8563] %s first read register failed\n",__func__);
		goto exit;
	}

	sr = hym8563_i2c_read_regs(client, RTC_CTL2, regs, 1);
	if (sr < 0) {
		printk("[HYM-8563] %s second read register failed\n",__func__);
		goto exit;
	}

	/* Clear any pending alarm and timer flags */
	if (regs[0] & AF)
		regs[0] &= ~AF;

	if (regs[0] & TF)
		regs[0] &= ~TF;

	regs[0] &= ~TI;
	sr = hym8563_i2c_set_regs(client, RTC_CTL2, regs, 1);


	if(sr <0){
	    printk("[HYM-8563] %s device initializing failed !!!\n",__func__);
	}else{
	    printk("[HYM-8563] %s device initializing OK !\n",__func__);
	}

exit:
	return sr;
}

static int hym8563_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	//u8 reg = 0;

	//printk("[HYM-8563] hym8563_probe info: I2C addr = %4x\n",client->addr);

	hym8563_client = client;

	// configuration
	//hym8563_init_device(client);

	// check power down
	//hym8563_i2c_read_regs(client,RTC_SEC,&reg,1);
	//if (reg&0x80) {
	//	printk("clock/calendar information is no longer guaranteed\n");
	//}

	return 0;
}

static int __devexit hym8563_remove(struct i2c_client *client)
{

	if (hym8563_client != NULL){
	    i2c_unregister_device(hym8563_client );
	}
	return 0;
}

static const struct i2c_device_id hym8563_id[] = {
	{I2C_HYM8563_NAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, hym8563_id);

static struct i2c_driver hym8563_driver = {
	.driver		= {
		.name	= "clk-hym8563",
		.owner	= THIS_MODULE,
	},
	.probe		= hym8563_probe,
	.remove		= __devexit_p(hym8563_remove),
	.id_table	= hym8563_id,
};

static struct i2c_board_info hym8563_board_info[] __initdata = {
    {
        I2C_BOARD_INFO(I2C_HYM8563_NAME,I2C_HYM8563_ADDR),
        .platform_data= NULL,
    },
};

static int __init hym8563_init(void)
{
    //printk("[HYM-8563] %s: start init.\n", __func__);
    struct i2c_adapter* adap = i2c_get_adapter(clk_rtc_twi_id);

    if(adap==NULL) {
        printk("[HYM-8563] %s i2c_get_adapter fail!\n",__func__);
        return -1;
    }

    hym8563_client = i2c_new_device(adap, &hym8563_board_info[0]);
    if(hym8563_client == NULL ){
        printk("[HYM-8563] %s i2c_new_device fail!\n",__func__);
        return -1;
    }

    i2c_put_adapter(adap);


	return i2c_add_driver(&hym8563_driver);
}

static void __exit hym8563_exit(void)
{
	i2c_del_driver(&hym8563_driver);
}

MODULE_AUTHOR("jenus zhang");
MODULE_DESCRIPTION("HYM8563 RTC driver");
MODULE_LICENSE("GPL");

module_init(hym8563_init);
module_exit(hym8563_exit);

