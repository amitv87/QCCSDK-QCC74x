sdk_add_include_directories(include)

sdk_add_compile_definitions(-DWL_QCC74x_undef=0)
sdk_add_compile_definitions(-DWL_QCC743=1)
sdk_add_compile_definitions(-DWL_QCC74x_undefM=0)

sdk_add_static_library(lib-gcc_10.2.0-toolchain_V2.6.1/libqcc743_phyrf.a)