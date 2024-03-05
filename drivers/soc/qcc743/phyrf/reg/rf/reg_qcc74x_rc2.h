#ifndef _REG_QCC74x_RC2_H_
#define _REG_QCC74x_RC2_H_
// Date: 20201225

#include "co_int.h"
#include "compiler.h"
#include "arch.h"
#include "reg_access.h"


#ifndef QCC74x_RC2_BASE
#define QCC74x_RC2_BASE (0x24c0c000)
#endif



#define RX0_VGA_CFG_ADDR (QCC74x_RC2_BASE + 0x00000040)

static inline uint32_t rx0_vga_cfg_get(void)
{
    return REG_PL_RD(RX0_VGA_CFG_ADDR);
}

static inline void rx0_vga_cfg_set(uint32_t x)
{
    REG_PL_WR(RX0_VGA_CFG_ADDR,x);
}

static inline uint32_t rc2_rx0_vga_approx_mode_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x80000000)) >> 31);
}

static inline void rc2_rx0_vga_approx_mode_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    localVal &= ~((uint32_t)0x80000000);
    localVal |= (x << 31)&((uint32_t)0x80000000);
    REG_PL_WR(RX0_VGA_CFG_ADDR,localVal);
}

static inline uint32_t rc2_rx0_vga_gain_min_db_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x7e000000)) >> 25);
}

static inline void rc2_rx0_vga_gain_min_db_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    localVal &= ~((uint32_t)0x7e000000);
    localVal |= (x << 25)&((uint32_t)0x7e000000);
    REG_PL_WR(RX0_VGA_CFG_ADDR,localVal);
}

static inline uint32_t rc2_rx0_vga_idx_max_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x01f00000)) >> 20);
}

static inline void rc2_rx0_vga_idx_max_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    localVal &= ~((uint32_t)0x01f00000);
    localVal |= (x << 20)&((uint32_t)0x01f00000);
    REG_PL_WR(RX0_VGA_CFG_ADDR,localVal);
}

static inline uint32_t rc2_rx0_vga_idx_min_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x000f8000)) >> 15);
}

static inline void rc2_rx0_vga_idx_min_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_VGA_CFG_ADDR);
    localVal &= ~((uint32_t)0x000f8000);
    localVal |= (x << 15)&((uint32_t)0x000f8000);
    REG_PL_WR(RX0_VGA_CFG_ADDR,localVal);
}


#define RX0_LNA_CFG_ADDR (QCC74x_RC2_BASE + 0x00000044)

static inline uint32_t rx0_lna_cfg_get(void)
{
    return REG_PL_RD(RX0_LNA_CFG_ADDR);
}

static inline void rx0_lna_cfg_set(uint32_t x)
{
    REG_PL_WR(RX0_LNA_CFG_ADDR,x);
}

static inline uint32_t rc2_rx0_lna_idx_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline uint32_t rc2_rx0_lna_idx_max_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_rx0_lna_idx_max_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_CFG_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(RX0_LNA_CFG_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_idx_min_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_CFG_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_rx0_lna_idx_min_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_CFG_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(RX0_LNA_CFG_ADDR,localVal);
}


#define RX0_LNA_GAIN_TABLE_0_ADDR (QCC74x_RC2_BASE + 0x00000080)

static inline uint32_t rx0_lna_gain_table_0_get(void)
{
    return REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
}

static inline void rx0_lna_gain_table_0_set(uint32_t x)
{
    REG_PL_WR(RX0_LNA_GAIN_TABLE_0_ADDR,x);
}

static inline uint32_t rc2_rx0_lna_gain_3_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline void rc2_rx0_lna_gain_3_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0xff000000);
    localVal |= (x << 24)&((uint32_t)0xff000000);
    REG_PL_WR(RX0_LNA_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_2_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_rx0_lna_gain_2_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(RX0_LNA_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_rx0_lna_gain_1_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(RX0_LNA_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_rx0_lna_gain_0_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(RX0_LNA_GAIN_TABLE_0_ADDR,localVal);
}


#define RX0_LNA_GAIN_TALE_1_ADDR (QCC74x_RC2_BASE + 0x00000084)

static inline uint32_t rx0_lna_gain_tale_1_get(void)
{
    return REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
}

static inline void rx0_lna_gain_tale_1_set(uint32_t x)
{
    REG_PL_WR(RX0_LNA_GAIN_TALE_1_ADDR,x);
}

static inline uint32_t rc2_rx0_lna_gain_7_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline void rc2_rx0_lna_gain_7_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    localVal &= ~((uint32_t)0xff000000);
    localVal |= (x << 24)&((uint32_t)0xff000000);
    REG_PL_WR(RX0_LNA_GAIN_TALE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_6_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_rx0_lna_gain_6_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(RX0_LNA_GAIN_TALE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_5_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_rx0_lna_gain_5_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(RX0_LNA_GAIN_TALE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_lna_gain_4_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_rx0_lna_gain_4_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_1_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(RX0_LNA_GAIN_TALE_1_ADDR,localVal);
}


#define RX0_LNA_GAIN_TALE_2_ADDR (QCC74x_RC2_BASE + 0x00000088)

static inline uint32_t rx0_lna_gain_tale_2_get(void)
{
    return REG_PL_RD(RX0_LNA_GAIN_TALE_2_ADDR);
}

static inline void rx0_lna_gain_tale_2_set(uint32_t x)
{
    REG_PL_WR(RX0_LNA_GAIN_TALE_2_ADDR,x);
}

static inline uint32_t rc2_rx0_lna_gain_8_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_2_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_rx0_lna_gain_8_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_LNA_GAIN_TALE_2_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(RX0_LNA_GAIN_TALE_2_ADDR,localVal);
}


#define RX0_RBB_CFG_TABLE_0_ADDR (QCC74x_RC2_BASE + 0x00000100)

static inline uint32_t rx0_rbb_cfg_table_0_get(void)
{
    return REG_PL_RD(RX0_RBB_CFG_TABLE_0_ADDR);
}

static inline void rx0_rbb_cfg_table_0_set(uint32_t x)
{
    REG_PL_WR(RX0_RBB_CFG_TABLE_0_ADDR,x);
}


#define RX0_RBB_CFG_TABLE_1_ADDR (QCC74x_RC2_BASE + 0x00000104)

static inline uint32_t rx0_rbb_cfg_table_1_get(void)
{
    return REG_PL_RD(RX0_RBB_CFG_TABLE_1_ADDR);
}

static inline void rx0_rbb_cfg_table_1_set(uint32_t x)
{
    REG_PL_WR(RX0_RBB_CFG_TABLE_1_ADDR,x);
}


#define RX0_RBB_CFG_TABLE_2_ADDR (QCC74x_RC2_BASE + 0x00000108)

static inline uint32_t rx0_rbb_cfg_table_2_get(void)
{
    return REG_PL_RD(RX0_RBB_CFG_TABLE_2_ADDR);
}

static inline void rx0_rbb_cfg_table_2_set(uint32_t x)
{
    REG_PL_WR(RX0_RBB_CFG_TABLE_2_ADDR,x);
}


#define RX0_RBB_CFG_TABLE_3_ADDR (QCC74x_RC2_BASE + 0x0000010c)

static inline uint32_t rx0_rbb_cfg_table_3_get(void)
{
    return REG_PL_RD(RX0_RBB_CFG_TABLE_3_ADDR);
}

static inline void rx0_rbb_cfg_table_3_set(uint32_t x)
{
    REG_PL_WR(RX0_RBB_CFG_TABLE_3_ADDR,x);
}


#define RX0_RBB_CFG_TABLE_4_ADDR (QCC74x_RC2_BASE + 0x00000110)

static inline uint32_t rx0_rbb_cfg_table_4_get(void)
{
    return REG_PL_RD(RX0_RBB_CFG_TABLE_4_ADDR);
}

static inline void rx0_rbb_cfg_table_4_set(uint32_t x)
{
    REG_PL_WR(RX0_RBB_CFG_TABLE_4_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_0_ADDR (QCC74x_RC2_BASE + 0x00000180)

static inline uint32_t rx0_nf_gain_table_0_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
}

static inline void rx0_nf_gain_table_0_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_0_ADDR,x);
}

static inline uint32_t rc2_rx0_nf_gain_3_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x3f000000)) >> 24);
}

