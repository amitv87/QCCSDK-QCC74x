/* newlib.h.  Generated from newlib.hin by configure.  */
/* newlib.hin.  Manually edited from the output of autoheader to
   remove all PACKAGE_ macros which will collide with any user
   package using newlib header files and having its own package name,
   version, etc...  */
#ifndef __NEWLIB_H__

#define __NEWLIB_H__ 1

/* EL/IX level */
/* #undef _ELIX_LEVEL */

/* Newlib version */
#include <_newlib_version.h>

/* C99 formats support (such as %a, %zu, ...) in IO functions like
 * printf/scanf enabled */
#define _WANT_IO_C99_FORMATS 1

/* long long type support in IO functions like printf/scanf enabled */
#define _WANT_IO_LONG_LONG 1

/* Register application finalization function using atexit. */
#define _WANT_REGISTER_FINI 1

/* long double type support in IO functions like printf/scanf enabled */
#define _WANT_IO_LONG_DOUBLE 1

/* Positional argument support in printf functions enabled.  */
/* #undef _WANT_IO_POS_ARGS */

/* Optional reentrant struct support.  Used mostly on platforms with
   very restricted storage.  */
/* #undef _WANT_REENT_SMALL */

/* Verify _REENT_CHECK macros allocate memory successfully. */
/* #undef _REENT_CHECK_VERIFY */

/* Multibyte supported */
/* #undef _MB_CAPABLE */

/* MB_LEN_MAX */
#define _MB_LEN_MAX 1

/* ICONV enabled */
#define _ICONV_ENABLED 1

/* Enable ICONV external CCS files loading capabilities */
/* #undef _ICONV_ENABLE_EXTERNAL_CCS */

/* Define if the linker supports .preinit_array/.init_array/.fini_array
 * sections.  */
#define HAVE_INITFINI_ARRAY 1

/* True if atexit() may dynamically allocate space for cleanup
   functions.  */
/* #undef _ATEXIT_DYNAMIC_ALLOC */

/* True if long double supported.  */
#define _HAVE_LONG_DOUBLE 1

/* Define if compiler supports -fno-tree-loop-distribute-patterns. */
#define _HAVE_CC_INHIBIT_LOOP_TO_LIBCALL 1

/* True if long double supported and it is equal to double.  */
/* #undef _LDBL_EQ_DBL */
 
/* Define if ivo supported in streamio.  */
#define _FVWRITE_IN_STREAMIO 1

/* Define if fseek functions support seek optimization.  */
#define _FSEEK_OPTIMIZATION 1

/* Define if wide char orientation is supported.  */
#define _WIDE_ORIENT 1

/* Define if unbuffered stream file optimization is supported.  */
#define _UNBUF_STREAM_OPT 1

/* Define if lite version of exit supported.  */
/* #undef _LITE_EXIT */

/* Define if declare atexit data as global.  */
/* #undef _REENT_GLOBAL_ATEXIT */

/* Define to move the stdio stream FILE objects out of struct _reent and make
   them global.  The stdio stream pointers of struct _reent are initialized to
   point to the global stdio FILE stream objects. */
/* #undef _WANT_REENT_GLOBAL_STDIO_STREAMS */

/* Define if small footprint nano-formatted-IO implementation used.  */
/* #undef _NANO_FORMATTED_IO */

/* Define if using retargetable functions for default lock routines.  */
#define _RETARGETABLE_LOCKING 1

/* Define to use type long for time_t.  */
/* #undef _WANT_USE_LONG_TIME_T */

/*
 * Iconv encodings enabled ("to" direction)
 */
#define _ICONV_TO_ENCODING_BIG5 1
#define _ICONV_TO_ENCODING_CP775 1
#define _ICONV_TO_ENCODING_CP850 1
#define _ICONV_TO_ENCODING_CP852 1
#define _ICONV_TO_ENCODING_CP855 1
#define _ICONV_TO_ENCODING_CP866 1
#define _ICONV_TO_ENCODING_EUC_JP 1
#define _ICONV_TO_ENCODING_EUC_TW 1
#define _ICONV_TO_ENCODING_EUC_KR 1
#define _ICONV_TO_ENCODING_ISO_8859_1 1
#define _ICONV_TO_ENCODING_ISO_8859_10 1
#define _ICONV_TO_ENCODING_ISO_8859_11 1
#define _ICONV_TO_ENCODING_ISO_8859_13 1
#define _ICONV_TO_ENCODING_ISO_8859_14 1
#define _ICONV_TO_ENCODING_ISO_8859_15 1
#define _ICONV_TO_ENCODING_ISO_8859_2 1
#define _ICONV_TO_ENCODING_ISO_8859_3 1
#define _ICONV_TO_ENCODING_ISO_8859_4 1
#define _ICONV_TO_ENCODING_ISO_8859_5 1
#define _ICONV_TO_ENCODING_ISO_8859_6 1
#define _ICONV_TO_ENCODING_ISO_8859_7 1
#define _ICONV_TO_ENCODING_ISO_8859_8 1
#define _ICONV_TO_ENCODING_ISO_8859_9 1
#define _ICONV_TO_ENCODING_ISO_IR_111 1
#define _ICONV_TO_ENCODING_KOI8_R 1
#define _ICONV_TO_ENCODING_KOI8_RU 1
#define _ICONV_TO_ENCODING_KOI8_U 1
#define _ICONV_TO_ENCODING_KOI8_UNI 1
#define _ICONV_TO_ENCODING_UCS_2 1
#define _ICONV_TO_ENCODING_UCS_2_INTERNAL 1
#define _ICONV_TO_ENCODING_UCS_2BE 1
#define _ICONV_TO_ENCODING_UCS_2LE 1
#define _ICONV_TO_ENCODING_UCS_4 1
#define _ICONV_TO_ENCODING_UCS_4_INTERNAL 1
#define _ICONV_TO_ENCODING_UCS_4BE 1
#define _ICONV_TO_ENCODING_UCS_4LE 1
#define _ICONV_TO_ENCODING_US_ASCII 1
#define _ICONV_TO_ENCODING_UTF_16 1
#define _ICONV_TO_ENCODING_UTF_16BE 1
#define _ICONV_TO_ENCODING_UTF_16LE 1
#define _ICONV_TO_ENCODING_UTF_8 1
#define _ICONV_TO_ENCODING_WIN_1250 1
#define _ICONV_TO_ENCODING_WIN_1251 1
#define _ICONV_TO_ENCODING_WIN_1252 1
#define _ICONV_TO_ENCODING_WIN_1253 1
#define _ICONV_TO_ENCODING_WIN_1254 1
#define _ICONV_TO_ENCODING_WIN_1255 1
#define _ICONV_TO_ENCODING_WIN_1256 1
#define _ICONV_TO_ENCODING_WIN_1257 1
#define _ICONV_TO_ENCODING_WIN_1258 1

