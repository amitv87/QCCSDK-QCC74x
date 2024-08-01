sdk_add_include_directories(.)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/board.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fw_header.c)

if(CONFIG_RF)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/board_rf.c)
endif()

if(CONFIG_PSRAM_COPY_CODE)
sdk_set_linker_script(qcc743_chip_psram.ld)
elseif(CONFIG_LINK_FOR_MFGTEST)
sdk_set_linker_script(qcc743_chip_mfgtest.ld)
elseif(CONFIG_LPAPP)
sdk_set_linker_script(qcc743_lp.ld)
else()
sdk_set_linker_script(qcc743_flash.ld)
endif()

# qcc744dk have psram, so define CONFIG_PSRAM for some code to determine this.
sdk_add_compile_definitions(-DCONFIG_PSRAM)

if(CONFIG_CONSOLE_WO)
sdk_add_compile_definitions(-DCONFIG_CONSOLE_WO)
endif()

sdk_add_compile_definitions_ifdef(CONFIG_PSRAM_COPY_CODE -DCONFIG_PSRAM_COPY_CODE)

sdk_add_link_options(-ufw_header)

sdk_add_link_options(-Wl,--build-id)