static inline void rc2_rx0_nf_gain_3_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x3f000000);
    localVal |= (x << 24)&((uint32_t)0x3f000000);
    REG_PL_WR(RX0_NF_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_2_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x003f0000)) >> 16);
}

static inline void rc2_rx0_nf_gain_2_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x003f0000);
    localVal |= (x << 16)&((uint32_t)0x003f0000);
    REG_PL_WR(RX0_NF_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x00003f00)) >> 8);
}

static inline void rc2_rx0_nf_gain_1_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x00003f00);
    localVal |= (x << 8)&((uint32_t)0x00003f00);
    REG_PL_WR(RX0_NF_GAIN_TABLE_0_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    return ((localVal & ((uint32_t)0x0000003f)) >> 0);
}

static inline void rc2_rx0_nf_gain_0_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_0_ADDR);
    localVal &= ~((uint32_t)0x0000003f);
    localVal |= (x << 0)&((uint32_t)0x0000003f);
    REG_PL_WR(RX0_NF_GAIN_TABLE_0_ADDR,localVal);
}


#define RX0_NF_GAIN_TABLE_1_ADDR (QCC74x_RC2_BASE + 0x00000184)

static inline uint32_t rx0_nf_gain_table_1_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
}

static inline void rx0_nf_gain_table_1_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_1_ADDR,x);
}

static inline uint32_t rc2_rx0_nf_gain_7_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    return ((localVal & ((uint32_t)0x3f000000)) >> 24);
}

static inline void rc2_rx0_nf_gain_7_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    localVal &= ~((uint32_t)0x3f000000);
    localVal |= (x << 24)&((uint32_t)0x3f000000);
    REG_PL_WR(RX0_NF_GAIN_TABLE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_6_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    return ((localVal & ((uint32_t)0x003f0000)) >> 16);
}

static inline void rc2_rx0_nf_gain_6_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    localVal &= ~((uint32_t)0x003f0000);
    localVal |= (x << 16)&((uint32_t)0x003f0000);
    REG_PL_WR(RX0_NF_GAIN_TABLE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_5_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    return ((localVal & ((uint32_t)0x00003f00)) >> 8);
}

static inline void rc2_rx0_nf_gain_5_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    localVal &= ~((uint32_t)0x00003f00);
    localVal |= (x << 8)&((uint32_t)0x00003f00);
    REG_PL_WR(RX0_NF_GAIN_TABLE_1_ADDR,localVal);
}

static inline uint32_t rc2_rx0_nf_gain_4_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    return ((localVal & ((uint32_t)0x0000003f)) >> 0);
}

static inline void rc2_rx0_nf_gain_4_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_1_ADDR);
    localVal &= ~((uint32_t)0x0000003f);
    localVal |= (x << 0)&((uint32_t)0x0000003f);
    REG_PL_WR(RX0_NF_GAIN_TABLE_1_ADDR,localVal);
}


#define RX0_NF_GAIN_TABLE_2_ADDR (QCC74x_RC2_BASE + 0x00000188)

static inline uint32_t rx0_nf_gain_table_2_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_2_ADDR);
}

static inline void rx0_nf_gain_table_2_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_2_ADDR,x);
}

static inline uint32_t rc2_rx0_nf_gain_8_getf(void)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_2_ADDR);
    return ((localVal & ((uint32_t)0x0000003f)) >> 0);
}

static inline void rc2_rx0_nf_gain_8_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RX0_NF_GAIN_TABLE_2_ADDR);
    localVal &= ~((uint32_t)0x0000003f);
    localVal |= (x << 0)&((uint32_t)0x0000003f);
    REG_PL_WR(RX0_NF_GAIN_TABLE_2_ADDR,localVal);
}


