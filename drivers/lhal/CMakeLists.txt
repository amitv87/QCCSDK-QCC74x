sdk_generate_library()

sdk_library_add_sources(src/qcc74x_common.c)

sdk_library_add_sources(
src/qcc74x_adc.c
src/qcc74x_acomp.c
src/qcc74x_cks.c
src/qcc74x_ef_ctrl.c
src/qcc74x_gpio.c
src/qcc74x_i2c.c
src/qcc74x_dma.c
src/qcc74x_rtc.c
src/qcc74x_sec_aes.c
src/qcc74x_sec_sha.c
src/qcc74x_sec_trng.c
src/qcc74x_spi.c
src/qcc74x_timer.c
src/qcc74x_uart.c
src/qcc74x_wdg.c
src/qcc74x_flash.c
src/flash/qcc74x_sf_cfg.c
src/flash/qcc74x_xip_sflash.c
src/flash/qcc74x_sflash.c
src/flash/qcc74x_sf_ctrl.c
src/flash/qcc74x_flash_secreg_port.c
src/flash/qcc74x_flash_secreg.c
)

if("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
src/qcc74x_dac.c
src/qcc74x_ir.c
src/qcc74x_pwm_v1.c
src/qcc74x_sdio2.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
src/qcc74x_dac.c
src/qcc74x_emac.c
src/qcc74x_ir.c
src/qcc74x_pwm_v1.c
src/qcc74x_cam.c
src/qcc74x_spi_psram.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undefl")
sdk_library_add_sources(
src/qcc74x_kys.c
src/qcc74x_pwm_v1.c
src/qcc74x_pwm_v2.c
src/qcc74x_spi_psram.c
src/qcc74x_auadc.c
src/qcc74x_ir.c
)
elseif("${CHIP}" STREQUAL "qcc743")
sdk_library_add_sources(
src/qcc74x_dac.c
src/qcc74x_emac.c
src/qcc74x_ir.c
src/qcc74x_mjpeg.c
src/qcc74x_pwm_v2.c
src/qcc74x_cam.c
src/qcc74x_sdio2.c
src/qcc74x_i2s.c
src/qcc74x_dbi.c
src/qcc74x_audac.c
src/qcc74x_auadc.c
src/qcc74x_platform_dma.c
src/qcc74x_wo.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
src/qcc74x_dac.c
src/qcc74x_emac.c
src/qcc74x_clock.c
src/qcc74x_pwm_v2.c
src/qcc74x_wo.c
)
elseif("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(
src/qcc74x_dac.c
src/qcc74x_emac.c
src/qcc74x_ir.c
src/qcc74x_mjpeg.c
src/qcc74x_pwm_v2.c
src/qcc74x_cam.c
src/qcc74x_csi.c
src/qcc74x_i2s.c
src/qcc74x_wo.c
)
endif()

if(CONFIG_CHERRYUSB)
if("${CHIP}" STREQUAL "qcc74x_undef")
sdk_library_add_sources(src/qcc74x_usb_v1.c)
elseif(("${CHIP}" STREQUAL "qcc74x_undef") OR ("${CHIP}" STREQUAL "qcc74x_undefl"))
# no usb
elseif(("${CHIP}" STREQUAL "qcc74x_undef"))
else()
sdk_library_add_sources(src/qcc74x_usb_v2.c)
endif()
endif()

# optional
sdk_library_add_sources(src/qcc74x_irq.c)
sdk_library_add_sources(src/qcc74x_l1c.c)
sdk_library_add_sources(src/qcc74x_mtimer.c)

sdk_add_include_directories(include)
sdk_add_include_directories(include/arch)
sdk_add_include_directories(include/arch/risc-v/t-head)
sdk_add_include_directories(include/arch/risc-v/t-head/Core/Include)
sdk_add_include_directories(include/arch/risc-v/t-head/csi_dsp/include)
sdk_add_include_directories(config/${CHIP})
sdk_add_include_directories(src/flash)

if((NOT ("${CHIP}" STREQUAL "qcc74x_undef")) AND (NOT ("${CHIP}" STREQUAL "qcc74x_undef")) AND (NOT ("${CHIP}" STREQUAL "qcc74x_undefl")))
sdk_library_add_sources(include/arch/risc-v/t-head/rv_hart.c)
sdk_library_add_sources(include/arch/risc-v/t-head/rv_pmp.c)
if((NOT ("${CPU_ID}" STREQUAL "d0")) AND (NOT ("${CPU_ID}" STREQUAL "lp")))
sdk_add_static_library(include/arch/risc-v/t-head/csi_dsp/lib/libcsi_xt900p32f_dsp.a)
endif()
endif()

sdk_library_add_sources(config/${CHIP}/device_table.c)

string(TOUPPER ${CHIP} CHIPNAME)
sdk_add_compile_definitions(-D${CHIPNAME})

if(CPU_ID)
string(TOUPPER ${CPU_ID} CPU_ID_NAME)
sdk_add_compile_definitions(-DCPU_${CPU_ID_NAME})
endif()

#if((NOT ("${CPU_ID}" STREQUAL "d0")) AND (NOT ("${CPU_ID}" STREQUAL "lp")))
#sdk_add_static_library(src/pka/libpka.a)
#else()
add_subdirectory(src/pka)
#endif()

sdk_add_private_compile_definitions_ifdef(CONFIG_FLASH_2LINE -D${CONFIG_FLASH_2LINE})
