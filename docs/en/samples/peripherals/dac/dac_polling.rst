DAC - poll
====================

This demo mainly introduces the generation of sine wave based on DAC polling mode.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_adc_gpio_init``.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/dac/dac_polling**

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

    qcc74x_dac_init(dac, DAC_SAMPLING_FREQ_32K);

- Get the ``dac`` handle and initialize the dac frequency to 32K

.. code-block:: C
    :linenos:

    qcc74x_dac_channel_enable(dac, DAC_CHANNEL_A);

- Configure dac channel, currently used A channel

.. code-block:: C
    :linenos:

    for (uint16_t i = 0; i < sizeof(SIN_LIST) / sizeof(uint16_t); i++) {
        qcc74x_dac_set_value(dac, DAC_CHANNEL_A, SIN_LIST[i]);
        qcc74x_mtimer_delay_us(100);
    }

- Call ``qcc74x_dac_set_value(dac, DAC_CHANNEL_A, SIN_LIST[i])`` to output the data to be converted through channel A

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

The GPIO corresponding to DAC Channel A outputs a sine wave.
