sdk_generate_library()

sdk_add_include_directories(.)

if("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
qcc74x_undef/qcc74x_undef_lp.c
qcc74x_undef/qcc74x_undef_sys.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
qcc74x_undef/qcc74x_undef_lp.c
qcc74x_undef/qcc74x_undef_sys.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undefl")
sdk_library_add_sources(
qcc74x_undefl/qcc74x_undefl_lp.c
qcc74x_undefl/qcc74x_undefl_sys.c
)
elseif("${CHIP}" STREQUAL "qcc743")
sdk_library_add_sources(
qcc743/qcc743_lp.c
qcc743/qcc743_sys.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
qcc74x_undef/qcc74x_undef_lp.c
qcc74x_undef/qcc74x_undef_sys.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
qcc74x_undef/qcc74x_undef_lp.c
qcc74x_undef/qcc74x_undef_sys.c
)
endif()

sdk_add_compile_definitions(-DCONFIG_PM)
sdk_add_include_directories(${CHIP})

