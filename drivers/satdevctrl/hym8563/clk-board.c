/*
 * drivers/power/axp_power/axp20-board.c
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <plat/sys_config.h>

struct hym_platform_info {
	int		id;
	const char	*name;
};

int clk_rtc_used = 0;
int clk_rtc_twi_id = 2;
int clk_rtc_twi_addr = 81;

static struct hym_platform_info hym_i2c_platform_data = {
    .name = "hym8563",
    .id = 0,
};

static struct i2c_board_info __initdata hym_i2c_board_info[] = {
	{
		.type = "i2c_hym8563",
		.platform_data  = &hym_i2c_platform_data,
	},
};

static int __init hym_board_init(void)
{
    int ret;
    printk("[i2c_hym8563] %s register the hym board.\n", __func__);
    
    ret = script_parser_fetch("satdev_ctrlpara", "clk_rtc_used", &clk_rtc_used, sizeof(int));
    if (ret)
    {
        printk("[i2c_hym8563] driver uning configuration failed(%d)\n", __LINE__);
        return -1;
    }

    if (clk_rtc_used)
    {
        ret = script_parser_fetch("satdev_ctrlpara", "clk_rtc_twi_id", &clk_rtc_twi_id, sizeof(int));
        if (ret)
        {
            printk("[i2c_hym8563] driver uning configuration failed(%d)\n", __LINE__);
            clk_rtc_twi_id = 2;
        }

        ret = script_parser_fetch("satdev_ctrlpara", "clk_rtc_twi_addr", &clk_rtc_twi_addr, sizeof(int));
        if (ret)
        {
            printk("[i2c_hym8563] driver uning configuration failed(%d)\n", __LINE__);
            clk_rtc_twi_addr = 81;
        } 
        hym_i2c_board_info[0].addr = clk_rtc_twi_addr;

        return i2c_register_board_info(clk_rtc_twi_id, hym_i2c_board_info,
				ARRAY_SIZE(hym_i2c_board_info));
    }
    else
        return -1;
}
device_initcall(hym_board_init);

MODULE_DESCRIPTION("RTC HYM8563 for generating 32Khz clock");
MODULE_AUTHOR("Jenus Zhang");
MODULE_LICENSE("GPL");
