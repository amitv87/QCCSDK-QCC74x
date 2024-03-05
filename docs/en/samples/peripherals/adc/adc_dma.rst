ADC - dma
====================

This demo mainly demonstrates how to read the voltage value of adc dma in single-ended mode. The default scan channel 0 ~ channel 10. **It should be noted that some chips may not support all channels**.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_adc_gpio_init``.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/adc/adc_dma**

.. code-block:: C
    :linenos:

    board_init();

- The ADC IP clock will be turned on in ``board_init``, and the ADC clock source and frequency division will be selected (ADC clock must be less than or equal to 500K).

.. code-block:: C
    :linenos:

    board_adc_gpio_init();

- Configure gpio pin as ``ADC`` function

.. code-block:: C
    :linenos:

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = true;
    cfg.continuous_conv_mode = true;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    qcc74x_adc_init(adc, &cfg);

- Get the ``adc`` handle, initialize the adc configuration, and set the adc sampling frequency to 500K

.. code-block:: C
    :linenos:

    qcc74x_adc_channel_config(adc, chan, TEST_ADC_CHANNELS);

- Configure adc channel information. The number of channels used can be configured through ``TEST_ADC_CHANNELS``. Channels 0 ~ 10 are enabled by default. Other channels can be selectively closed according to ``board_adc_gpio_init``.

.. code-block:: C
    :linenos:

    qcc74x_adc_link_rxdma(adc, true);

- Enable adc rx dma function

.. code-block:: C
    :linenos:

    dma0_ch0 = qcc74x_device_get_by_name("dma0_ch0");

    struct qcc74x_dma_channel_config_s config;

    config.direction = DMA_PERIPH_TO_MEMORY;
    config.src_req = DMA_REQUEST_ADC;
    config.dst_req = DMA_REQUEST_NONE;
    config.src_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    config.dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    config.src_burst_count = DMA_BURST_INCR1;
    config.dst_burst_count = DMA_BURST_INCR1;
    config.src_width = DMA_DATA_WIDTH_32BIT;
    config.dst_width = DMA_DATA_WIDTH_32BIT;
    qcc74x_dma_channel_init(dma0_ch0, &config);

    qcc74x_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);

- Configure ``DMA CH0`` as ``ADC RX``
- Register dma channel interrupt

.. code-block:: C
    :linenos:

    struct qcc74x_dma_channel_lli_pool_s lli[1]; /* max trasnfer size 4064 * 1 */
    struct qcc74x_dma_channel_lli_transfer_s transfers[1];

    memset(raw_data, 0, sizeof(raw_data));

    transfers[0].src_addr = (uint32_t)DMA_ADDR_ADC_RDR;
    transfers[0].dst_addr = (uint32_t)raw_data;
    transfers[0].nbytes = sizeof(raw_data);

    qcc74x_dma_channel_lli_reload(dma0_ch0, lli, 1, transfers, 1);
    qcc74x_dma_channel_start(dma0_ch0);

    qcc74x_adc_start_conversion(adc);

    while (dma_tc_flag0 != 1) {
        qcc74x_mtimer_delay_ms(1);
    }

    qcc74x_adc_stop_conversion(adc);


- Allocate an lli memory pool, the number is 1, and can transfer up to 4064 * 1 byte
- Configure a piece of memory for transmission
- Call ``qcc74x_dma_channel_lli_reload`` to initialize
- Call ``qcc74x_dma_channel_start`` to start transmission
- Call ``qcc74x_adc_start_conversion`` to enable conversion of adc
- Wait for transfer to complete and enter interrupt
- Call ``qcc74x_adc_stop_conversion`` to stop adc conversion

.. code-block:: C
    :linenos:

    for (size_t j = 0; j < TEST_ADC_CHANNELS * TEST_COUNT; j++) {
        struct qcc74x_adc_result_s result;
        printf("raw data:%08x\r\n", raw_data[j]);
        qcc74x_adc_parse_result(adc, &raw_data[j], &result, 1);
        printf("pos chan %d,%d mv \r\n", result.pos_chan, result.millivolt);
    }

- Call ``qcc74x_adc_parse_result`` to parse the adc conversion result, and save the parsed value to the ``result`` structure
- Print channel number and voltage value

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------
Print raw data, channel number and voltage value corresponding to the channel.
