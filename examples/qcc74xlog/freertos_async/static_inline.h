#include "example_log.h"

#ifndef __STATIC_INLINE_H__
#define __STATIC_INLINE_H__

/*!< 只能在不被include的c文件中定义tag标签 */
/*!< 头文件中可以使用extern将标签导出 */
QCC74xLOG_EXTERN_TAG(INLINE);

/*!< 设置此文件使用的tag标签 */
#undef QCC74xLOG_TAG
#define QCC74xLOG_TAG QCC74xLOG_GET_TAG(INLINE)

static inline void test_log_static_inline(void)
{
    LOG_I("static inline\r\n");
}

#endif