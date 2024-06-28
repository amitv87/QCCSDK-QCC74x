cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})

sdk_add_include_directories(.)

target_sources(app PRIVATE otbr_cmd.c)

sdk_set_main_file(main.c)

sdk_add_compile_definitions(-DCONFIG_COEX_WIFI_MODE)

project(openthread_wifi_br)
