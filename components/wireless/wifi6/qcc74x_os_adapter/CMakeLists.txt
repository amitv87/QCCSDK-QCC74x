sdk_generate_library()

sdk_library_add_sources(src/rtos_al.c)
sdk_library_add_sources(src/platform_qcc74x_sdk.c)
sdk_add_compile_definitions(-Dqcc74x_SDK)

sdk_add_include_directories(include)
