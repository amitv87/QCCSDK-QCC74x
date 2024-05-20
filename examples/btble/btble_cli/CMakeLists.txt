cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})
target_sources(app PRIVATE
                   app/player/app_player.c
                   app/player/cli_player.c
                   app/bt/app_bt.c
                   multimedia_port.c
                   )
                   
sdk_add_include_directories(.)

sdk_set_main_file(main.c)

project(btble_cli)