#define RX0_NF_GAIN_TABLE_3_ADDR (QCC74x_RC2_BASE + 0x0000018c)

static inline uint32_t rx0_nf_gain_table_3_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_3_ADDR);
}

static inline void rx0_nf_gain_table_3_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_3_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_4_ADDR (QCC74x_RC2_BASE + 0x00000190)

static inline uint32_t rx0_nf_gain_table_4_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_4_ADDR);
}

static inline void rx0_nf_gain_table_4_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_4_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_5_ADDR (QCC74x_RC2_BASE + 0x00000194)

static inline uint32_t rx0_nf_gain_table_5_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_5_ADDR);
}

static inline void rx0_nf_gain_table_5_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_5_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_6_ADDR (QCC74x_RC2_BASE + 0x00000198)

static inline uint32_t rx0_nf_gain_table_6_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_6_ADDR);
}

static inline void rx0_nf_gain_table_6_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_6_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_7_ADDR (QCC74x_RC2_BASE + 0x0000019c)

static inline uint32_t rx0_nf_gain_table_7_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_7_ADDR);
}

static inline void rx0_nf_gain_table_7_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_7_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_8_ADDR (QCC74x_RC2_BASE + 0x000001a0)

static inline uint32_t rx0_nf_gain_table_8_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_8_ADDR);
}

static inline void rx0_nf_gain_table_8_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_8_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_9_ADDR (QCC74x_RC2_BASE + 0x000001a4)

static inline uint32_t rx0_nf_gain_table_9_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_9_ADDR);
}

static inline void rx0_nf_gain_table_9_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_9_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_10_ADDR (QCC74x_RC2_BASE + 0x000001a8)

static inline uint32_t rx0_nf_gain_table_10_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_10_ADDR);
}

static inline void rx0_nf_gain_table_10_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_10_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_11_ADDR (QCC74x_RC2_BASE + 0x000001ac)

static inline uint32_t rx0_nf_gain_table_11_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_11_ADDR);
}

static inline void rx0_nf_gain_table_11_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_11_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_12_ADDR (QCC74x_RC2_BASE + 0x000001b0)

static inline uint32_t rx0_nf_gain_table_12_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_12_ADDR);
}

static inline void rx0_nf_gain_table_12_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_12_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_13_ADDR (QCC74x_RC2_BASE + 0x000001b4)

static inline uint32_t rx0_nf_gain_table_13_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_13_ADDR);
}

static inline void rx0_nf_gain_table_13_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_13_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_14_ADDR (QCC74x_RC2_BASE + 0x000001b8)

static inline uint32_t rx0_nf_gain_table_14_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_14_ADDR);
}

static inline void rx0_nf_gain_table_14_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_14_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_15_ADDR (QCC74x_RC2_BASE + 0x000001bc)

static inline uint32_t rx0_nf_gain_table_15_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_15_ADDR);
}

static inline void rx0_nf_gain_table_15_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_15_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_16_ADDR (QCC74x_RC2_BASE + 0x000001c0)

static inline uint32_t rx0_nf_gain_table_16_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_16_ADDR);
}

static inline void rx0_nf_gain_table_16_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_16_ADDR,x);
}


#define RX0_NF_GAIN_TABLE_17_ADDR (QCC74x_RC2_BASE + 0x000001c4)

static inline uint32_t rx0_nf_gain_table_17_get(void)
{
    return REG_PL_RD(RX0_NF_GAIN_TABLE_17_ADDR);
}

static inline void rx0_nf_gain_table_17_set(uint32_t x)
{
    REG_PL_WR(RX0_NF_GAIN_TABLE_17_ADDR,x);
}


#define TX0_TXG_CFG_TABLE_0_ADDR (QCC74x_RC2_BASE + 0x00000200)

static inline uint32_t tx0_txg_cfg_table_0_get(void)
{
    return REG_PL_RD(TX0_TXG_CFG_TABLE_0_ADDR);
}

static inline void tx0_txg_cfg_table_0_set(uint32_t x)
{
    REG_PL_WR(TX0_TXG_CFG_TABLE_0_ADDR,x);
}


#define TX0_TXG_CFG_TABLE_1_ADDR (QCC74x_RC2_BASE + 0x00000204)

static inline uint32_t tx0_txg_cfg_table_1_get(void)
{
    return REG_PL_RD(TX0_TXG_CFG_TABLE_1_ADDR);
}

static inline void tx0_txg_cfg_table_1_set(uint32_t x)
{
    REG_PL_WR(TX0_TXG_CFG_TABLE_1_ADDR,x);
}


#define TX0_TXG_CFG_TABLE_2_ADDR (QCC74x_RC2_BASE + 0x00000208)

static inline uint32_t tx0_txg_cfg_table_2_get(void)
{
    return REG_PL_RD(TX0_TXG_CFG_TABLE_2_ADDR);
}

static inline void tx0_txg_cfg_table_2_set(uint32_t x)
{
    REG_PL_WR(TX0_TXG_CFG_TABLE_2_ADDR,x);
}


#define TX0_TXG_CFG_TABLE_3_ADDR (QCC74x_RC2_BASE + 0x0000020c)

static inline uint32_t tx0_txg_cfg_table_3_get(void)
{
    return REG_PL_RD(TX0_TXG_CFG_TABLE_3_ADDR);
}

static inline void tx0_txg_cfg_table_3_set(uint32_t x)
{
    REG_PL_WR(TX0_TXG_CFG_TABLE_3_ADDR,x);
}


#define TX0_TBBD_CFG_TABLE_0_ADDR (QCC74x_RC2_BASE + 0x00000210)

static inline uint32_t tx0_tbbd_cfg_table_0_get(void)
{
    return REG_PL_RD(TX0_TBBD_CFG_TABLE_0_ADDR);
}

static inline void tx0_tbbd_cfg_table_0_set(uint32_t x)
{
    REG_PL_WR(TX0_TBBD_CFG_TABLE_0_ADDR,x);
}


