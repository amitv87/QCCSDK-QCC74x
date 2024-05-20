SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../../..

export QCC74x_SDK_BASE

CHIP ?= qcc743
BOARD ?= qcc743_lp_dk
CROSS_COMPILE ?= riscv64-unknown-elf-

CONFIG_LPAPP ?= y

# add custom cmake definition
# cmake_definition+=-Dxxx=sss

include $(QCC74x_SDK_BASE)/project.build
