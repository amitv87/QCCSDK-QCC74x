/*
 * wpa_supplicant/hostapd - Default include files
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This header file is included into all C files so that commonly used header
 * files can be selected with OS specific ifdef blocks in one place instead of
 * having to have OS/C library specific selection in many files.
 */

#ifndef INCLUDES_H
#define INCLUDES_H

/* Include possible build time configuration before including anything else */
#include "build_config.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef _WIN32_WCE
#include <sys/types.h>
#include <errno.h>
#endif /* _WIN32_WCE */
#include <ctype.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif /* _MSC_VER */


#ifdef CONFIG_RWNX
#include "rtos_al.h"
//#ifdef CFG_HSU
//#include "hsu.h"
//#endif
//#include "dbg.h"
uint32_t dbg_vsnprintf_offset(char *buffer, uint32_t size, uint32_t offset, const char *fmt, va_list args);
#define dbg_vsnprintf(buffer, size, fmt, args) dbg_vsnprintf_offset(buffer, size, 0, fmt, args)
#include "export/dbg/trace_compo.h"
#include "export/dbg/dbg_assert.h"
#define vsnprintf dbg_vsnprintf
#define D_CRT       "\x9A"   ///< Prefix for critical
#define D_ERR       "\x9B"   ///< Prefix for error
#define D_WRN       "\x9C"   ///< Prefix for warning
#define D_INF       "\x9D"   ///< Prefix for info
#define D_VRB       "\x9E"   ///< Prefix for verbose debug
#define D_PHY       "\x87"   ///< Prefix for Modem / RF
#define dbg(fmt, ...)   printf(fmt, ## __VA_ARGS__)
#include "export/phy.h"
#include "export/export_macsw.h"
#endif /* CONFIG_RWNX */

#ifdef CONFIG_LWIP

#include <sys/socket.h>

#else /* ! CONFIG_LWIP */
#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef __vxworks
#include <sys/uio.h>
#include <sys/time.h>
#endif /* __vxworks */
#endif /* CONFIG_NATIVE_WINDOWS */
#endif /* CONFIG_LWIP */

#endif /* INCLUDES_H */
