// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t qcc74x_err_t;

/* Definitions for error constants. */
#define QCC74x_OK          0       /*!< qcc74x_err_t value indicating success (no error) */
#define QCC74x_FAIL        -1      /*!< Generic qcc74x_err_t code indicating failure */

#define QCC74x_ERR_NO_MEM              0x101   /*!< Out of memory */
#define QCC74x_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
#define QCC74x_ERR_INVALID_STATE       0x103   /*!< Invalid state */
#define QCC74x_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
#define QCC74x_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
#define QCC74x_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
#define QCC74x_ERR_TIMEOUT             0x107   /*!< Operation timed out */
#define QCC74x_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
#define QCC74x_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
#define QCC74x_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
#define QCC74x_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */

#define QCC74x_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
#define QCC74x_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
#define QCC74x_ERR_FLASH_BASE          0x6000  /*!< Starting number of flash error codes */

/**
  * @brief Returns string for qcc74x_err_t error codes
  *
  * This function finds the error code in a pre-generated lookup-table and
  * returns its string representation.
  *
  * The function is generated by the Python script
  * tools/gen_qcc74x_err_to_name.py which should be run each time an qcc74x_err_t
  * error is modified, created or removed from the IDF project.
  *
  * @param code qcc74x_err_t error code
  * @return string error message
  */
const char *qcc74x_err_to_name(qcc74x_err_t code);

/**
  * @brief Returns string for qcc74x_err_t and system error codes
  *
  * This function finds the error code in a pre-generated lookup-table of
  * qcc74x_err_t errors and returns its string representation. If the error code
  * is not found then it is attempted to be found among system errors.
  *
  * The function is generated by the Python script
  * tools/gen_qcc74x_err_to_name.py which should be run each time an qcc74x_err_t
  * error is modified, created or removed from the IDF project.
  *
  * @param code qcc74x_err_t error code
  * @param[out] buf buffer where the error message should be written
  * @param buflen Size of buffer buf. At most buflen bytes are written into the buf buffer (including the terminating null byte).
  * @return buf containing the string error message
  */
const char *qcc74x_err_to_name_r(qcc74x_err_t code, char *buf, size_t buflen);

/** @cond */
void _qcc74x_error_check_failed(qcc74x_err_t rc, const char *file, int line, const char *function, const char *expression) __attribute__((noreturn));

/** @cond */
void _qcc74x_error_check_failed_without_abort(qcc74x_err_t rc, const char *file, int line, const char *function, const char *expression);

#ifndef __ASSERT_FUNC
/* This won't happen on IDF, which defines __ASSERT_FUNC in assert.h, but it does happen when building on the host which
   uses /usr/include/assert.h or equivalent.
*/
#ifdef __ASSERT_FUNCTION
#define __ASSERT_FUNC __ASSERT_FUNCTION /* used in glibc assert.h */
#else
#define __ASSERT_FUNC "??"
#endif
#endif
/** @endcond */

/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not QCC74x_OK.
 * Prints the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define QCC74x_ERROR_CHECK(x) do {                                         \
        qcc74x_err_t __err_rc = (x);                                       \
        (void) sizeof(__err_rc);                                        \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define QCC74x_ERROR_CHECK(x) do {                                         \
        qcc74x_err_t __err_rc = (x);                                       \
        if (__err_rc != QCC74x_OK) {                                       \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define QCC74x_ERROR_CHECK(x) do {                                         \
        qcc74x_err_t __err_rc = (x);                                       \
        if (__err_rc != QCC74x_OK) {                                       \
            _qcc74x_error_check_failed(__err_rc, __FILE__, __LINE__,       \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif

/**
 * Macro which can be used to check the error code. Prints the error code, error location, and the failed statement to
 * serial output.
 * In comparison with QCC74x_ERROR_CHECK(), this prints the same error message but isn't terminating the program.
 */
#ifdef NDEBUG
#define QCC74x_ERROR_CHECK_WITHOUT_ABORT(x) ({                                         \
        qcc74x_err_t __err_rc = (x);                                                   \
        __err_rc;                                                                   \
    })
#else
#define QCC74x_ERROR_CHECK_WITHOUT_ABORT(x) ({                                         \
        qcc74x_err_t __err_rc = (x);                                                   \
        if (__err_rc != QCC74x_OK) {                                                   \
            _qcc74x_error_check_failed_without_abort(__err_rc, __FILE__, __LINE__,     \
                                    __ASSERT_FUNC, #x);                             \
        }                                                                           \
        __err_rc;                                                                   \
    })
#endif //NDEBUG

#ifdef __cplusplus
}
#endif
