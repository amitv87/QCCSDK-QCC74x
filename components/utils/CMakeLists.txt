sdk_generate_library()

if(CONFIG_LOG_DISABLE)
    sdk_add_compile_definitions(-DCONFIG_LOG_DISABLE)
endif()

if(CONFIG_ASSERT_DISABLE)
    sdk_add_compile_definitions(-DCONFIG_ASSERT_DISABLE)
endif()

if(DEFINED CONFIG_LOG_LEVEL)
    sdk_add_compile_definitions(-DCONFIG_LOG_LEVEL=${CONFIG_LOG_LEVEL})
else()
    sdk_add_compile_definitions(-DCONFIG_LOG_LEVEL=3)
endif()

if(CONFIG_LOG_POOL_SIZE)
    sdk_add_compile_definitions(-DCONFIG_LOG_POOL_SIZE=${CONFIG_LOG_POOL_SIZE})
endif()


# log
if(CONFIG_LOG_NCOLOR)
    sdk_add_compile_definitions(-DCONFIG_LOG_NCOLOR=1)
endif()

sdk_library_add_sources(log/log.c)
sdk_add_include_directories(log)

if(CONFIG_QCC74xLOG)
    sdk_add_compile_definitions(-DCONFIG_QCC74xLOG)
    sdk_library_add_sources(log/qcc74xlog/qcc74xlog.c)
    sdk_add_include_directories(log/qcc74xlog)
    if(CONFIG_FREERTOS)
        sdk_library_add_sources(log/log_freertos.c)
    endif()
    if(CONFIG_QCC74xLOG_DEBUG)
    sdk_add_private_compile_definitions(-DCONFIG_QCC74xLOG_DEBUG)
    endif()
endif()

# ring_buffer
sdk_library_add_sources(ring_buffer/ring_buffer.c)
sdk_add_include_directories(ring_buffer)

# qcc74x block pool debug enable
if (CONFIG_QCC74x_BLOCK_POOL_DEBUG)
    sdk_add_compile_definitions(-DCONFIG_QCC74x_BLOCK_POOL_DEBUG)
endif()

# qcc74x block pool mutex
if (CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT)
    sdk_add_compile_definitions(-DCONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT=${CONFIG_QCC74x_BLOCK_POOL_MTX_TIMEOUT})
endif()

# qcc74x block pool
sdk_library_add_sources(qcc74x_block_pool/qcc74x_block_pool.c)
sdk_add_include_directories(qcc74x_block_pool)

if(DEFINED CONFIG_TIMEZONE)
    sdk_add_compile_definitions(-DQCC74x_TIMESTAMP_TIMEZONE=${CONFIG_TIMEZONE})
else()
    sdk_add_compile_definitions(-DQCC74x_TIMESTAMP_TIMEZONE=8)
endif()

# timestamp
sdk_library_add_sources(qcc74x_timestamp/qcc74x_timestamp.c)
sdk_add_include_directories(qcc74x_timestamp)

# getopt
sdk_library_add_sources(getopt/utils_getopt.c)
sdk_add_include_directories(getopt)

if(CONFIG_COREDUMP_V2)
sdk_add_compile_definitions(-DCONFIG_COREDUMP)
sdk_library_add_sources(coredump/coredump.c)
sdk_library_add_sources(coredump/encoder_v2.c)
sdk_library_add_sources(coredump/bin_fmt_v2.c)
sdk_add_include_directories(coredump)
elseif(CONFIG_COREDUMP)
sdk_add_compile_definitions(-DCONFIG_COREDUMP)
sdk_library_add_sources(coredump/coredump.c)
sdk_library_add_sources(coredump/encoder_v1.c)
sdk_library_add_sources(coredump/bin_fmt_v1.c)
sdk_add_include_directories(coredump)
endif()

# partition
if(CONFIG_PARTITION)
sdk_library_add_sources(partition/partition.c)
sdk_add_include_directories(partition)
endif()

# qcc74x_mtd
if(CONFIG_QCC74x_MTD)
sdk_library_add_sources(qcc74x_mtd/qcc74x_mtd.c qcc74x_mtd/qcc74x_boot2.c)
sdk_add_include_directories(qcc74x_mtd/include)
endif()

#cjson
sdk_library_add_sources(cjson/cJSON.c)
sdk_add_include_directories(cjson)

# math
sdk_library_add_sources(math/src/utils_base64.c math/src/utils_sha256.c math/src/utils_crc.c math/src/utils_hex.c)
sdk_add_include_directories(math/include)

# ota
if(CONFIG_QCC74x_OTA)
sdk_library_add_sources(qcc74x_ota/qcc74x_ota.c)
sdk_add_include_directories(qcc74x_ota)
endif()

# list
sdk_library_add_sources(list/src/utils_list.c)
sdk_add_include_directories(list/include)

