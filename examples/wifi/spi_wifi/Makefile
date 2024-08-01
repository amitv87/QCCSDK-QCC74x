SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../../../

export QCC74x_SDK_BASE

CHIP ?= qcc743
BOARD ?= qcc743dk
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

CONFIG_USE_LPAPP ?= 0
cmake_definition+=-DCONFIG_USE_LPAPP=$(CONFIG_USE_LPAPP)

include $(QCC74x_SDK_BASE)/project.build 