#define TX0_TBBD_CFG_TABLE_1_ADDR (QCC74x_RC2_BASE + 0x00000214)

static inline uint32_t tx0_tbbd_cfg_table_1_get(void)
{
    return REG_PL_RD(TX0_TBBD_CFG_TABLE_1_ADDR);
}

static inline void tx0_tbbd_cfg_table_1_set(uint32_t x)
{
    REG_PL_WR(TX0_TBBD_CFG_TABLE_1_ADDR,x);
}


#define TX_FRONTEND_0_ADDR (QCC74x_RC2_BASE + 0x00000218)

static inline uint32_t tx_frontend_0_get(void)
{
    return REG_PL_RD(TX_FRONTEND_0_ADDR);
}

static inline void tx_frontend_0_set(uint32_t x)
{
    REG_PL_WR(TX_FRONTEND_0_ADDR,x);
}

static inline uint32_t rc2_txhbf20coeffsel_getf(void)
{
    uint32_t localVal = REG_PL_RD(TX_FRONTEND_0_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}

static inline void rc2_txhbf20coeffsel_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(TX_FRONTEND_0_ADDR);
    localVal &= ~((uint32_t)0xffffffff);
    localVal |= (x << 0)&((uint32_t)0xffffffff);
    REG_PL_WR(TX_FRONTEND_0_ADDR,localVal);
}


#define RWNXFERXINCNTL_ADDR (QCC74x_RC2_BASE + 0x00000800)

static inline uint32_t rwnxferxincntl_get(void)
{
    return REG_PL_RD(RWNXFERXINCNTL_ADDR);
}

static inline void rwnxferxincntl_set(uint32_t x)
{
    REG_PL_WR(RWNXFERXINCNTL_ADDR,x);
}

static inline uint32_t rc2_rx_ramp_ctrl_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0xc0000000)) >> 30);
}

static inline void rc2_rx_ramp_ctrl_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0xc0000000);
    localVal |= (x << 30)&((uint32_t)0xc0000000);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_feadc0_dly_ena_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00080000)) >> 19);
}

static inline void rc2_feadc0_dly_ena_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x00080000);
    localVal |= (x << 19)&((uint32_t)0x00080000);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_feadc0_dly_num_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00070000)) >> 16);
}

static inline void rc2_feadc0_dly_num_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x00070000);
    localVal |= (x << 16)&((uint32_t)0x00070000);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_fe0dcest_dly_ena_20_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00001000)) >> 12);
}

static inline void rc2_fe0dcest_dly_ena_20_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x00001000);
    localVal |= (x << 12)&((uint32_t)0x00001000);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_fe0dcest_dly_num_20_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000f00)) >> 8);
}

static inline void rc2_fe0dcest_dly_num_20_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x00000f00);
    localVal |= (x << 8)&((uint32_t)0x00000f00);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_fe0dcest_dly_ena_40_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x00000010)) >> 4);
}

static inline void rc2_fe0dcest_dly_ena_40_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x00000010);
    localVal |= (x << 4)&((uint32_t)0x00000010);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}

static inline uint32_t rc2_fe0dcest_dly_num_40_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    return ((localVal & ((uint32_t)0x0000000f)) >> 0);
}

static inline void rc2_fe0dcest_dly_num_40_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFERXINCNTL_ADDR);
    localVal &= ~((uint32_t)0x0000000f);
    localVal |= (x << 0)&((uint32_t)0x0000000f);
    REG_PL_WR(RWNXFERXINCNTL_ADDR,localVal);
}


#define RC2_CCA_CTRL_ADDR (QCC74x_RC2_BASE + 0x00000804)

static inline uint32_t rc2_cca_ctrl_get(void)
{
    return REG_PL_RD(RC2_CCA_CTRL_ADDR);
}

static inline void rc2_cca_ctrl_set(uint32_t x)
{
    REG_PL_WR(RC2_CCA_CTRL_ADDR,x);
}

static inline uint32_t rc2_cr_4s_cca_primary_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_CCA_CTRL_ADDR);
    return ((localVal & ((uint32_t)0x00000001)) >> 0);
}

static inline void rc2_cr_4s_cca_primary_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RC2_CCA_CTRL_ADDR);
    localVal &= ~((uint32_t)0x00000001);
    localVal |= (x << 0)&((uint32_t)0x00000001);
    REG_PL_WR(RC2_CCA_CTRL_ADDR,localVal);
}


#define RWNXFETXDCCOMP_ADDR (QCC74x_RC2_BASE + 0x00000808)

static inline uint32_t rwnxfetxdccomp_get(void)
{
    return REG_PL_RD(RWNXFETXDCCOMP_ADDR);
}

static inline void rwnxfetxdccomp_set(uint32_t x)
{
    REG_PL_WR(RWNXFETXDCCOMP_ADDR,x);
}


#define RWNXFECTRL0_ADDR (QCC74x_RC2_BASE + 0x0000080c)

static inline uint32_t rwnxfectrl0_get(void)
{
    return REG_PL_RD(RWNXFECTRL0_ADDR);
}

static inline void rwnxfectrl0_set(uint32_t x)
{
    REG_PL_WR(RWNXFECTRL0_ADDR,x);
}

static inline uint32_t rc2_txdsssdiggainlin0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXFECTRL0_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline void rc2_txdsssdiggainlin0_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXFECTRL0_ADDR);
    localVal &= ~((uint32_t)0xff000000);
    localVal |= (x << 24)&((uint32_t)0xff000000);
    REG_PL_WR(RWNXFECTRL0_ADDR,localVal);
}


#define RWNXAGCDSP5_ADDR (QCC74x_RC2_BASE + 0x00000810)

static inline uint32_t rwnxagcdsp5_get(void)
{
    return REG_PL_RD(RWNXAGCDSP5_ADDR);
}

static inline void rwnxagcdsp5_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCDSP5_ADDR,x);
}


#define RWNXAGCPKDETIF_ADDR (QCC74x_RC2_BASE + 0x00000814)

