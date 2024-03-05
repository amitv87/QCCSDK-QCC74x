sdk_generate_library()

sdk_add_include_directories(
    include
    src/qcc743/drv
    )

sdk_add_private_include_directories(
    src/qcc743/codec
    src/qcc743/drv
    src/qcc743/port
    )

sdk_library_add_sources(
    src/qcc743/codec/codec_debug.c
    src/qcc743/codec/codec_eqdrc.c
    src/qcc743/codec/codec_input.c
    src/qcc743/codec/codec_output.c
    src/qcc743/codec/codec_volume.c
    src/qcc743/codec/xcodec.c
    src/qcc743/drv/msp_qcc743_auadc.c
    src/qcc743/drv/msp_qcc743_aupwm.c
    src/qcc743/drv/msp_qcc743_dma.c
    src/qcc743/drv/msp_qcc743_glb.c
    src/qcc743/drv/msp_qcc743_glb_gpio.c
    src/qcc743/port/msp_sys.c
    src/qcc743/codec/test/test_output.c
    )

sdk_add_compile_options(--save-temps)
