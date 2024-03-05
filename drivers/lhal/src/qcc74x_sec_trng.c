#include "qcc74x_sec_trng.h"
#include "hardware/sec_eng_reg.h"

#define QCC74x_PUT_LE32TOBYTES(p, val) \
    {                                \
        p[0] = val & 0xff;           \
        p[1] = (val >> 8) & 0xff;    \
        p[2] = (val >> 16) & 0xff;   \
        p[3] = (val >> 24) & 0xff;   \
    }

#if defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undefL)
#define QCC74x_SEC_ENG_BASE ((uint32_t)0x40004000)
#elif defined(QCC743) || defined(QCC74x_undefP) || defined(QCC74x_undef) || defined(QCC74x_undef)
#define QCC74x_SEC_ENG_BASE ((uint32_t)0x20004000)
#endif

int qcc74x_trng_read(struct qcc74x_device_s *dev, uint8_t data[32])
{
#ifdef romapi_qcc74x_trng_read
    return romapi_qcc74x_trng_read(dev, data);
#else
    uint32_t regval;
    uint32_t reg_base;
    uint64_t start_time;
    uint8_t *p = (uint8_t *)data;

    reg_base = QCC74x_SEC_ENG_BASE;

    /* enable trng */
    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_EN;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_INT_CLR_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    /* busy will be set to 1 after trigger, the gap is 1T */
    __ASM volatile("nop");
    __ASM volatile("nop");
    __ASM volatile("nop");
    __ASM volatile("nop");

    start_time = qcc74x_mtimer_get_time_ms();
    while (getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET) & SEC_ENG_SE_TRNG_0_BUSY) {
        if ((qcc74x_mtimer_get_time_ms() - start_time) > 100) {
            return -ETIMEDOUT;
        }
    }

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_INT_CLR_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_TRIG_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    /* busy will be set to 1 after trigger, the gap is 1T */
    __ASM volatile("nop");
    __ASM volatile("nop");
    __ASM volatile("nop");
    __ASM volatile("nop");

    start_time = qcc74x_mtimer_get_time_ms();
    while (getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET) & SEC_ENG_SE_TRNG_0_BUSY) {
        if ((qcc74x_mtimer_get_time_ms() - start_time) > 100) {
            return -ETIMEDOUT;
        }
    }

    /* copy trng value */
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_0_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_1_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_2_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_3_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_4_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_5_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_6_OFFSET));
    p += 4;
    QCC74x_PUT_LE32TOBYTES(p, getreg32(reg_base + SEC_ENG_SE_TRNG_0_DOUT_7_OFFSET));
    p += 4;

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval &= ~SEC_ENG_SE_TRNG_0_TRIG_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_DOUT_CLR_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval &= ~SEC_ENG_SE_TRNG_0_DOUT_CLR_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    /* disable trng */
    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval &= ~SEC_ENG_SE_TRNG_0_EN;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    regval = getreg32(reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);
    regval |= SEC_ENG_SE_TRNG_0_INT_CLR_1T;
    putreg32(regval, reg_base + SEC_ENG_SE_TRNG_0_CTRL_0_OFFSET);

    return 0;
#endif
}

int qcc74x_trng_readlen(uint8_t *data, uint32_t len)
{
    uint8_t tmp_buf[32];
    uint32_t readlen = 0;
    uint32_t i = 0, cnt = 0;

    while (readlen < len) {
        if (qcc74x_trng_read(NULL, tmp_buf) != 0) {
            return -ETIMEDOUT;
        }

        cnt = len - readlen;

        if (cnt > sizeof(tmp_buf)) {
            cnt = sizeof(tmp_buf);
        }

        for (i = 0; i < cnt; i++) {
            data[readlen + i] = tmp_buf[i];
        }

        readlen += cnt;
    }

    return 0;
}

__WEAK long random(void)
{
    uint32_t data[8];

    qcc74x_trng_read(NULL, (uint8_t *)data);

    return data[0];
}

void qcc74x_group0_request_trng_access(struct qcc74x_device_s *dev)
{
#ifdef romapi_qcc74x_group0_request_trng_access
    romapi_qcc74x_group0_request_trng_access(dev);
#else
    uint32_t regval;
    uint32_t reg_base;

    reg_base = QCC74x_SEC_ENG_BASE;

    regval = getreg32(reg_base + SEC_ENG_SE_CTRL_PROT_RD_OFFSET);
    if (((regval >> 4) & 0x03) == 0x03) {
        putreg32(0x04, reg_base + SEC_ENG_SE_TRNG_0_CTRL_PROT_OFFSET);

        regval = getreg32(reg_base + SEC_ENG_SE_CTRL_PROT_RD_OFFSET);
        if (((regval >> 4) & 0x03) == 0x01) {
        }
    }
#endif
}

void qcc74x_group0_release_trng_access(struct qcc74x_device_s *dev)
{
#ifdef romapi_qcc74x_group0_release_trng_access
    romapi_qcc74x_group0_release_trng_access(dev);
#else
    uint32_t reg_base;

    reg_base = QCC74x_SEC_ENG_BASE;

    putreg32(0x06, reg_base + SEC_ENG_SE_TRNG_0_CTRL_PROT_OFFSET);
#endif
}