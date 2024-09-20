cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})

sdk_add_include_directories(.)

if ("${OT_SERIAL}" STREQUAL "SYS_CLI")
sdk_add_compile_definitions(-DOT_SERIAL_SHELL)
elseif ("${OT_SERIAL}" STREQUAL "UART") 
sdk_add_compile_definitions(-DOT_SERIAL_UART)
endif()

if(CONFIG_LMAC154_LOG_ENABLE)
sdk_add_compile_definitions(-DCONFIG_LMAC154_LOG_ENABLE=1)
sdk_add_compile_definitions(-DAPP_PRINT=lmac154_log)
else()
sdk_add_compile_definitions(-DAPP_PRINT=printf)
endif()

sdk_set_main_file(main.c)

project(openthread_cli)
