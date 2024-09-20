SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../..

export QCC74x_SDK_BASE

CHIP ?= qcc743
BOARD ?= qcc743dk
CROSS_COMPILE ?= riscv64-unknown-elf-

DISABLE_RELEASE_MFG ?= 0
cmake_definition+=-DDISABLE_RELEASE_MFG=$(DISABLE_RELEASE_MFG)

DISABLE_MFG_AUTOBOOT ?= 0
cmake_definition+=-DDISABLE_MFG_AUTOBOOT=$(DISABLE_MFG_AUTOBOOT)

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(QCC74x_SDK_BASE)/project.build
