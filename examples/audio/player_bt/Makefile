SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../../..
CONFIG_BOARD_CONFIG_8M ?= config_8M

export QCC74x_SDK_BASE
export CONFIG_BOARD_CONFIG_8M

CHIP ?= qcc743
BOARD ?= qcc744dk
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(QCC74x_SDK_BASE)/project.build
