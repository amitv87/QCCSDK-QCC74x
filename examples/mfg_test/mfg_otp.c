#include <stdint.h>
#include "mfg_otp.h"
#include "qcc74x_efuse.h"
#include "qcc74x_ef_ctrl.h"

#define OTP_SIM                         0
#define OTP_DEBUG(...)                  //printf(__VA_ARGS__)
#define OTP_ERROR(...)                  printf(__VA_ARGS__)
#define OTP_ADDR_START                  0x100
#define OTP_ADDR_END                    0x1E0
#define OTP_PN_ADDR_START               0x100
#define OTP_PN_ADDR_END                 0x114
#define OTP_QC_PN_LEN                   (24)

#if OTP_SIM
static uint32_t otp_memory[256]; // In-memory simulation of OTP values
#endif
struct qcc74x_device_s *efuse_dev;
static qcc74x_efuse_device_info_type device_info;
static uint8_t chip_id[8];

// OTP initialization function to set read and write functions
void otp_init()
{
#if OTP_SIM
    for (int i = 0; i < sizeof(otp_memory)/sizeof(otp_memory[0]); i++) {
        otp_memory[i] = 0;
    }
    // QCOMRFIQD0744010NB001
    // otp_memory[(OTP_PN_ADDR_START) >> 2] |= 0x4d4f4351;
    // otp_memory[(OTP_PN_ADDR_START + 4) >> 2] |= 0x51494652;
    // otp_memory[(OTP_PN_ADDR_START + 8) >> 2] |= 0x34373044;
    // otp_memory[(OTP_PN_ADDR_START + 12) >> 2] |= 0x30313034;
    // otp_memory[(OTP_PN_ADDR_START + 16) >> 2] |= 0x3030424e;
    // otp_memory[(OTP_PN_ADDR_START + 20) >> 2] |= 0x00000331;
#endif

    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");
    if (NULL == efuse_dev) {
        printf("efuse device driver not found!\n");
        return;
    }

    qcc74x_efuse_get_device_info(&device_info);
    qcc74x_efuse_get_chipid(chip_id);

#if 1
    /* printf device_info */
    printf("DEVICE_INFO:\r\n");
    printf("    PACKAGE: %s\r\n", device_info.package_name);
#if !defined(QCC74x_undef)
    printf("    PSRAM:   %s\r\n", device_info.psram_info_name);
#endif
    printf("    FLASH:   %s\r\n", device_info.flash_info_name);
#if !defined(QCC74x_undef) && !defined(QCC74x_undefL)
    printf("    VERSION: %d\r\n", device_info.version);
#endif
    printf("CHIP_ID:\r\n   ");
    for (int i = 0; i < sizeof(chip_id) / sizeof(chip_id[0]) - 2; i++) {
        printf(" %02X", chip_id[i]);
    }
    printf("\r\n");
#endif
}

int otp_get_soc_signature(uint32_t *value)
{
    if (NULL == efuse_dev) {
        return -1;
    }

    *value = device_info.version << 29;
    *value |= device_info.flash_info << 26;
    *value |= device_info.psram_info << 24;
    *value |= device_info.package << 22;
    *value |= (chip_id[3] << 16) & ((1 << 22) - 1);
    *value |= (chip_id[4] << 8 ) & ((1 << 22) - 1);
    *value |= (chip_id[5] << 0 ) & ((1 << 22) - 1);
    return 0;
}

static int otp_address_check(uint32_t addr)
{
    if ((addr & 0x3) != 0) {
        OTP_ERROR("Error address: 0x%x\n", addr);
        return -1;
    }

    if (!(addr >= OTP_ADDR_START && addr <= OTP_ADDR_END)) {
        OTP_ERROR("Error address: 0x%x\n", addr);
        return -1;
    }

    return 0;
}

static int otp_read32(uint32_t addr, uint32_t *value)
{
    // Check if otp address is aligned by 4 bytes and in range
    if (otp_address_check(addr)) {
        return -1;
    }

#if OTP_SIM
    *value = otp_memory[addr >> 2];
#else
    qcc74x_ef_ctrl_read_direct(efuse_dev, addr, value, 1, 1);
    OTP_DEBUG("otp_read32 efuse_dev: %p\n", efuse_dev);
    OTP_DEBUG("otp_read32 addr: 0x%x\n", addr);
    OTP_DEBUG("otp_read32 value: 0x%x\n", value[0]);
#endif
    return 0;
}

static int otp_write32(uint32_t addr, uint32_t value)
{
    // Check if otp address is aligned by 4 bytes and in range
    if (otp_address_check(addr)) {
        OTP_ERROR("Address not 4 bytes aligned\n");
        return -1;
    }

#if OTP_SIM
    otp_memory[addr >> 2] |= value;
#else
    OTP_DEBUG("otp_write32 efuse_dev: %p\n", efuse_dev);
    OTP_DEBUG("otp_write32 addr: 0x%x\n", addr);
    OTP_DEBUG("otp_write32 value: 0x%x\n", value);
    qcc74x_ef_ctrl_write_direct(efuse_dev, addr, &value, 1, 1);
#endif
    return 0;
}

static int otp_read8(uint32_t addr, uint8_t *value)
{
    uint32_t word_addr = addr & ~0x3; // Align to the nearest lower word address
    uint32_t word;
    
    // Check if the address is in the valid range
    if (!(addr >= OTP_ADDR_START && addr <= OTP_ADDR_END)) {
        OTP_ERROR("Error address: 0x%x\n", addr);
        return -1;
    }

    // Read the 32-bit word containing the byte
    if (otp_read32(word_addr, &word) != 0) {
        return -1;
    }

    // Extract the byte from the word
    int byte_offset = addr & 0x3; // The offset within the word (0, 1, 2, 3)
    *value = (word >> (byte_offset * 8)) & 0xFF;

    return 0;
}

