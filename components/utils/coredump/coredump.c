#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "qcc74x_flash.h"
#include "coredump.h"

uint8_t core_build_id[BUILD_ID_LEN];

static size_t get_size_of_sections(void) {
    size_t dump_sections_size = 0;

    for (int i = 0; (&_dump_sections + i)->addr != 0xffffffff; i++) {
        dump_sections_size += (&_dump_sections + i)->len;
    }
    return dump_sections_size;
}

static void print_build_id_fmt(uint8_t *build_id)
{
    printf("\r\ncheck build_id with `riscv64-unknown-elf-readelf -n *.elf` output \r\n");
    printf( \
           "build_id: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x" \
           "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n" \
           ,(build_id)[0], (build_id)[1], (build_id)[2], (build_id)[3], (build_id)[4] \
           ,(build_id)[5], (build_id)[6], (build_id)[7], (build_id)[8], (build_id)[9] \
           ,(build_id)[10], (build_id)[11], (build_id)[12], (build_id)[13], (build_id)[14] \
           ,(build_id)[15], (build_id)[16], (build_id)[17], (build_id)[18], (build_id)[19]);
}

static void coredump_print_n_k(uintptr_t addr, uintptr_t lma_addr, uint32_t len, const char *desc)
{
    uint32_t printed_len;
    uint32_t seg;
#define COREDUMP_PRINT_SEG_N_K 1
    for(seg = 0; seg * COREDUMP_PRINT_SEG_N_K * 1024 < len; seg++) {
        printed_len = seg*COREDUMP_PRINT_SEG_N_K*1024;
        coredump_print(addr + printed_len, lma_addr + printed_len, len - printed_len >= COREDUMP_PRINT_SEG_N_K*1024 ? COREDUMP_PRINT_SEG_N_K*1024: len - printed_len, desc);
    }
}

/**
 * Coredump initialize.
 *
 * @return result
 */
static uint32_t coredump_flash_addr;
static size_t coredump_flash_size;
void core_partition_init(uint32_t flash_addr, size_t flash_size) {
    coredump_flash_addr = flash_addr;
    coredump_flash_size = flash_size;
}

/* write xip flash buffer to flash */
int coredump_xip_flash_write(uint32_t lma, uint8_t *lma_xip, size_t len)
{
    uint8_t write_buf[32];
    size_t rest = len;
    size_t buf_len;
    int ret = 0;

    while(rest > 0) {
        buf_len = sizeof(write_buf) > rest ? rest : sizeof(write_buf);
        memcpy(write_buf, lma_xip + (len - rest), buf_len);
        ret = qcc74x_flash_write(lma + (len - rest), (uint8_t *)write_buf, buf_len);
        if(ret)
            break;
        rest -= buf_len;
    }

    return ret;
}

void coredump_run(void) {
    bool coredump_flash_disable = 0;
    uint32_t lma = coredump_flash_addr;

    if(coredump_flash_size < get_size_of_sections() + 4096) {
        coredump_flash_disable = 1;
    } else {
        /* erase flash sector first */
        qcc74x_flash_erase(lma, coredump_flash_size);
        core_bin_start_hook(&lma, coredump_flash_size);
    }
    memcpy(core_build_id, elf_build_id, BUILD_ID_LEN);
    print_build_id_fmt(elf_build_id);

    /* Dump all pre-defined memory region by default */
    for (int i = 0; (&_dump_sections + i)->addr != 0xffffffff; i++) {
        if ((&_dump_sections + i)->addr == 0)
            break;
        if ((&_dump_sections + i)->len == 0)
            continue;
        coredump_print_n_k((uintptr_t)(&_dump_sections + i)->addr, (uintptr_t)(&_dump_sections + i)->addr, (&_dump_sections + i)->len, "predefined");
        if(!coredump_flash_disable) {
            core_bin_sections_hook(&lma, (uint8_t *)(&_dump_sections + i)->addr, (&_dump_sections + i)->len);
        }
    }

    if(!coredump_flash_disable) {
        core_bin_end_hook(coredump_flash_addr);
    }
    while (1) {
        asm ("nop");
    }
}