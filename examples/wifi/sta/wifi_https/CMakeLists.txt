cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})

# User
sdk_add_compile_definitions(-DCONFIG_CLI_CMD_ENABLE)
sdk_add_compile_definitions(-DQCC74x_VERIFY)
#sdk_add_compile_definitions(-DQCC743_DHCP_DEBUG)

target_sources(app PRIVATE
                   https/src/https.c
                   https/src/qcc74x_https.c
                   ssl_rev_client.c
                   )

sdk_add_include_directories(.)
sdk_add_include_directories(./https/include)

sdk_set_main_file(main.c)

project(wifi_https)
