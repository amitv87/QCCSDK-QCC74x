SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../../../..

export QCC74x_SDK_BASE

CHIP ?= qcc74x_undef
BOARD ?= qcc74x_undefdk
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(QCC74x_SDK_BASE)/project.build