static int otp_write8(uint32_t addr, uint8_t value)
{
    uint32_t word_addr = addr & ~0x3; // Align to the nearest lower word address
    uint32_t word;
    
    // Check if the address is in the valid range
    if (!(addr >= OTP_ADDR_START && addr <= OTP_ADDR_END)) {
        OTP_ERROR("Error address: 0x%x\n", addr);
        return -1;
    }

    // Read the 32-bit word containing the byte
    if (otp_read32(word_addr, &word) != 0) {
        return -1;
    }

    // Modify the specific byte within the word
    int byte_offset = addr & 0x3; // The offset within the word (0, 1, 2, 3)
    uint32_t mask = 0xFF << (byte_offset * 8); // Mask to clear the byte
    word = (word & ~mask) | (value << (byte_offset * 8));

    // Write the modified word back to the OTP
    if (otp_write32(word_addr, word) != 0) {
        return -1;
    }

    return 0;
}

static const qcc74x_manufacture_otp_struct otp_table[] = {
    {"ID",          0x100, otp_write8,  otp_read8 },
    {"MREL",        0x101, otp_write8,  otp_read8 },
    {"MWL",         0x102, otp_write8,  otp_read8 },
    {"MIS",         0x103, otp_write8,  otp_read8 },
    {"MIDPRESENT1", 0x104, otp_write8,  otp_read8 },
    {"MIDPRESENT2", 0x105, otp_write8,  otp_read8 },
    {"MIDPRESENT3", 0x106, otp_write8,  otp_read8 },
    {"CRC",         0x107, otp_write8,  otp_read8 },
    {"FWREL",       0x108, otp_write32, otp_read32},
    {"FWRELRT",     0x10c, otp_write32, otp_read32},
    {"MDT",         0x110, otp_write8,  otp_read8 },
    {"QC",          0x111, otp_write8,  otp_read8 },
    {"MDTRT",       0x112, otp_write8,  otp_read8 },
    {"QCRT",        0x113, otp_write8,  otp_read8 },
};

const qcc74x_manufacture_otp_struct* otp_find_by_name(const char *name)
{
    const int OTP_TABLE_SIZE = (sizeof(otp_table) / sizeof(otp_table[0]));

    for (int i = 0; i < OTP_TABLE_SIZE; ++i) {
        if (strcmp(otp_table[i].otp_name, name) == 0) {
            return &otp_table[i];
        }
    }
    return NULL;
}

int otp_write(const qcc74x_manufacture_otp_struct* otp_field, uint32_t value)
{
    if (otp_field == NULL || otp_field->otp_write_func == NULL) {
        return -1;
    }

    OTP_DEBUG("OTP Write: %s, 0x%x, %p, %p\n", 
        otp_field->otp_name, otp_field->otp_addr, 
        otp_field->otp_write_func, otp_field->otp_read_func);

#if OTP_SIM
    int ret = otp_field->otp_write_func(otp_field->otp_addr, value);
    for (int i = OTP_ADDR_START; i <= OTP_ADDR_END; i += 4) {
        OTP_DEBUG("%04x: %08x\n", i, otp_memory[i >> 2]);
    }
    return ret;
#else
    return otp_field->otp_write_func(otp_field->otp_addr, value);
#endif
}

int otp_read(const qcc74x_manufacture_otp_struct* otp_field, uint32_t *value)
{
    if (otp_field == NULL || otp_field->otp_read_func == NULL) {
        return -1;
    }

    // Clear value is important
    *value = 0;

    OTP_DEBUG("OTP Read: %s, 0x%x, %p, %p\n", 
        otp_field->otp_name, otp_field->otp_addr, 
        otp_field->otp_write_func, otp_field->otp_read_func);

#if OTP_SIM
    return otp_field->otp_read_func(otp_field->otp_addr, value);
#else
    return otp_field->otp_read_func(otp_field->otp_addr, value);
#endif
}

int otp_get_mac_addr(uint8_t mac_0[6], uint8_t mac_1[6], uint8_t mac_2[6])
{
    int ret_0,ret_1,ret_2;

    if (NULL == efuse_dev) {
        return -1;
    }

    ret_0 = qcc74x_efuse_read_mac_address_opt(0, mac_0, 1);
    ret_1 = qcc74x_efuse_read_mac_address_opt(1, mac_1, 1);
    ret_2 = qcc74x_efuse_read_mac_address_opt(2, mac_2, 1);

    if ((ret_0 == 0) && (ret_1 == 0) && (ret_2 == 0)) {
        return -1;
    } else {
        return 0;
    }
}

int otp_get_part_number(uint32_t *value)
{
    if (NULL == efuse_dev) {
        return -1;
    }

    for (int i = 0; i < (OTP_QC_PN_LEN / 4); i++){
        if (otp_read32(OTP_PN_ADDR_START + 4*i, &(value[i])) != 0) {
            return -1;
        }
    }

    return 0;
}

int otp_set_part_number(uint32_t *value)
{
    OTP_DEBUG("efuse_dev: 0x%p\n", efuse_dev);

    if (NULL == efuse_dev) {
        return -1;
    }

    for (int i = 0; i < (OTP_QC_PN_LEN / 4); i++){
        OTP_DEBUG("otp_set_part_number value : 0x%x\n", value[i]);
        if (otp_write32(OTP_PN_ADDR_START + 4*i, value[i]) != 0) {
            return -1;
        }
    }

    return 0;
}
