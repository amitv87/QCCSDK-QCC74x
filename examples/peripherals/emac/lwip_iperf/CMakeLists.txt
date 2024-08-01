cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})

sdk_add_include_directories(.)

target_sources(app PRIVATE eth_iperf_server.c)

sdk_add_compile_definitions(-D${CONFIG_ETHERNET_PHY})

sdk_set_main_file(main.c)

project(lwip_iperf)
