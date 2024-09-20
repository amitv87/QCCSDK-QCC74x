sdk_generate_library()

if("${CHIP}" STREQUAL "qcc743")
  sdk_add_private_compile_definitions(-DQCC743)
endif()

sdk_add_include_directories(./include)
sdk_add_private_include_directories(./)
sdk_library_add_sources(./openthread_httpd.c)
sdk_library_add_sources(./openthread_rest.c)