/*
 * Iconv encodings enabled ("from" direction)
 */
#define _ICONV_FROM_ENCODING_BIG5 1
#define _ICONV_FROM_ENCODING_CP775 1
#define _ICONV_FROM_ENCODING_CP850 1
#define _ICONV_FROM_ENCODING_CP852 1
#define _ICONV_FROM_ENCODING_CP855 1
#define _ICONV_FROM_ENCODING_CP866 1
#define _ICONV_FROM_ENCODING_EUC_JP 1
#define _ICONV_FROM_ENCODING_EUC_TW 1
#define _ICONV_FROM_ENCODING_EUC_KR 1
#define _ICONV_FROM_ENCODING_ISO_8859_1 1
#define _ICONV_FROM_ENCODING_ISO_8859_10 1
#define _ICONV_FROM_ENCODING_ISO_8859_11 1
#define _ICONV_FROM_ENCODING_ISO_8859_13 1
#define _ICONV_FROM_ENCODING_ISO_8859_14 1
#define _ICONV_FROM_ENCODING_ISO_8859_15 1
#define _ICONV_FROM_ENCODING_ISO_8859_2 1
#define _ICONV_FROM_ENCODING_ISO_8859_3 1
#define _ICONV_FROM_ENCODING_ISO_8859_4 1
#define _ICONV_FROM_ENCODING_ISO_8859_5 1
#define _ICONV_FROM_ENCODING_ISO_8859_6 1
#define _ICONV_FROM_ENCODING_ISO_8859_7 1
#define _ICONV_FROM_ENCODING_ISO_8859_8 1
#define _ICONV_FROM_ENCODING_ISO_8859_9 1
#define _ICONV_FROM_ENCODING_ISO_IR_111 1
#define _ICONV_FROM_ENCODING_KOI8_R 1
#define _ICONV_FROM_ENCODING_KOI8_RU 1
#define _ICONV_FROM_ENCODING_KOI8_U 1
#define _ICONV_FROM_ENCODING_KOI8_UNI 1
#define _ICONV_FROM_ENCODING_UCS_2 1
#define _ICONV_FROM_ENCODING_UCS_2_INTERNAL 1
#define _ICONV_FROM_ENCODING_UCS_2BE 1
#define _ICONV_FROM_ENCODING_UCS_2LE 1
#define _ICONV_FROM_ENCODING_UCS_4 1
#define _ICONV_FROM_ENCODING_UCS_4_INTERNAL 1
#define _ICONV_FROM_ENCODING_UCS_4BE 1
#define _ICONV_FROM_ENCODING_UCS_4LE 1
#define _ICONV_FROM_ENCODING_US_ASCII 1
#define _ICONV_FROM_ENCODING_UTF_16 1
#define _ICONV_FROM_ENCODING_UTF_16BE 1
#define _ICONV_FROM_ENCODING_UTF_16LE 1
#define _ICONV_FROM_ENCODING_UTF_8 1
#define _ICONV_FROM_ENCODING_WIN_1250 1
#define _ICONV_FROM_ENCODING_WIN_1251 1
#define _ICONV_FROM_ENCODING_WIN_1252 1
#define _ICONV_FROM_ENCODING_WIN_1253 1
#define _ICONV_FROM_ENCODING_WIN_1254 1
#define _ICONV_FROM_ENCODING_WIN_1255 1
#define _ICONV_FROM_ENCODING_WIN_1256 1
#define _ICONV_FROM_ENCODING_WIN_1257 1
#define _ICONV_FROM_ENCODING_WIN_1258 1

#endif /* !__NEWLIB_H__ */

