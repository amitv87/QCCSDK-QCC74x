cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(qcc74x_sdk REQUIRED HINTS $ENV{QCC74x_SDK_BASE})


add_subdirectory(lua)

target_sources(app PRIVATE

)

sdk_add_include_directories(.)

sdk_set_main_file(main.c)

project(lua)
