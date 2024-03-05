ADC - int
====================

This demo mainly demonstrates reading the voltage value in adc interrupt mode. The default scan channel 0 ~ channel 10. **It should be noted that some chips may not support all channels**.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_adc_gpio_init``.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/adc/adc_int**

.. code-block:: C
    :linenos:

    board_init();

- The ADC IP clock will be turned on in ``board_init``, and the ADC clock source and frequency division will be selected (ADC clock must be less than or equal to 500K).

.. code-block:: C
    :linenos:

    board_adc_gpio_init();

- Configure related pins as ``ADC`` function

.. code-block:: C
    :linenos:

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = true;
    cfg.continuous_conv_mode = false;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    qcc74x_adc_init(adc, &cfg);

- Get the ``adc`` handle, initialize the adc configuration, and set the adc sampling frequency to 500K

.. code-block:: C
    :linenos:

    qcc74x_adc_channel_config(adc, chan, TEST_ADC_CHANNELS);

- Configure adc channel information. The number of pairs used is configurable according to ``TEST_ADC_CHANNELS``. Channels 0 ~ 10 are enabled by default. Other channels can be selectively closed according to ``board_adc_gpio_init``.

.. code-block:: C
    :linenos:

    qcc74x_adc_rxint_mask(adc, false);
    qcc74x_irq_attach(adc->irq_num, adc_isr, NULL);
    qcc74x_irq_enable(adc->irq_num);

- Call ``qcc74x_adc_rxint_mask`` to turn on adc conversion completion interrupt
- Call ``qcc74x_irq_attach`` connection interrupt handler function
- Call ``qcc74x_irq_enable`` to enable interrupts

.. code-block:: C
    :linenos:

    for (size_t i = 0; i < TEST_COUNT; i++) {
        read_count = 0;
        qcc74x_adc_start_conversion(adc);

        while (read_count < TEST_ADC_CHANNELS) {
            qcc74x_mtimer_delay_ms(1);
        }
        for (size_t j = 0; j < TEST_ADC_CHANNELS; j++) {
            struct qcc74x_adc_result_s result;
            printf("raw data:%08x\r\n", raw_data[j]);
            qcc74x_adc_parse_result(adc, (uint32_t *)&raw_data[j], &result, 1);
            printf("pos chan %d,%d mv \r\n", result.pos_chan, result.millivolt);
        }
        qcc74x_adc_stop_conversion(adc);
        qcc74x_mtimer_delay_ms(100);
    }

- Call ``qcc74x_adc_start_conversion(adc)`` to enable conversion of adc
- Call ``qcc74x_adc_parse_result(adc, (uint32_t *)&raw_data[j], &result, 1)`` to parse the conversion result of adc, and save the parsed value to the ``result`` structure
- Call ``qcc74x_adc_stop_conversion(adc)`` to stop adc conversion

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------
Print raw data, channel number and voltage value corresponding to the channel.
