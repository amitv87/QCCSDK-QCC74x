#ifndef __QCC743_COMMON_H__
#define __QCC743_COMMON_H__

#include "qcc743.h"
#include "qcc74x_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  COMMON
 *  @{
 */

/**
 * @brief Memory access macro
 */
#define QCC74x_RD_WORD(addr)       (*((volatile uint32_t *)(uintptr_t)(addr)))
#define QCC74x_WR_WORD(addr, val)  ((*(volatile uint32_t *)(uintptr_t)(addr)) = (val))
#define QCC74x_RD_SHORT(addr)      (*((volatile uint16_t *)(uintptr_t)(addr)))
#define QCC74x_WR_SHORT(addr, val) ((*(volatile uint16_t *)(uintptr_t)(addr)) = (val))
#define QCC74x_RD_BYTE(addr)       (*((volatile uint8_t *)(uintptr_t)(addr)))
#define QCC74x_WR_BYTE(addr, val)  ((*(volatile uint8_t *)(uintptr_t)(addr)) = (val))
#define QCC74x_RDWD_FRM_BYTEP(p)   ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | (p[0]))

#define QCC74x_WRWD_TO_BYTEP(p, val)   \
    {                              \
        p[0] = val & 0xff;         \
        p[1] = (val >> 8) & 0xff;  \
        p[2] = (val >> 16) & 0xff; \
        p[3] = (val >> 24) & 0xff; \
    }
/**
 * @brief Register access macro
 */
#define QCC74x_RD_REG16(addr, regname)                QCC74x_RD_SHORT(addr + regname##_OFFSET)
#define QCC74x_WR_REG16(addr, regname, val)           QCC74x_WR_SHORT(addr + regname##_OFFSET, val)
#define QCC74x_RD_REG(addr, regname)                  QCC74x_RD_WORD(addr + regname##_OFFSET)
#define QCC74x_WR_REG(addr, regname, val)             QCC74x_WR_WORD(addr + regname##_OFFSET, val)
#define QCC74x_SET_REG_BIT(val, bitname)              ((val) | (1U << bitname##_POS))
#define QCC74x_CLR_REG_BIT(val, bitname)              ((val)&bitname##_UMSK)
#define QCC74x_GET_REG_BITS_VAL(val, bitname)         (((val)&bitname##_MSK) >> bitname##_POS)
#define QCC74x_SET_REG_BITS_VAL(val, bitname, bitval) (((val)&bitname##_UMSK) | ((uint32_t)(bitval) << bitname##_POS))
#define QCC74x_IS_REG_BIT_SET(val, bitname)           (((val) & (1U << (bitname##_POS))) != 0)
#define QCC74x_DRV_DUMMY           \
    {                          \
        __ASM volatile("nop"); \
        __ASM volatile("nop"); \
        __ASM volatile("nop"); \
        __ASM volatile("nop"); \
    }

/** @defgroup  COMMON_Public_Types
 *  @{
 */
#ifdef BIT
#undef BIT
#define BIT(n) (1UL << (n))
#else
#define BIT(n) (1UL << (n))
#endif

/**
 * @brief Null Type definition
 */
#ifndef NULL
#define NULL 0
#endif

/**
 * @brief Error type definition
 */
typedef enum {
    SUCCESS = 0,
    ERROR = 1,
    TIMEOUT = 2,
    INVALID = 3, /* invalid arguments */
    NORESC = 4   /* no resource or resource temperary unavailable */
} QCC74x_Err_Type;

/**
 * @brief Functional type definition
 */
typedef enum {
    DISABLE = 0,
    ENABLE = 1,
} QCC74x_Fun_Type;

/**
 * @brief Status type definition
 */
typedef enum {
    RESET = 0,
    SET = 1,
} QCC74x_Sts_Type;

/**
 * @brief Mask type definition
 */
typedef enum {
    UNMASK = 0,
    MASK = 1
} QCC74x_Mask_Type;

/**
 * @brief Logical status Type definition
 */
typedef enum {
    LOGIC_LO = 0,
    LOGIC_HI = !LOGIC_LO
} LogicalStatus;

/**
 * @brief Active status Type definition
 */
typedef enum {
    DEACTIVE = 0,
    ACTIVE = !DEACTIVE
} ActiveStatus;

/**
 *  @brief Interrupt callback function type
 */
typedef void(intCallback_Type)(void);
typedef void (*pFunc)(void);

#ifdef DEBUG
void check_failed(uint8_t *file, uint32_t line);
#define CHECK_PARAM(expr) ((expr) ? (void)0 : check_failed((uint8_t *)__FILE__, __LINE__))
#else
#define CHECK_PARAM(expr) ((void)0)
#endif /* DEBUG */

/*@} end of group COMMON_Public_Types */

/** @defgroup  COMMON_Public_Constants
 *  @{
 */

/** @defgroup DRIVER_INT_PERIPH
 *  @{
 */
#define IS_INT_PERIPH(INT_PERIPH) ((INT_PERIPH) < IRQn_LAST)

/*@} end of group DRIVER_INT_PERIPH */

/** @defgroup DRIVER_INT_MASK
 *  @{
 */
#define IS_QCC74x_MASK_TYPE(type) (((type) == MASK) || ((type) == UNMASK))

#define ARCH_MemCpy      arch_memcpy
#define ARCH_MemSet      arch_memset
#define ARCH_MemCmp      arch_memcmp
#define ARCH_MemCpy4     arch_memcpy4
#define arch_memcpy_fast arch_memcpy_fast
#define ARCH_MemSet4     arch_memset4
#define QCC74x_Soft_CRC32  qcc74x_soft_crc32
#define CPU_Interrupt_Enable(irq)
#define CPU_Interrupt_Disable(irq)
#define Interrupt_Handler_Register(irq, callback)
/*@} end of group COMMON_Public_Constants */

/** @defgroup DRIVER_Public_FunctionDeclaration
 *  @brief DRIVER functions declaration
 *  @{
 */

// void Interrupt_Handler_Register(IRQn_Type irq, pFunc interruptFun);
void ASM_Delay_Us(uint32_t core, uint32_t cnt, uint32_t loopT);
void arch_delay_us(uint32_t cnt);
void arch_delay_ms(uint32_t cnt);

void *ARCH_MemCpy(void *dst, const void *src, uint32_t n);
uint32_t *ARCH_MemCpy4(uint32_t *dst, const uint32_t *src, uint32_t n);
void *arch_memcpy_fast(void *pdst, const void *psrc, uint32_t n);
void *ARCH_MemSet(void *s, uint8_t c, uint32_t n);
uint32_t *ARCH_MemSet4(uint32_t *dst, const uint32_t val, uint32_t n);
int ARCH_MemCmp(const void *s1, const void *s2, uint32_t n);

void C906_All_Int_Enable(void);
void C906_All_Int_Disable(void);

/*@} end of group DRIVER_COMMON  */

#ifdef __cplusplus
}
#endif

#endif /* __QCC743_COMMON_H__ */
