#include <stdint.h>

#define __STARTUP_CLEAR_BSS      1
#define __STARTUP_CLEAR_WIFI_BSS 1

/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __itcm_load_addr;
extern uint32_t __dtcm_load_addr;
extern uint32_t __ram_load_addr;
extern uint32_t __nocache_ram_load_addr;
extern uint32_t __psram_load_addr;

extern uint32_t __text_code_start__;
extern uint32_t __text_code_end__;
extern uint32_t __tcm_code_start__;
extern uint32_t __tcm_code_end__;
extern uint32_t __tcm_data_start__;
extern uint32_t __tcm_data_end__;
extern uint32_t __ram_data_start__;
extern uint32_t __ram_data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __wifi_bss_start;
extern uint32_t __wifi_bss_end;
extern uint32_t __noinit_data_start__;
extern uint32_t __noinit_data_end__;
extern uint32_t __psram_data_start__;
extern uint32_t __psram_data_end__;
#ifndef CONIFG_DISABLE_NOCACHE_RAM_LOAD
extern uint32_t __nocache_ram_data_start__;
extern uint32_t __nocache_ram_data_end__;
#endif

extern uint32_t __StackTop;
extern uint32_t __StackLimit;
extern uint32_t __HeapBase;
extern uint32_t __HeapLimit;

void start_load(void)
{
    uint32_t *tmp_src, *tmp_dst;
    uint32_t *pTable __attribute__((unused));

    /* Copy ITCM code */
    tmp_src = &__itcm_load_addr;
    tmp_dst = &__tcm_code_start__;

    for (; tmp_dst < &__tcm_code_end__;) {
        *tmp_dst++ = *tmp_src++;
    }

    /* Copy DTCM code */
    tmp_src = &__dtcm_load_addr;
    tmp_dst = &__tcm_data_start__;

    for (; tmp_dst < &__tcm_data_end__;) {
        *tmp_dst++ = *tmp_src++;
    }

    /* Add OCARAM data copy */
    tmp_src = &__ram_load_addr;
    tmp_dst = &__ram_data_start__;

    for (; tmp_dst < &__ram_data_end__;) {
        *tmp_dst++ = *tmp_src++;
    }

    /* Add psram data copy */
    tmp_src = &__psram_load_addr;
    tmp_dst = &__psram_data_start__;

    for (; tmp_dst < &__psram_data_end__;) {
        *tmp_dst++ = *tmp_src++;
    }

#ifndef CONIFG_DISABLE_NOCACHE_RAM_LOAD
    /* Add no cache ram data copy */
    tmp_src = &__nocache_ram_load_addr;
    tmp_dst = &__nocache_ram_data_start__;

    for (; tmp_dst < &__nocache_ram_data_end__;) {
        *tmp_dst++ = *tmp_src++;
    }
#endif

#if LP_APP && LP_RAM_REUSE
    extern uint32_t __ram_lp_code_start__;
    extern uint32_t __ram_lp_code_end__;
    extern uint32_t __ram_lp_load_addr;

    tmp_src = &__ram_lp_load_addr;
    tmp_dst = &__ram_lp_code_start__;

    for (; tmp_dst < &__ram_lp_code_end__;) {
        *tmp_dst++ = *tmp_src++;
    }
#endif

#ifdef __STARTUP_CLEAR_BSS
    tmp_dst = &__bss_start__;
    for (; tmp_dst < &__bss_end__;) {
        *tmp_dst++ = 0ul;
    }

#endif
#ifdef __STARTUP_CLEAR_WIFI_BSS
    tmp_dst = &__wifi_bss_start;

    for (; tmp_dst < &__wifi_bss_end;) {
        *tmp_dst++ = 0ul;
    }

#endif
}
