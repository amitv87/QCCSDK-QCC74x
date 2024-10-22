sdk_generate_library()

sdk_library_add_sources(startup/vector.S)
sdk_library_add_sources(startup/riscv_fpu.S)
sdk_library_add_sources(startup/start_load.c)
sdk_library_add_sources(startup/system_qcc743.c)
sdk_library_add_sources(startup/interrupt.c)

if(CONFIG_ROMAPI)
sdk_library_add_sources(src/qcc743_romapi_e907.c)
sdk_library_add_sources(src/qcc743_romapi_patch.c)
sdk_add_compile_definitions(-DQCC74x_USE_ROM_DRIVER)
else()
sdk_library_add_sources(src/qcc743_glb_gpio.c)
sdk_library_add_sources(src/qcc743_glb.c)
sdk_library_add_sources(src/qcc743_hbn.c)
sdk_library_add_sources(src/qcc743_l1c.c)
sdk_library_add_sources(src/qcc743_pds.c)
sdk_library_add_sources(src/qcc743_aon.c)
endif()
sdk_library_add_sources(src/qcc743_clock.c)
sdk_library_add_sources(src/qcc743_common.c)
sdk_library_add_sources(src/qcc743_ef_cfg.c)
sdk_library_add_sources(src/qcc743_sdh.c)
sdk_library_add_sources(src/qcc743_sdu.c)
sdk_library_add_sources(src/qcc743_tzc_sec.c)
sdk_library_add_sources(src/qcc743_psram.c)
sdk_library_add_sources(src/qcc743_pm.c)
sdk_library_add_sources(src/qcc743_sec_dbg.c)

if(CONFIG_RF)
sdk_library_add_sources(src/qcc743_mfg_efuse.c)
sdk_library_add_sources(src/qcc743_mfg_flash.c)
sdk_library_add_sources(src/qcc743_mfg_media.c)
endif()

sdk_library_add_sources(port/qcc743_gpio.c)
sdk_library_add_sources(port/qcc743_clock.c)

sdk_add_include_directories(
include
include/hardware
)

SET(MCPU "e907")
SET(MARCH "rv32imafcpzpsfoperand_xtheade")
SET(MABI "ilp32f")

sdk_add_compile_definitions(-DARCH_RISCV -DQCC74x_USE_HAL_DRIVER)
sdk_add_compile_options(-march=${MARCH} -mabi=${MABI} -mtune=${MCPU})
sdk_add_link_options(-march=${MARCH} -mabi=${MABI} -mtune=${MCPU})

sdk_add_compile_definitions(-DCONFIG_IRQ_NUM=80)
