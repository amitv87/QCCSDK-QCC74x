#include "qcc74x_platform_dma.h"
#include "qcc74x_l1c.h"
#include "hardware/platform_dma_reg.h"

struct qcc74x_platform_dma_env_tag dma_env;

void qcc74x_platform_dma_init(struct qcc74x_device_s *dev)
{
    arch_memset(dma_env.last_dma, 0, sizeof(dma_env.last_dma));

    // configure the channel priorities
    putreg32(0x0C, dev->reg_base + DMA_ARBITRATION_OFFSET);

    putreg32(0x04, dev->reg_base + 0x44);

    // configure the interrupts (enable all interrupts)
    putreg32(DMA_ERROR_BIT | DMA_LLI_IRQ_MASK, dev->reg_base + DMA_INT_UNMASK_SET_OFFSET);
}

void qcc74x_platform_dma_push(struct qcc74x_device_s *dev, struct qcc74x_platform_dma_desc *first, struct qcc74x_platform_dma_desc *last)
{
    uint32_t value;
    uintptr_t flag;
    uint32_t channel_idx;

    channel_idx = dev->idx;

    // current DMA descriptor is pushed at the end of the list
    last->next = 0;

    // prevent accesses to the same DMA channel from both BK and IRQ simultaneously
    flag = qcc74x_irq_save();

    // set the MUTEX
    putreg32(1 << channel_idx, dev->reg_base + DMA_CHANNEL_MUTEX_SET_OFFSET);

    // read the root in the DMA HW
    if (channel_idx == 4) {
        value = getreg32(dev->reg_base + DMA_CH4_LLI_ROOT_OFFSET);
    } else {
        value = getreg32(dev->reg_base + DMA_CH_LLI_ROOT_OFFSET + channel_idx * 4);
    }

    // check if the root is empty
    if (value) {
        // append the descriptor to the last LLI of the list
        dma_env.last_dma[channel_idx]->next = (uint32_t)first;

        // clear the MUTEX
        putreg32(1 << channel_idx, dev->reg_base + DMA_CHANNEL_MUTEX_CLEAR_OFFSET);
    } else {
        // write directly the DMA to the root
        if (channel_idx == 4) {
            putreg32((uint32_t)first, dev->reg_base + DMA_CH4_LLI_ROOT_OFFSET);
        } else {
            putreg32((uint32_t)first, dev->reg_base + DMA_CH_LLI_ROOT_OFFSET + channel_idx * 4);
        }
    }

    // save the new last LLI descriptor on this channel
    dma_env.last_dma[channel_idx] = last;

    qcc74x_irq_restore(flag);
}

void qcc74x_platform_dma_buserr_isr(struct qcc74x_device_s *dev)
{
    // For now consider this error as a fatal one
    while (1) {
    }
}

void qcc74x_platform_dma_wait_eot(struct qcc74x_device_s *dev)
{
    uint32_t channel_idx;

    channel_idx = dev->idx;
    uint32_t stopped = 0;
    if (channel_idx < 5) {
        do {
            stopped = getreg32(dev->reg_base + DMA_INT_RAWSTATUS_OFFSET);
            stopped = (stopped >> (20 + channel_idx)) & 0x01;
        } while (!stopped);
    }
}

void qcc74x_platform_dma_clear_eot(struct qcc74x_device_s *dev)
{
    uint32_t channel_idx;

    channel_idx = dev->idx;
    if (channel_idx < 5) {
        putreg32(1 << (20 + channel_idx), dev->reg_base + DMA_INT_ACK_OFFSET);
    }
}