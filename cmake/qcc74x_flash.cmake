if("${CMAKE_SYSTEM_NAME}" STREQUAL "Generic")
set(TOOL_SUFFIX ".exe")
set(CMAKE ${QCC74x_SDK_BASE}/tools/cmake/bin/cmake.exe)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
set(TOOL_SUFFIX "-ubuntu")
set(CMAKE cmake)
endif()

set(QCC74x_FW_POST_PROC ${QCC74x_SDK_BASE}/tools/qcc74x_tools/QConn_Secure/QConn_Secure${TOOL_SUFFIX})

set(QCC74x_FW_POST_PROC_CONFIG --chipname=${CHIP} --imgfile=${BIN_FILE})

if(BOARD_DIR)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --brdcfgdir=${BOARD_DIR}/${BOARD}/config)
elseif(CONFIG_BOARD_CONFIG_8M)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --brdcfgdir=${QCC74x_SDK_BASE}/bsp/board/${BOARD}/${CONFIG_BOARD_CONFIG_8M})
else()
list(APPEND QCC74x_FW_POST_PROC_CONFIG --brdcfgdir=${QCC74x_SDK_BASE}/bsp/board/${BOARD}/config)
endif()

if(CONFIG_AES_KEY)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --key=${CONFIG_AES_KEY})
endif()

if(CONFIG_AES_IV)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --iv=${CONFIG_AES_IV})
endif()

if(CONFIG_PUBLIC_KEY)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --publickey=${CONFIG_PUBLIC_KEY})
endif()

if(CONFIG_PRIVATE_KEY)
list(APPEND QCC74x_FW_POST_PROC_CONFIG --privatekey=${CONFIG_PRIVATE_KEY})
endif()

if(CONFIG_FW_POST_PROC_CUSTOM)
list(APPEND QCC74x_FW_POST_PROC_CONFIG ${CONFIG_FW_POST_PROC_CUSTOM})
endif()

add_custom_target(combine
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${QCC74x_FW_POST_PROC} ${QCC74x_FW_POST_PROC_CONFIG})

if("${CONFIG_POST_BUILD}" STREQUAL "RELEASE_MFG")
file(GLOB OLD_MFG_BIN "${QCC74x_SDK_BASE}/bsp/board/${BOARD}/config/mfg*.bin")
file(GLOB DTS_FILES "${QCC74x_SDK_BASE}/bsp/board/${BOARD}/config/*.dts")
file(GLOB BIN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/build/build_out/*.bin")
file(GLOB INI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.ini")

add_custom_target(post_build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE} -E remove ${OLD_MFG_BIN}
        COMMAND ${CMAKE} -E copy ${BIN_FILE} ${QCC74x_SDK_BASE}/bsp/board/${BOARD}/config/
        COMMAND ${CMAKE} -E remove_directory mfg_release
        COMMAND ${CMAKE} -E copy_directory ${QCC74x_SDK_BASE}/tools/qcc74x_tools mfg_release
        COMMAND ${CMAKE} -E copy ${BIN_FILES} ${DTS_FILES} ${INI_FILES} mfg_release)
elseif("${CONFIG_POST_BUILD}" STREQUAL "CONCAT_WITH_LP_FW")
add_custom_target(post_build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${QCC74x_SDK_BASE}/tools/lpfw/patch_lpfw${TOOL_SUFFIX} ${BIN_FILE} ${QCC74x_SDK_BASE}/tools/lpfw/bin/${CHIP}_lp_fw.bin)
else()
add_custom_target(post_build)
endif()
