sdk_generate_library()

if("${CHIP}" STREQUAL "qcc743")
  sdk_add_private_compile_definitions(-DQCC74x_QCC743)
  sdk_add_private_compile_definitions(-DQCC743)
endif()

sdk_add_include_directories(./include)

sdk_library_add_sources(./ot_settings_easyflash.c)
sdk_library_add_sources(./ot_sys_qcc74x.c)
sdk_library_add_sources(./ot_alarm_qcc743.c)
sdk_library_add_sources(./ot_freertos.c)
sdk_library_add_sources(./ot_radio.c)

if (NOT DEFINED OT_NCP)
  sdk_library_add_sources(./ot_logging.c)
endif()

if ("${OT_SERIAL}" STREQUAL "SYS_CLI")
  sdk_library_add_sources(./ot_shell.c)
elseif ("${OT_SERIAL}" STREQUAL "UART") 
  sdk_library_add_sources(./ot_uart_qcc74x.c)
  if ($(OT_SERIAL_BUFFER_SIZE)) 
    sdk_add_private_compile_definitions(-DOT_SERIAL_BUFFER_SIZE=${OT_SERIAL_BUFFER_SIZE})
  endif()
  if ($(OT_SERIAL_LINE_SIZE)) 
    sdk_add_private_compile_definitions(-DOT_SERIAL_LINE_SIZE=${OT_SERIAL_LINE_SIZE})
  endif()
else ()
  message (FATAL_ERROR "openthread_port: ${OT_SERIAL} is not available for OT_SERIAL.")
endif()

# disable undef warning
# sdk_add_compile_options(-Wno-undef)