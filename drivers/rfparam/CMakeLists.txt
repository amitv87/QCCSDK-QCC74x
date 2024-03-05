sdk_generate_library()

sdk_library_add_sources(Src/rfparam_adapter.c)
sdk_library_add_sources(Src/rfparam_rftlv.c)

sdk_add_include_directories(Inc)

if("${CHIP}" STREQUAL "qcc743")
sdk_add_compile_definitions(-DRFPARAM_QCC743)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_add_compile_definitions(-DRFPARAM_QCC74x_undef)
endif()
