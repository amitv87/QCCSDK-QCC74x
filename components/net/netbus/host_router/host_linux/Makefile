CONFIG_QCC74x_INTF ?=

DEBUG = n

ifeq ($(DEBUG),y)
	MY_CFLAGS = -O -g
else
	MY_CFLAGS = -O2
endif

EXTRA_CFLAGS += $(MY_CFLAGS)

MODULE_NAME = qcwnet

ifeq ($(CONFIG_QCC74x_INTF),)
$(error "Define CONFIG_QCC74x_INTF as USB or SDIO")
endif

SRC := main.c wifi.c msg_handlers.c ctl_port.c
ifeq ($(CONFIG_QCC74x_INTF), USB)
ccflags-y += -DQCC74x_INTF_USB
SRC += qcc74x_usb_eth.c usb_msg_handlers.c
else
ccflags-y += -DQCC74x_INTF_SDIO
SRC += qcc74x_sdio_eth.c sdio_msg_handlers.c qcc74x_sdio.c tty.c
endif

TARGET=PC
export TARGET

CPPFLAGS :=

KDIR := /lib/modules/$(shell uname -r)/build

$(MODULE_NAME)-objs = $(patsubst %.c,%.o, $(SRC))

obj-m := $(MODULE_NAME).o

PWD := $(shell pwd)

all: lkm userspace

lkm:
	KCPPFLAGS="$(CPPFLAGS)" $(MAKE) -C $(KDIR) M=$(PWD) modules

.PHONY: userspace
userspace:
	$(MAKE) -C userspace

clean:
	KCPPFLAGS="$(CPPFLAGS)" $(MAKE) -C $(KDIR) M=$(PWD) clean
	$(MAKE) -C userspace clean
