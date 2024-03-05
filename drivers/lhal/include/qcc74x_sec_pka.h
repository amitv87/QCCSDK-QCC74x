#ifndef _QCC74x_SEC_PKA_H
#define _QCC74x_SEC_PKA_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

/** @addtogroup SEC_PKA
  * @{
  */

#define SEC_ENG_PKA_REG_SIZE_8   1
#define SEC_ENG_PKA_REG_SIZE_16  2
#define SEC_ENG_PKA_REG_SIZE_32  3
#define SEC_ENG_PKA_REG_SIZE_64  4
#define SEC_ENG_PKA_REG_SIZE_96  5
#define SEC_ENG_PKA_REG_SIZE_128 6
#define SEC_ENG_PKA_REG_SIZE_192 7
#define SEC_ENG_PKA_REG_SIZE_256 8
#define SEC_ENG_PKA_REG_SIZE_384 9
#define SEC_ENG_PKA_REG_SIZE_512 10

#define SEC_ENG_PKA_OP_PPSEL         0x00
#define SEC_ENG_PKA_OP_LMOD2N        0x11
#define SEC_ENG_PKA_OP_LDIV2N        0x12
#define SEC_ENG_PKA_OP_LMUL2N        0x13
#define SEC_ENG_PKA_OP_LDIV          0x14
#define SEC_ENG_PKA_OP_LSQR          0x15
#define SEC_ENG_PKA_OP_LMUL          0x16
#define SEC_ENG_PKA_OP_LSUB          0x17
#define SEC_ENG_PKA_OP_LADD          0x18
#define SEC_ENG_PKA_OP_LCMP          0x19
#define SEC_ENG_PKA_OP_MDIV2         0x21
#define SEC_ENG_PKA_OP_MINV          0x22
#define SEC_ENG_PKA_OP_MEXP          0x23
#define SEC_ENG_PKA_OP_MSQR          0x24
#define SEC_ENG_PKA_OP_MMUL          0x25
#define SEC_ENG_PKA_OP_MREM          0x26
#define SEC_ENG_PKA_OP_MSUB          0x27
#define SEC_ENG_PKA_OP_MADD          0x28
#define SEC_ENG_PKA_OP_RESIZE        0x31
#define SEC_ENG_PKA_OP_MOVDAT        0x32
#define SEC_ENG_PKA_OP_NLIR          0x33
#define SEC_ENG_PKA_OP_SLIR          0x34
#define SEC_ENG_PKA_OP_CLIR          0x35
#define SEC_ENG_PKA_OP_CFLIRI_BUFFER 0x36
#define SEC_ENG_PKA_OP_CTLIRI_PLD    0x37
#define SEC_ENG_PKA_OP_CFLIR_BUFFER  0x38
#define SEC_ENG_PKA_OP_CTLIR_PLD     0x39

#ifdef __cplusplus
extern "C" {
#endif

void qcc74x_pka_init(struct qcc74x_device_s *dev);
void qcc74x_pka_deinit(struct qcc74x_device_s *dev);

void qcc74x_pka_lmod2n(struct qcc74x_device_s *dev,
                     uint8_t s0_regindex,
                     uint8_t s0_regsize,
                     uint8_t d0_regindex,
                     uint8_t d0_regsize,
                     uint16_t bit_shift,
                     uint8_t lastop);

void qcc74x_pka_ldiv2n(struct qcc74x_device_s *dev,
                     uint8_t s0_regindex,
                     uint8_t s0_regsize,
                     uint8_t d0_regindex,
                     uint8_t d0_regsize,
                     uint16_t bit_shift,
                     uint8_t lastop);

void qcc74x_pka_lmul2n(struct qcc74x_device_s *dev,
                     uint8_t s0_regindex,
                     uint8_t s0_regsize,
                     uint8_t d0_regindex,
                     uint8_t d0_regsize,
                     uint16_t bit_shift,
                     uint8_t lastop);

void qcc74x_pka_ldiv(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_lsqr(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t lastop);

void qcc74x_pka_lmul(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t lastop);

void qcc74x_pka_lsub(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t lastop);

void qcc74x_pka_ladd(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t lastop);

uint8_t qcc74x_pka_lcmp(struct qcc74x_device_s *dev,
                      uint8_t s0_regindex,
                      uint8_t s0_regsize,
                      uint8_t s1_regindex,
                      uint8_t s1_regsize);

void qcc74x_pka_minv(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_mexp(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_msqr(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_mmul(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_mrem(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_msub(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_madd(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t s1_regindex,
                   uint8_t s1_regsize,
                   uint8_t s2_regindex,
                   uint8_t s2_regsize,
                   uint8_t lastop);

void qcc74x_pka_regsize(struct qcc74x_device_s *dev,
                      uint8_t s0_regindex,
                      uint8_t s0_regsize,
                      uint8_t d0_regindex,
                      uint8_t d0_regsize,
                      uint8_t lastop);

void qcc74x_pka_movdat(struct qcc74x_device_s *dev,
                     uint8_t s0_regindex,
                     uint8_t s0_regsize,
                     uint8_t d0_regindex,
                     uint8_t d0_regsize,
                     uint8_t lastop);

void qcc74x_pka_nlir(struct qcc74x_device_s *dev,
                   uint8_t s0_regindex,
                   uint8_t s0_regsize,
                   uint8_t d0_regindex,
                   uint8_t d0_regsize,
                   uint8_t lastop);

void qcc74x_pka_slir(struct qcc74x_device_s *dev,
                   uint8_t regindex,
                   uint8_t regsize,
                   uint32_t data,
                   uint8_t lastop);

void qcc74x_pka_clir(struct qcc74x_device_s *dev,
                   uint8_t regindex,
                   uint8_t regsize,
                   uint16_t size,
                   uint8_t lastop);

void qcc74x_pka_write(struct qcc74x_device_s *dev,
                    uint8_t regindex,
                    uint8_t regsize,
                    const uint32_t *data,
                    uint16_t size,
                    uint8_t lastop);

void qcc74x_pka_read(struct qcc74x_device_s *dev,
                   uint8_t regindex,
                   uint8_t regsize,
                   uint32_t *data,
                   uint16_t size);

void qcc74x_pka_gf2mont(struct qcc74x_device_s *dev,
                      uint8_t s_regindex,
                      uint8_t s_regsize,
                      uint8_t d_regindex,
                      uint8_t d_regsize,
                      uint8_t t_regindex,
                      uint8_t t_regsize,
                      uint8_t p_regindex,
                      uint8_t p_regsize,
                      uint32_t size);

void qcc74x_pka_mont2gf(struct qcc74x_device_s *dev,
                      uint8_t s_regindex,
                      uint8_t s_regsize,
                      uint8_t d_regindex,
                      uint8_t d_regsize,
                      uint8_t invt_regindex,
                      uint8_t invt_regsize,
                      uint8_t t_regindex,
                      uint8_t t_regsize,
                      uint8_t p_regindex,
                      uint8_t p_regsize);

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

/**
  * @}
  */

#endif