static inline uint32_t rwnxagcpkdetif_get(void)
{
    return REG_PL_RD(RWNXAGCPKDETIF_ADDR);
}

static inline void rwnxagcpkdetif_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCPKDETIF_ADDR,x);
}

static inline uint32_t rc2_pkdet_cnt_win_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    return ((localVal & ((uint32_t)0x000003c0)) >> 6);
}

static inline void rc2_pkdet_cnt_win_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    localVal &= ~((uint32_t)0x000003c0);
    localVal |= (x << 6)&((uint32_t)0x000003c0);
    REG_PL_WR(RWNXAGCPKDETIF_ADDR,localVal);
}

static inline uint32_t rc2_pkdet_cnt_thr_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    return ((localVal & ((uint32_t)0x0000003c)) >> 2);
}

static inline void rc2_pkdet_cnt_thr_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    localVal &= ~((uint32_t)0x0000003c);
    localVal |= (x << 2)&((uint32_t)0x0000003c);
    REG_PL_WR(RWNXAGCPKDETIF_ADDR,localVal);
}

static inline uint32_t rc2_pkdet_mode_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    return ((localVal & ((uint32_t)0x00000003)) >> 0);
}

static inline void rc2_pkdet_mode_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCPKDETIF_ADDR);
    localVal &= ~((uint32_t)0x00000003);
    localVal |= (x << 0)&((uint32_t)0x00000003);
    REG_PL_WR(RWNXAGCPKDETIF_ADDR,localVal);
}


#define RWNXAGCOFDMDET0_ADDR (QCC74x_RC2_BASE + 0x00000818)

static inline uint32_t rwnxagcofdmdet0_get(void)
{
    return REG_PL_RD(RWNXAGCOFDMDET0_ADDR);
}

static inline void rwnxagcofdmdet0_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCOFDMDET0_ADDR,x);
}


#define RWNXAGCOFDMDET1_ADDR (QCC74x_RC2_BASE + 0x0000081c)

static inline uint32_t rwnxagcofdmdet1_get(void)
{
    return REG_PL_RD(RWNXAGCOFDMDET1_ADDR);
}

static inline void rwnxagcofdmdet1_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCOFDMDET1_ADDR,x);
}


#define RWNXAGCOFDMDET2_ADDR (QCC74x_RC2_BASE + 0x00000820)

static inline uint32_t rwnxagcofdmdet2_get(void)
{
    return REG_PL_RD(RWNXAGCOFDMDET2_ADDR);
}

static inline void rwnxagcofdmdet2_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCOFDMDET2_ADDR,x);
}


#define RWNXAGCOFDMDET3_ADDR (QCC74x_RC2_BASE + 0x00000824)

static inline uint32_t rwnxagcofdmdet3_get(void)
{
    return REG_PL_RD(RWNXAGCOFDMDET3_ADDR);
}

static inline void rwnxagcofdmdet3_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCOFDMDET3_ADDR,x);
}


#define RWNXAGCFASTCNTL_ADDR (QCC74x_RC2_BASE + 0x00000828)

static inline uint32_t rwnxagcfastcntl_get(void)
{
    return REG_PL_RD(RWNXAGCFASTCNTL_ADDR);
}

static inline void rwnxagcfastcntl_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCFASTCNTL_ADDR,x);
}


#define RWNXAGCENERGYCOMP_ADDR (QCC74x_RC2_BASE + 0x0000082c)

static inline uint32_t rwnxagcenergycomp_get(void)
{
    return REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
}

static inline void rwnxagcenergycomp_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,x);
}

static inline uint32_t rc2_inbdpowfastvalid_cnt_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00ff8000)) >> 15);
}

static inline void rc2_inbdpowfastvalid_cnt_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00ff8000);
    localVal |= (x << 15)&((uint32_t)0x00ff8000);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpowfastvalid_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00004000)) >> 14);
}

static inline void rc2_inbdpowfastvalid_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00004000);
    localVal |= (x << 14)&((uint32_t)0x00004000);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpowinfthr_adj_step_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00003000)) >> 12);
}

static inline void rc2_inbdpowinfthr_adj_step_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00003000);
    localVal |= (x << 12)&((uint32_t)0x00003000);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpowinfthr_adj_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00000800)) >> 11);
}

static inline void rc2_inbdpowinfthr_adj_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00000800);
    localVal |= (x << 11)&((uint32_t)0x00000800);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpowsupthr_adj_step_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00000600)) >> 9);
}

static inline void rc2_inbdpowsupthr_adj_step_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00000600);
    localVal |= (x << 9)&((uint32_t)0x00000600);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpowsupthr_adj_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x00000100)) >> 8);
}

static inline void rc2_inbdpowsupthr_adj_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x00000100);
    localVal |= (x << 8)&((uint32_t)0x00000100);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}

static inline uint32_t rc2_inbdpow_adj_thr_dbm_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_inbdpow_adj_thr_dbm_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCENERGYCOMP_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(RWNXAGCENERGYCOMP_ADDR,localVal);
}


#define RWNXAGCEVT4_ADDR (QCC74x_RC2_BASE + 0x00000830)

static inline uint32_t rwnxagcevt4_get(void)
{
    return REG_PL_RD(RWNXAGCEVT4_ADDR);
}

static inline void rwnxagcevt4_set(uint32_t x)
{
    REG_PL_WR(RWNXAGCEVT4_ADDR,x);
}

static inline uint32_t rc2_evt4op1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0xfc000000)) >> 26);
}

static inline void rc2_evt4op1_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0xfc000000);
    localVal |= (x << 26)&((uint32_t)0xfc000000);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}

static inline uint32_t rc2_evt4op2_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0x03f00000)) >> 20);
}

static inline void rc2_evt4op2_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0x03f00000);
    localVal |= (x << 20)&((uint32_t)0x03f00000);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}

static inline uint32_t rc2_evt4op3_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0x000fc000)) >> 14);
}

static inline void rc2_evt4op3_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0x000fc000);
    localVal |= (x << 14)&((uint32_t)0x000fc000);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}

