sdk_generate_library()
sdk_add_include_directories(./include)
sdk_add_private_compile_definitions(-DQCC74x_MFD_PLAT_H="qcc_mfd_qcc743.h")
sdk_library_add_sources(qcc_mfd.c qcc_mfd_decrypt_qcc743.c)
