#
# Makefile for I2C gsensor chip drivers.
#
#obj-$(CONFIG_SATDEV_CLK_WIFI) += clk-board.o
obj-$(CONFIG_SATDEV_CLK_WIFI) += hym8563.o

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif
