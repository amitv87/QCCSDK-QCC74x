SDK_DEMO_PATH ?= .
QCC74x_SDK_BASE ?= $(SDK_DEMO_PATH)/../..

export QCC74x_SDK_BASE

CHIP ?= qcc743
BOARD ?= qcc743dk
CROSS_COMPILE ?= riscv64-unknown-elf-
CONFIG_COREDUMP := n
CONFIG_BOOT2_VER ?= 1.0.0
CONFIG_BOOT2_ANTI_ROLLBACK_VER ?= 0

# add custom cmake definition
cmake_definition+=-DCONFIG_BOOT2_VER=${CONFIG_BOOT2_VER}
cmake_definition+=-DCONFIG_BOOT2_ANTI_ROLLBACK_VER=$(CONFIG_BOOT2_ANTI_ROLLBACK_VER)

include $(QCC74x_SDK_BASE)/project.build