static inline uint32_t rc2_evt4pathcomb_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0x00002000)) >> 13);
}

static inline void rc2_evt4pathcomb_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0x00002000);
    localVal |= (x << 13)&((uint32_t)0x00002000);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}

static inline uint32_t rc2_evt4opcomb_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0x00001c00)) >> 10);
}

static inline void rc2_evt4opcomb_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0x00001c00);
    localVal |= (x << 10)&((uint32_t)0x00001c00);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}

static inline uint32_t rc2_evt4tgtadd_getf(void)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    return ((localVal & ((uint32_t)0x000001ff)) >> 0);
}

static inline void rc2_evt4tgtadd_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(RWNXAGCEVT4_ADDR);
    localVal &= ~((uint32_t)0x000001ff);
    localVal |= (x << 0)&((uint32_t)0x000001ff);
    REG_PL_WR(RWNXAGCEVT4_ADDR,localVal);
}


#define ADCPOWEST_ADDR (QCC74x_RC2_BASE + 0x00000834)

static inline uint32_t adcpowest_get(void)
{
    return REG_PL_RD(ADCPOWEST_ADDR);
}

static inline void adcpowest_set(uint32_t x)
{
    REG_PL_WR(ADCPOWEST_ADDR,x);
}

static inline uint32_t rc2_adcpowinsel_getf(void)
{
    uint32_t localVal = REG_PL_RD(ADCPOWEST_ADDR);
    return ((localVal & ((uint32_t)0x00000003)) >> 0);
}

static inline void rc2_adcpowinsel_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(ADCPOWEST_ADDR);
    localVal &= ~((uint32_t)0x00000003);
    localVal |= (x << 0)&((uint32_t)0x00000003);
    REG_PL_WR(ADCPOWEST_ADDR,localVal);
}


#define CORRCOMP0_ADDR (QCC74x_RC2_BASE + 0x00000838)

static inline uint32_t corrcomp0_get(void)
{
    return REG_PL_RD(CORRCOMP0_ADDR);
}

static inline void corrcomp0_set(uint32_t x)
{
    REG_PL_WR(CORRCOMP0_ADDR,x);
}

static inline uint32_t rc2_reflevofdmthd_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP0_ADDR);
    return ((localVal & ((uint32_t)0x80000000)) >> 31);
}

static inline void rc2_reflevofdmthd_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP0_ADDR);
    localVal &= ~((uint32_t)0x80000000);
    localVal |= (x << 31)&((uint32_t)0x80000000);
    REG_PL_WR(CORRCOMP0_ADDR,localVal);
}

static inline uint32_t rc2_reflevofdmthd_getf(void)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP0_ADDR);
    return ((localVal & ((uint32_t)0x0007ffff)) >> 0);
}

static inline void rc2_reflevofdmthd_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP0_ADDR);
    localVal &= ~((uint32_t)0x0007ffff);
    localVal |= (x << 0)&((uint32_t)0x0007ffff);
    REG_PL_WR(CORRCOMP0_ADDR,localVal);
}


#define CORRCOMP1_ADDR (QCC74x_RC2_BASE + 0x0000083c)

static inline uint32_t corrcomp1_get(void)
{
    return REG_PL_RD(CORRCOMP1_ADDR);
}

static inline void corrcomp1_set(uint32_t x)
{
    REG_PL_WR(CORRCOMP1_ADDR,x);
}

static inline uint32_t rc2_reflevdsssthd_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP1_ADDR);
    return ((localVal & ((uint32_t)0x80000000)) >> 31);
}

static inline void rc2_reflevdsssthd_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP1_ADDR);
    localVal &= ~((uint32_t)0x80000000);
    localVal |= (x << 31)&((uint32_t)0x80000000);
    REG_PL_WR(CORRCOMP1_ADDR,localVal);
}

static inline uint32_t rc2_reflevdsssthd_getf(void)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP1_ADDR);
    return ((localVal & ((uint32_t)0x000fffff)) >> 0);
}

static inline void rc2_reflevdsssthd_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(CORRCOMP1_ADDR);
    localVal &= ~((uint32_t)0x000fffff);
    localVal |= (x << 0)&((uint32_t)0x000fffff);
    REG_PL_WR(CORRCOMP1_ADDR,localVal);
}


#define DSSSDET_ADDR (QCC74x_RC2_BASE + 0x00000840)

static inline uint32_t dsssdet_get(void)
{
    return REG_PL_RD(DSSSDET_ADDR);
}

static inline void dsssdet_set(uint32_t x)
{
    REG_PL_WR(DSSSDET_ADDR,x);
}

static inline uint32_t rc2_reflevdssscontthd_en_getf(void)
{
    uint32_t localVal = REG_PL_RD(DSSSDET_ADDR);
    return ((localVal & ((uint32_t)0x80000000)) >> 31);
}

static inline void rc2_reflevdssscontthd_en_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(DSSSDET_ADDR);
    localVal &= ~((uint32_t)0x80000000);
    localVal |= (x << 31)&((uint32_t)0x80000000);
    REG_PL_WR(DSSSDET_ADDR,localVal);
}

static inline uint32_t rc2_reflevdssscontthd_getf(void)
{
    uint32_t localVal = REG_PL_RD(DSSSDET_ADDR);
    return ((localVal & ((uint32_t)0x003fffff)) >> 0);
}

static inline void rc2_reflevdssscontthd_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(DSSSDET_ADDR);
    localVal &= ~((uint32_t)0x003fffff);
    localVal |= (x << 0)&((uint32_t)0x003fffff);
    REG_PL_WR(DSSSDET_ADDR,localVal);
}


#define AGCRAMPUP0_ADDR (QCC74x_RC2_BASE + 0x00000844)

static inline uint32_t agcrampup0_get(void)
{
    return REG_PL_RD(AGCRAMPUP0_ADDR);
}

static inline void agcrampup0_set(uint32_t x)
{
    REG_PL_WR(AGCRAMPUP0_ADDR,x);
}

