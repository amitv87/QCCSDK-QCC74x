#define DBG_TAG "TEST"

#include "qcc74xlog.h"

QCC74xLOG_DEFINE_TAG(TEST, DBG_TAG, true);
#undef QCC74xLOG_TAG
#define QCC74xLOG_TAG QCC74xLOG_GET_TAG(TEST)

extern qcc74xlog_t example_recorder;

void test_log(void)
{
    QCC74xLOG_F(&example_recorder, "hello test this is fatal error\r\n");
    QCC74xLOG_E(&example_recorder, "hello test this is error\r\n");
    QCC74xLOG_W(&example_recorder, "hello test this is warning\r\n");
    QCC74xLOG_I(&example_recorder, "hello test this is information\r\n");
    QCC74xLOG_D(&example_recorder, "hello test this is degug information\r\n");
    QCC74xLOG_T(&example_recorder, "hello test this is trace information\r\n");
}