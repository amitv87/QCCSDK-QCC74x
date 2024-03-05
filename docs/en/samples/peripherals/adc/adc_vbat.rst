ADC - vbat
====================

This demo mainly demonstrates the voltage value of adc measurement chip VDD33.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/adc/adc_vbat**

.. code-block:: C
    :linenos:

    board_init();

- The ADC IP clock will be turned on in ``board_init``, and the ADC clock source and frequency division will be selected (ADC clock must be less than or equal to 500K).

.. code-block:: C
    :linenos:

    adc = qcc74x_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct qcc74x_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = false;
    cfg.continuous_conv_mode = false;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_3P2V;

    struct qcc74x_adc_channel_s chan;

    chan.pos_chan = ADC_CHANNEL_VABT_HALF;
    chan.neg_chan = ADC_CHANNEL_GND;

    qcc74x_adc_init(adc, &cfg);

- Get the ``adc`` handle, initialize the adc configuration, and set the adc sampling frequency to 500K.

.. code-block:: C
    :linenos:

    qcc74x_adc_channel_config(adc, chan, 1);

- Configure adc channel

.. code-block:: C
    :linenos:

    qcc74x_adc_vbat_enable(adc);

- Enable vbat function

.. code-block:: C
    :linenos:

    struct qcc74x_adc_result_s result;
    for (uint16_t i = 0; i < 10; i++) {
        qcc74x_adc_start_conversion(adc);
        while (qcc74x_adc_get_count(adc) == 0) {
            qcc74x_mtimer_delay_ms(1);
        }
        uint32_t raw_data = qcc74x_adc_read_raw(adc);

        qcc74x_adc_parse_result(adc, &raw_data, &result, 1);
        printf("vBat = %d mV\r\n", (uint32_t)(result.millivolt * 2));
        qcc74x_adc_stop_conversion(adc);

        qcc74x_mtimer_delay_ms(500);
    }

- Call ``qcc74x_adc_start_conversion(adc)`` to enable conversion of adc
- Call ``qcc74x_adc_get_count(adc)`` to read the number of completed conversions
- Call ``qcc74x_adc_read_raw(adc)`` to read the conversion value of adc once
- Call ``qcc74x_adc_parse_result(adc, &raw_data, &result, 1)`` to parse the conversion result of adc, and save the parsed value to the ``result`` structure
- Call ``qcc74x_adc_stop_conversion(adc)`` to stop adc conversion

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------
Print the voltage value of chip VDD33.
