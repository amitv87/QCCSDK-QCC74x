ADC - tsen
====================

This demo mainly demonstrates measuring the voltage difference of the diode through adc and calculating the ambient temperature.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/adc/adc_tsen**

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
    cfg.vref = ADC_VREF_2P0V;

    struct qcc74x_adc_channel_s chan;

    chan.pos_chan = ADC_CHANNEL_TSEN_P;
    chan.neg_chan = ADC_CHANNEL_GND;

    qcc74x_adc_init(adc, &cfg);

- Get the ``adc`` handle, initialize the adc configuration (the reference voltage must be set to 2.0V), and set the adc sampling frequency to 500K.

.. code-block:: C
    :linenos:

    qcc74x_adc_channel_config(adc, chan, 1);

- Configure adc channel

.. code-block:: C
    :linenos:

    qcc74x_adc_tsen_init(adc, ADC_TSEN_MOD_INTERNAL_DIODE);

- Turn on the tsen function and use the internal diode to measure the voltage value.

.. code-block:: C
    :linenos:

        for (i = 0; i < 50; i++) {
            average_filter += qcc74x_adc_tsen_get_temp(adc);
            qcc74x_mtimer_delay_ms(10);
        }

        printf("temp = %d\r\n", (uint32_t)(average_filter / 50.0));
        average_filter = 0;

- Call ``qcc74x_adc_tsen_get_temp(adc)`` to get the ambient temperature value calculated by adc tsen.

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------
Prints the calculated ambient temperature.