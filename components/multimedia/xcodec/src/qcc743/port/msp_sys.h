#ifndef __MSP_SYS_H__
#define __MSP_SYS_H__

#include <msp_port.h>

#define MSP_AUDIO_IRQn         (20) //qcc743
#define MSP_DMA_IRQn           (31) //qcc743

#define msp_cache_flush  csi_dcache_clean_range         //(uint32_t*addr, uint32_t len)
#define msp_cache_remove csi_dcache_clean_invalid_range //(uint32_t*addr, uint32_t len)

// audio config
void msp_config_audiopll(void);

#endif
