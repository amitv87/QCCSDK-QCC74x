cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})

sdk_add_compile_definitions(-DLP_APP)
sdk_add_compile_definitions(-DCONFIG_CLI_CMD_ENABLE)
sdk_add_compile_definitions(-DconfigUSE_TICKLESS_IDLE=1)
sdk_add_compile_definitions(-Dconfig_CUSTOM_TICKLESS=2)
sdk_add_compile_definitions(-DCFG_QCC74x_WIFI_PS_ENABLE)
sdk_add_compile_definitions(-DCFG_PS)
sdk_add_compile_definitions(-DCONFIG_RWNX_LWIP)
#sdk_add_compile_definitions(-DconfigEXPECTED_IDLE_TIME_BEFORE_SLEEP=40)
sdk_add_compile_definitions(-DWPA_SUPPLICANT_CLEANUP_INTERVAL=1800)
sdk_add_compile_definitions(-DQCC74x_RA_EN=1)
sdk_add_compile_definitions(-DPM_PDS_LDO_LEVEL_SOC_DEFAULT=10)
sdk_add_compile_definitions(-DPM_PDS_LDO_LEVEL_AON_DEFAULT=10)

if (CONFIG_DCDC_ENABLE)
    sdk_add_compile_definitions(-DPM_PDS_LDO_LEVEL_RT_DEFAULT=8)
else ()
    sdk_add_compile_definitions(-DPM_PDS_LDO_LEVEL_RT_DEFAULT=10)
endif ()

sdk_add_compile_definitions(-DLP_FW_START_ADDR=0x63026800)
message(WARNING "lpfw bin load in WRAM(0x63026800)")

target_sources(app PRIVATE
    tickless.c)

sdk_add_include_directories(.)

sdk_set_main_file(main.c)

project(ble_lp)
