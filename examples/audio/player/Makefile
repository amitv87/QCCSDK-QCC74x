SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../../..

export QCC74x_SDK_BASE

CHIP ?= qcc743
BOARD ?= qcc744dk
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(QCC74x_SDK_BASE)/project.build
$(shell ./tools/genromfs/genromfs -d romfs/ -f ./build/build_out/romfs.bin)
