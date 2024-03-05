#include "board.h"
#include "qcc74x_mtimer.h"
#include "qcc74x_platform_dma.h"
#include "qcc74x_l1c.h"

#define DMA_BUFFER_LENGTH (4096)
#define DMA_BUFFER_OFFSET_SRC (1)
#define DMA_BUFFER_OFFSET_DST (1)
//ATTR_NOCACHE_NOINIT_RAM_SECTION
ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src1_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_SRC];
ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src2_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_SRC];
ATTR_NOCACHE_NOINIT_RAM_SECTION uint8_t src3_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_SRC];

ATTR_NOINIT_PSRAM_SECTION uint8_t dst1_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_DST];
ATTR_NOINIT_PSRAM_SECTION uint8_t dst2_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_DST];
ATTR_NOINIT_PSRAM_SECTION uint8_t dst3_buffer[DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_DST];

ATTR_WIFI_RAM_SECTION struct qcc74x_platform_dma_desc dmslist[4];

static void sram_init()
{
    uint32_t i;

    for (i = 0; i < DMA_BUFFER_LENGTH + DMA_BUFFER_OFFSET_SRC; i++) {
        src1_buffer[i] = i & 0xff;
        src2_buffer[i] = (i * 0x07) & 0xff;
        src3_buffer[i] = (i * 0x0b) & 0xff;
    }

    memset(dst1_buffer, 0, sizeof(dst1_buffer));
    memset(dst2_buffer, 0, sizeof(dst2_buffer));
    memset(dst3_buffer, 0, sizeof(dst3_buffer));

    qcc74x_l1c_dcache_clean_range(src1_buffer, sizeof(src1_buffer));
    qcc74x_l1c_dcache_clean_range(src2_buffer, sizeof(src2_buffer));
    qcc74x_l1c_dcache_clean_range(src3_buffer, sizeof(src3_buffer));
    qcc74x_l1c_dcache_clean_range(dst1_buffer, sizeof(dst1_buffer));
    qcc74x_l1c_dcache_clean_range(dst2_buffer, sizeof(dst2_buffer));
    qcc74x_l1c_dcache_clean_range(dst3_buffer, sizeof(dst3_buffer));
}

int main(void)
{
    uint32_t i;
    struct qcc74x_device_s *dma_chx;
    uint64_t start_time = 0;

    board_init();
    sram_init();

    dmslist[0].src = (uint32_t)(src1_buffer + DMA_BUFFER_OFFSET_SRC);
    dmslist[0].dest = (uint32_t)(dst1_buffer + DMA_BUFFER_OFFSET_DST);
    dmslist[0].length = DMA_BUFFER_LENGTH;
    dmslist[0].ctrl = 0;
    dmslist[0].next = (uint32_t)&dmslist[1];

    dmslist[1].src = (uint32_t)(src2_buffer + DMA_BUFFER_OFFSET_SRC);
    dmslist[1].dest = (uint32_t)(dst2_buffer + DMA_BUFFER_OFFSET_DST);
    dmslist[1].length = DMA_BUFFER_LENGTH;
    dmslist[1].ctrl = 0;
    dmslist[1].next = (uint32_t)&dmslist[2];

    dmslist[2].src = (uint32_t)(src3_buffer + DMA_BUFFER_OFFSET_SRC);
    dmslist[2].dest = (uint32_t)(dst3_buffer + DMA_BUFFER_OFFSET_DST);
    dmslist[2].length = DMA_BUFFER_LENGTH;
    dmslist[2].ctrl = 0;
    dmslist[2].next = 0;

    printf("platform dma memory case:\r\n");
    printf("src1=%08x,dst1=%08x,len=%d\r\n", dmslist[0].src, dmslist[0].dest, dmslist[0].length);
    printf("src2=%08x,dst2=%08x,len=%d\r\n", dmslist[1].src, dmslist[1].dest, dmslist[1].length);
    printf("src3=%08x,dst3=%08x,len=%d\r\n", dmslist[2].src, dmslist[2].dest, dmslist[2].length);

    dma_chx = qcc74x_device_get_by_name("plfm_dma_ch2");

    if (dma_chx == NULL) {
        printf("device not found\r\n");
        while (1)
            ;
    }

    start_time = qcc74x_mtimer_get_time_us();

    qcc74x_platform_dma_init(dma_chx);

    qcc74x_platform_dma_push(dma_chx, &dmslist[0], &dmslist[2]);
    //qcc74x_platform_dma_push(dma_chx, &dmslist[1], &dmslist[1]);
    //qcc74x_platform_dma_push(dma_chx, &dmslist[2], &dmslist[2]);

    qcc74x_platform_dma_wait_eot(dma_chx);
    qcc74x_platform_dma_clear_eot(dma_chx);

    printf("copy finished with time=%dus\r\n", (int)(qcc74x_mtimer_get_time_us() - start_time));

    /* Check data */
    qcc74x_l1c_dcache_invalidate_range(src1_buffer, sizeof(src1_buffer));
    qcc74x_l1c_dcache_invalidate_range(src2_buffer, sizeof(src2_buffer));
    qcc74x_l1c_dcache_invalidate_range(src3_buffer, sizeof(src3_buffer));
    qcc74x_l1c_dcache_invalidate_range(dst1_buffer, sizeof(dst1_buffer));
    qcc74x_l1c_dcache_invalidate_range(dst2_buffer, sizeof(dst2_buffer));
    qcc74x_l1c_dcache_invalidate_range(dst3_buffer, sizeof(dst3_buffer));
    for (i = 0; i < DMA_BUFFER_LENGTH; i++) {
        if (src1_buffer[i + DMA_BUFFER_OFFSET_SRC] != dst1_buffer[i + DMA_BUFFER_OFFSET_DST]) {
            printf("Error! index: %ld, src1: 0x%02x, dst1: 0x%02x\r\n", i,
                   src1_buffer[i + DMA_BUFFER_OFFSET_SRC],
                   dst1_buffer[i + DMA_BUFFER_OFFSET_DST]);
        }
        if (src2_buffer[i + DMA_BUFFER_OFFSET_SRC] != dst2_buffer[i + DMA_BUFFER_OFFSET_DST]) {
            printf("Error! index: %ld, src2: 0x%02x, dst2: 0x%02x\r\n", i,
                   src2_buffer[i + DMA_BUFFER_OFFSET_SRC],
                   dst2_buffer[i + DMA_BUFFER_OFFSET_DST]);
        }
        if (src3_buffer[i + DMA_BUFFER_OFFSET_SRC] != dst3_buffer[i + DMA_BUFFER_OFFSET_DST]) {
            printf("Error! index: %ld, src3: 0x%02x, dst3: 0x%02x\r\n", i,
                   src3_buffer[i + DMA_BUFFER_OFFSET_SRC],
                   dst3_buffer[i + DMA_BUFFER_OFFSET_DST]);
        }
    }
    printf("case end\r\n");
    while (1) {
    }
}