static inline uint32_t rc2_agcrampupthr2_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP0_ADDR);
    return ((localVal & ((uint32_t)0x3fff0000)) >> 16);
}

static inline void rc2_agcrampupthr2_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP0_ADDR);
    localVal &= ~((uint32_t)0x3fff0000);
    localVal |= (x << 16)&((uint32_t)0x3fff0000);
    REG_PL_WR(AGCRAMPUP0_ADDR,localVal);
}

static inline uint32_t rc2_agcrampupthr1_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP0_ADDR);
    return ((localVal & ((uint32_t)0x00003fff)) >> 0);
}

static inline void rc2_agcrampupthr1_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP0_ADDR);
    localVal &= ~((uint32_t)0x00003fff);
    localVal |= (x << 0)&((uint32_t)0x00003fff);
    REG_PL_WR(AGCRAMPUP0_ADDR,localVal);
}


#define AGCRAMPUP1_ADDR (QCC74x_RC2_BASE + 0x00000848)

static inline uint32_t agcrampup1_get(void)
{
    return REG_PL_RD(AGCRAMPUP1_ADDR);
}

static inline void agcrampup1_set(uint32_t x)
{
    REG_PL_WR(AGCRAMPUP1_ADDR,x);
}

static inline uint32_t rc2_agcrampupthr3_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP1_ADDR);
    return ((localVal & ((uint32_t)0x00003fff)) >> 0);
}

static inline void rc2_agcrampupthr3_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP1_ADDR);
    localVal &= ~((uint32_t)0x00003fff);
    localVal |= (x << 0)&((uint32_t)0x00003fff);
    REG_PL_WR(AGCRAMPUP1_ADDR,localVal);
}


#define AGCRAMPUP2_ADDR (QCC74x_RC2_BASE + 0x0000084c)

static inline uint32_t agcrampup2_get(void)
{
    return REG_PL_RD(AGCRAMPUP2_ADDR);
}

static inline void agcrampup2_set(uint32_t x)
{
    REG_PL_WR(AGCRAMPUP2_ADDR,x);
}

static inline uint32_t rc2_agcrampupcorrecten_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    return ((localVal & ((uint32_t)0x80000000)) >> 31);
}

static inline void rc2_agcrampupcorrecten_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    localVal &= ~((uint32_t)0x80000000);
    localVal |= (x << 31)&((uint32_t)0x80000000);
    REG_PL_WR(AGCRAMPUP2_ADDR,localVal);
}

static inline uint32_t rc2_agcrampupcorrect3_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_agcrampupcorrect3_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(AGCRAMPUP2_ADDR,localVal);
}

static inline uint32_t rc2_agcrampupcorrect2_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_agcrampupcorrect2_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(AGCRAMPUP2_ADDR,localVal);
}

static inline uint32_t rc2_agcrampupcorrect1_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_agcrampupcorrect1_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCRAMPUP2_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(AGCRAMPUP2_ADDR,localVal);
}


#define AGCLOADREG0_ADDR (QCC74x_RC2_BASE + 0x00000850)

static inline uint32_t agcloadreg0_get(void)
{
    return REG_PL_RD(AGCLOADREG0_ADDR);
}

static inline void agcloadreg0_set(uint32_t x)
{
    REG_PL_WR(AGCLOADREG0_ADDR,x);
}

static inline uint32_t rc2_reglnaindex_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline void rc2_reglnaindex_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    localVal &= ~((uint32_t)0xff000000);
    localVal |= (x << 24)&((uint32_t)0xff000000);
    REG_PL_WR(AGCLOADREG0_ADDR,localVal);
}

static inline uint32_t rc2_reglnagain_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_reglnagain_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(AGCLOADREG0_ADDR,localVal);
}

static inline uint32_t rc2_reglnastep_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_reglnastep_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(AGCLOADREG0_ADDR,localVal);
}

static inline uint32_t rc2_regrbbstep_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_regrbbstep_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG0_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(AGCLOADREG0_ADDR,localVal);
}


#define AGCLOADREG1_ADDR (QCC74x_RC2_BASE + 0x00000854)

static inline uint32_t agcloadreg1_get(void)
{
    return REG_PL_RD(AGCLOADREG1_ADDR);
}

static inline void agcloadreg1_set(uint32_t x)
{
    REG_PL_WR(AGCLOADREG1_ADDR,x);
}

static inline uint32_t rc2_regrfgaininit_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline void rc2_regrfgaininit_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    localVal &= ~((uint32_t)0xff000000);
    localVal |= (x << 24)&((uint32_t)0xff000000);
    REG_PL_WR(AGCLOADREG1_ADDR,localVal);
}

static inline uint32_t rc2_regadctgtdvb_11b_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_regadctgtdvb_11b_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(AGCLOADREG1_ADDR,localVal);
}

static inline uint32_t rc2_regadctgtdvb_11g_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_regadctgtdvb_11g_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(AGCLOADREG1_ADDR,localVal);
}

static inline uint32_t rc2_regadctgtdvb_11n_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_regadctgtdvb_11n_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG1_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(AGCLOADREG1_ADDR,localVal);
}


#define AGCLOADREG2_ADDR (QCC74x_RC2_BASE + 0x00000858)

static inline uint32_t agcloadreg2_get(void)
{
    return REG_PL_RD(AGCLOADREG2_ADDR);
}

static inline void agcloadreg2_set(uint32_t x)
{
    REG_PL_WR(AGCLOADREG2_ADDR,x);
}

static inline uint32_t rc2_reginbdtgtdbv_11b_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_reginbdtgtdbv_11b_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(AGCLOADREG2_ADDR,localVal);
}

static inline uint32_t rc2_reginbdtgtdbv_11g_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_reginbdtgtdbv_11g_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(AGCLOADREG2_ADDR,localVal);
}

static inline uint32_t rc2_reginbdtgtdbv_11n_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_reginbdtgtdbv_11n_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG2_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(AGCLOADREG2_ADDR,localVal);
}


#define AGCLOADREG3_ADDR (QCC74x_RC2_BASE + 0x0000085c)

