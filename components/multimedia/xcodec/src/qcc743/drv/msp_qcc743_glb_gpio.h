
#ifndef __MSP_QCC743_GLB_GPIO_H__
#define __MSP_QCC743_GLB_GPIO_H__

#include "msp_qcc743_platform.h"
#include "msp_qcc743_gpio.h"

QCC74x_Err_Type ms_GLB_GPIO_Init(GLB_GPIO_Cfg_Type *cfg);
QCC74x_Err_Type msp_GLB_GPIO_Write(GLB_GPIO_Type gpioPin, uint32_t val);
#endif

