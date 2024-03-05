#include "board.h"
#include "qcc74x_cks.h"
#include "qcc74x_dma.h"
#include "qcc74x_mtimer.h"
#include "qcc74x_core.h"

#define DATA_LEN 512

static volatile uint8_t dma_tc_flag0 = 0;
struct qcc74x_device_s *cks;
struct qcc74x_device_s *dma0_ch0;
struct qcc74x_dma_channel_lli_pool_s lli[20]; /* max trasnfer size 4064 * 20 */

void dma0_ch0_isr(void *arg)
{
    dma_tc_flag0++;
    printf("tc done\r\n");
}

uint16_t sw_chksum(uint8_t *data, uint32_t len) {
  uint32_t sum = 0;
  uint16_t chksum = 0;
  uint32_t size = len;

  if (len % 2 == 1) {
    size = len - 1;
    sum += data[size];
  }

  for (uint32_t i = 0; i < size; i = i + 2) {
    sum += ((uint32_t)data[i]);
    sum += ((uint32_t)data[i + 1] << 8);
  }

  while (sum >> 16) {
    sum = (sum >> 16) + (sum & 0x0000FFFF);
  }

  chksum = (uint16_t)sum;
  return ~chksum;
}

uint16_t get_cks_with_dma(uint8_t* data,uint32_t length)
{
    uint16_t checksum = 0;

    struct qcc74x_dma_channel_lli_transfer_s transfers[1];

    transfers[0].src_addr = (uint32_t)data;
    transfers[0].dst_addr = (uint32_t)(cks->reg_base + 0x4);
    transfers[0].nbytes = length;

    qcc74x_dma_channel_lli_reload(dma0_ch0, lli, 20, transfers, 1);
    qcc74x_dma_channel_start(dma0_ch0);

    while(dma_tc_flag0 == 0) {
    }
    dma_tc_flag0 = 0;
    
    checksum = qcc74x_cks_compute(cks, data, 0);

    return checksum;
}

static void test_case1(void){
    uint16_t dma_cks = 0;
    uint16_t hw_cks = 0;
    uint16_t sw_cks = 0;
    uint32_t time = 0, i;
    struct qcc74x_dma_channel_config_s config;
    
    uint32_t data_src1[DATA_LEN/4];
    
    for(i = 0;i < DATA_LEN; i++){
        ((uint8_t *)data_src1)[i] = i & 0xff;
    }
    
    time = (unsigned int)qcc74x_mtimer_get_time_us();
    
    sw_cks = sw_chksum((uint8_t *)data_src1, sizeof(data_src1));
    
    printf("software checksum time=%ldus\r\n", (unsigned int)qcc74x_mtimer_get_time_us() - time);
    
    printf("sw_cks is %04x\r\n", sw_cks);
    
    qcc74x_cks_reset(cks);
    qcc74x_cks_set_endian(cks, CKS_BIG_ENDIAN);
    
    time = (unsigned int)qcc74x_mtimer_get_time_us();
    
    hw_cks = qcc74x_cks_compute(cks, (uint8_t *)data_src1, sizeof(data_src1));
    
    printf("hardware checksum time=%ldus\r\n", (unsigned int)qcc74x_mtimer_get_time_us() - time);
    
    printf("hw_cks is %04x\r\n", hw_cks);
    
    qcc74x_cks_reset(cks);
    qcc74x_cks_set_endian(cks, CKS_BIG_ENDIAN);
    
    printf("\r\ndma case 1:\n");
    
    config.direction = DMA_MEMORY_TO_MEMORY;
    config.src_req = 0;
    config.dst_req = 0;
    config.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    config.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    config.src_burst_count = DMA_BURST_INCR1;
    config.dst_burst_count = DMA_BURST_INCR1;
    config.src_width = DMA_DATA_WIDTH_8BIT;
    config.dst_width = DMA_DATA_WIDTH_8BIT;
    qcc74x_dma_channel_init(dma0_ch0, &config);
    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    
    time = (unsigned int)qcc74x_mtimer_get_time_us();
    
    dma_cks = get_cks_with_dma((uint8_t *)data_src1, sizeof(data_src1));
    printf("dma checksum time=%ldus\r\n", (unsigned int)qcc74x_mtimer_get_time_us() - time);
    printf("dma_cks is %04x\r\n", dma_cks);
    
    qcc74x_cks_reset(cks);
    qcc74x_cks_set_endian(cks, CKS_BIG_ENDIAN);
    
    printf("\r\ndma case 2:\n");
    config.src_width = DMA_DATA_WIDTH_32BIT;
    config.src_burst_count = DMA_BURST_INCR4;
    config.dst_width = DMA_DATA_WIDTH_8BIT;
    config.dst_burst_count = DMA_BURST_INCR16;
    time = (unsigned int)qcc74x_mtimer_get_time_us();
    
    dma_cks = get_cks_with_dma((uint8_t *)data_src1, sizeof(data_src1));
    printf("dma checksum time=%ldus\r\n", (unsigned int)qcc74x_mtimer_get_time_us() - time);
    printf("dma_cks is %04x\r\n", dma_cks);
}

/* main */
int main(void)
{
    board_init();
    
    printf("CKS dma case:\r\n");

    cks = qcc74x_device_get_by_name("cks");
    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");

    test_case1();

    printf("\r\nend\r\n");

    while (1) {
    }
}