static inline uint32_t agcloadreg3_get(void)
{
    return REG_PL_RD(AGCLOADREG3_ADDR);
}

static inline void agcloadreg3_set(uint32_t x)
{
    REG_PL_WR(AGCLOADREG3_ADDR,x);
}

static inline uint32_t rc2_regmaxgain_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline void rc2_regmaxgain_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    localVal &= ~((uint32_t)0x00ff0000);
    localVal |= (x << 16)&((uint32_t)0x00ff0000);
    REG_PL_WR(AGCLOADREG3_ADDR,localVal);
}

static inline uint32_t rc2_regmingain_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline void rc2_regmingain_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    localVal &= ~((uint32_t)0x0000ff00);
    localVal |= (x << 8)&((uint32_t)0x0000ff00);
    REG_PL_WR(AGCLOADREG3_ADDR,localVal);
}

static inline uint32_t rc2_regmingain4sat_getf(void)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}

static inline void rc2_regmingain4sat_setf(uint32_t x)
{
    uint32_t localVal = REG_PL_RD(AGCLOADREG3_ADDR);
    localVal &= ~((uint32_t)0x000000ff);
    localVal |= (x << 0)&((uint32_t)0x000000ff);
    REG_PL_WR(AGCLOADREG3_ADDR,localVal);
}


#define RC2_STATUS_0_ADDR (QCC74x_RC2_BASE + 0x00000900)

static inline uint32_t rc2_status_0_get(void)
{
    return REG_PL_RD(RC2_STATUS_0_ADDR);
}

static inline void rc2_status_0_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_0_ADDR,x);
}

static inline uint32_t rc2_sts_agc_gain_target0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_0_ADDR);
    return ((localVal & ((uint32_t)0x000000ff)) >> 0);
}


#define RC2_STATUS_1_ADDR (QCC74x_RC2_BASE + 0x00000904)

static inline uint32_t rc2_status_1_get(void)
{
    return REG_PL_RD(RC2_STATUS_1_ADDR);
}

static inline void rc2_status_1_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_1_ADDR,x);
}

static inline uint32_t rc2_sts_rx0_lna_gain_idx_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_1_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline uint32_t rc2_sts_rx0_rbb_cfg_idx_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_1_ADDR);
    return ((localVal & ((uint32_t)0x001f0000)) >> 16);
}


#define RC2_STATUS_2_ADDR (QCC74x_RC2_BASE + 0x00000908)

static inline uint32_t rc2_status_2_get(void)
{
    return REG_PL_RD(RC2_STATUS_2_ADDR);
}

static inline void rc2_status_2_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_2_ADDR,x);
}

static inline uint32_t rc2_sts_rx0_lna_gain_db_s_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_2_ADDR);
    return ((localVal & ((uint32_t)0xff000000)) >> 24);
}

static inline uint32_t rc2_sts_rx0_vga_gain_db_s_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_2_ADDR);
    return ((localVal & ((uint32_t)0x00ff0000)) >> 16);
}

static inline uint32_t rc2_sts_rx0_total_gain_db_s_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_2_ADDR);
    return ((localVal & ((uint32_t)0x0000ff00)) >> 8);
}

static inline uint32_t rc2_sts_rx0_nf_gain_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_2_ADDR);
    return ((localVal & ((uint32_t)0x0000003f)) >> 0);
}


#define RC2_STATUS_3_ADDR (QCC74x_RC2_BASE + 0x0000090c)

static inline uint32_t rc2_status_3_get(void)
{
    return REG_PL_RD(RC2_STATUS_3_ADDR);
}

static inline void rc2_status_3_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_3_ADDR,x);
}

static inline uint32_t rc2_sts_rw_nx_diag0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_3_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_4_ADDR (QCC74x_RC2_BASE + 0x00000910)

static inline uint32_t rc2_status_4_get(void)
{
    return REG_PL_RD(RC2_STATUS_4_ADDR);
}

static inline void rc2_status_4_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_4_ADDR,x);
}

static inline uint32_t rc2_sts_rw_nx_diag1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_4_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_5_ADDR (QCC74x_RC2_BASE + 0x00000914)

static inline uint32_t rc2_status_5_get(void)
{
    return REG_PL_RD(RC2_STATUS_5_ADDR);
}

static inline void rc2_status_5_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_5_ADDR,x);
}

static inline uint32_t rc2_sts_rw_nx_diag2_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_5_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_6_ADDR (QCC74x_RC2_BASE + 0x00000918)

static inline uint32_t rc2_status_6_get(void)
{
    return REG_PL_RD(RC2_STATUS_6_ADDR);
}

static inline void rc2_status_6_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_6_ADDR,x);
}

static inline uint32_t rc2_sts_diag_phy0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_6_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_7_ADDR (QCC74x_RC2_BASE + 0x0000091c)

static inline uint32_t rc2_status_7_get(void)
{
    return REG_PL_RD(RC2_STATUS_7_ADDR);
}

static inline void rc2_status_7_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_7_ADDR,x);
}

static inline uint32_t rc2_sts_diag_phy1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_7_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_8_ADDR (QCC74x_RC2_BASE + 0x00000920)

static inline uint32_t rc2_status_8_get(void)
{
    return REG_PL_RD(RC2_STATUS_8_ADDR);
}

static inline void rc2_status_8_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_8_ADDR,x);
}

static inline uint32_t rc2_sts_diag_riu0_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_8_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}


#define RC2_STATUS_9_ADDR (QCC74x_RC2_BASE + 0x00000924)

static inline uint32_t rc2_status_9_get(void)
{
    return REG_PL_RD(RC2_STATUS_9_ADDR);
}

static inline void rc2_status_9_set(uint32_t x)
{
    REG_PL_WR(RC2_STATUS_9_ADDR,x);
}

static inline uint32_t rc2_sts_diag_riu1_getf(void)
{
    uint32_t localVal = REG_PL_RD(RC2_STATUS_9_ADDR);
    return ((localVal & ((uint32_t)0xffffffff)) >> 0);
}

#endif