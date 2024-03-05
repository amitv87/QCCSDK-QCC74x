DAC - dma
====================

This demo mainly introduces the generation of sine waves based on DAC DMA mode.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_dac_gpio_init``.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/dac/dac_dma**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the DAC IP clock and select the DAC clock source and frequency division.

.. code-block:: C
    :linenos:

    board_dac_gpio_init();

- Configure related pins as ``DAC`` function

.. code-block:: C
    :linenos:

    dac = qcc74x_device_get_by_name("dac");

    /* 512K / 1 = 512K */
    qcc74x_dac_init(dac, DAC_CLK_DIV_1);
    qcc74x_dac_channel_enable(dac, DAC_CHANNEL_A);
    qcc74x_dac_channel_enable(dac, DAC_CHANNEL_B);
    qcc74x_dac_link_txdma(dac, true);

- Get the ``dac`` handle and initialize the dac frequency to 512K
- ``qcc74x_dac_channel_enable`` configure dac channel information, currently used A channel and B channel
- ``qcc74x_dac_link_txdma`` enables dac txdma function

.. code-block:: C
    :linenos:

    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");

    struct qcc74x_dma_channel_config_s config;

    config.direction = DMA_MEMORY_TO_PERIPH;
    config.src_req = DMA_REQUEST_NONE;
    config.dst_req = DMA_REQUEST_DAC;
    config.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    config.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    config.src_burst_count = DMA_BURST_INCR1;
    config.dst_burst_count = DMA_BURST_INCR1;
    config.src_width = DMA_DATA_WIDTH_16BIT;
    config.dst_width = DMA_DATA_WIDTH_16BIT;
    qcc74x_dma_channel_init(dma0_ch0, &config);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

- Configure ``DMA CH0`` as ``DAC``
- Register dma channel interrupt

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_lli_pool_s lli[1]; /* max trasnfer size 4064 * 1 */
    struct qcc74x_dma_channel_lli_transfer_s transfers[1];

    transfers[0].src_addr = (uint32_t)SIN_LIST;
    transfers[0].dst_addr = (uint32_t)DMA_ADDR_DAC_TDR;
    transfers[0].nbytes = sizeof(SIN_LIST);
    qcc74x_l1c_dcache_clean_range((void*)SIN_LIST,sizeof(SIN_LIST));

    qcc74x_dma_channel_lli_reload(dma0_ch0, lli, 1, transfers, 1);
    qcc74x_dma_channel_start(dma0_ch0);

    while (dma_tc_flag0 != 1) {
        qcc74x_mtimer_delay_ms(1);
    }

- Allocate an lli memory pool, the number is 1, and can transfer up to 4064 * 1 byte
- Configure a piece of memory for transmission
- Call ``qcc74x_dma_channel_lli_reload`` to initialize
- Call ``qcc74x_dma_channel_start`` to start transmission
- Wait for transfer to complete and enter interrupt

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

The corresponding GPIOs of DAC Channel A and B output sine waves.

