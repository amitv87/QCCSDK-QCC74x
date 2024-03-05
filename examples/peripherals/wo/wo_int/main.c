#include "qcc74x_mtimer.h"
#include "board.h"
#include "qcc74x_wo.h"

static uint16_t data_write_arr[256];

struct qcc74x_device_s *wo;

void wo_isr(int irq, void *arg)
{
    uint32_t int_status = qcc74x_wo_get_int_status(wo);

    if (int_status & WO_INT_END) {
        printf("interrupt end!\n");
        qcc74x_wo_int_clear(wo, WO_INT_END);
    }
    if (int_status & WO_INT_FIFO) {
        printf("interrupt fifo!\n");
        qcc74x_wo_int_clear(wo, WO_INT_FIFO | WO_INT_FER);
        qcc74x_wo_disable(wo);
        qcc74x_wo_int_mask(wo, WO_INT_FIFO);
    }
    if (int_status & WO_INT_FER) {
        printf("interrupt fer!\n");
        qcc74x_wo_int_clear(wo, WO_INT_FER);
    }
}

struct qcc74x_wo_cfg_s cfg = {
    .code_total_cnt = 10,
    .code0_first_cnt = 6,
    .code1_first_cnt = 2,
    .code0_first_level = 1,
    .code1_first_level = 1,
    .idle_level = 0,
    .fifo_threshold = 16,
    .mode = WO_MODE_WRITE,
};

int main(void)
{
    board_init();
    wo = qcc74x_device_get_by_name("wo");
    qcc74x_wo_init(wo, &cfg);
    qcc74x_wo_int_unmask(wo, WO_INT_END);
    qcc74x_wo_int_mask(wo, WO_INT_FIFO | WO_INT_FER);
    qcc74x_irq_attach(wo->irq_num, wo_isr, NULL);
    qcc74x_irq_enable(wo->irq_num);

    qcc74x_wo_disable(wo);
    qcc74x_wo_push_fifo(wo, data_write_arr, 5);
    qcc74x_wo_enable(wo);
    qcc74x_mtimer_delay_us(50);

    qcc74x_wo_disable(wo);
    qcc74x_wo_push_fifo(wo, data_write_arr, 128);
    qcc74x_wo_int_mask(wo, WO_INT_END);
    qcc74x_wo_int_unmask(wo, WO_INT_FIFO | WO_INT_FER);
    qcc74x_wo_enable(wo);
    qcc74x_mtimer_delay_us(200);
    for (uint32_t i = 0; i < 130; i++) {
        *(volatile uint32_t *)0x20000B04 = i;
    }
}
