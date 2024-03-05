IR - swm
====================

This demo mainly introduces IR to send and receive data in software mode.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_ir_gpio_init``. Connect the infrared transmitting diode and receiving head to the IR pin. The specific connection method is as follows (taking QCC74x_undef as an example):

.. table:: Hardware connections
    :widths: 50, 50
    :width: 80%
    :align: center

    +-------------------+-----------------------------------+
    | IR signal         | External module                   |
    +===================+===================================+
    | VCC               | Infrared receiver VCC             |
    +-------------------+-----------------------------------+
    | GND               | Infrared receiver GND             |
    +-------------------+-----------------------------------+
    | RX                | Infrared receiver OUT             |
    +-------------------+-----------------------------------+
    | VCC               | Infrared emitting diode anode     |
    +-------------------+-----------------------------------+
    | TX                | Infrared emitting diode cathode   |
    +-------------------+-----------------------------------+

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/ir/ir_swm**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the IR clock and select the IR clock source and frequency division.

.. code-block:: C
    :linenos:

    board_ir_gpio_init();

- Configure related pins as ``IR`` function

.. code-block:: C
    :linenos:

    uint16_t tx_buffer[] = { 1777, 1777, 3555, 3555, 1777, 1777, 1777, 1777, 1777, 1777,
                             3555, 1777, 1777, 1777, 1777, 3555, 3555, 1777, 1777, 3555, 1777 };
    struct qcc74x_ir_tx_config_s tx_cfg;

    irtx = qcc74x_device_get_by_name("irtx");

    /* TX init */
    tx_cfg.tx_mode = IR_TX_SWM;
    qcc74x_ir_tx_init(irtx, &tx_cfg);

- Get ``irtx`` handle
- Set tx_mode to SWM mode and call ``qcc74x_ir_tx_init(irtx, &tx_cfg)`` to initialize ir tx

.. code-block:: C
    :linenos:

    uint16_t rx_buffer[30];
    uint8_t rx_len;
    struct qcc74x_ir_rx_config_s rx_cfg;

    irrx = qcc74x_device_get_by_name("irrx");

    /* RX init */
    rx_cfg.rx_mode = IR_RX_SWM;
    rx_cfg.input_inverse = true;
    rx_cfg.deglitch_enable = false;
    rx_cfg.end_threshold = 3999;
    qcc74x_ir_rx_init(irrx, &rx_cfg);

    /* Enable rx, wait for sending */
    qcc74x_ir_rx_enable(irrx, true);

- Get ``irrx`` handle
- Set rx_mode to SWM mode and call ``qcc74x_ir_rx_init(irrx, &rx_cfg)`` to initialize ir rx
- Call ``qcc74x_ir_rx_enable(irrx, true)`` to enable ir rx and wait for data to be sent

.. code-block:: C
    :linenos:

    qcc74x_ir_swm_send(irtx, tx_buffer, sizeof(tx_buffer) / sizeof(tx_buffer[0]));
    rx_len = qcc74x_ir_swm_receive(irrx, rx_buffer, 30);

- Call ``qcc74x_ir_swm_send(irtx, tx_buffer, sizeof(tx_buffer) / sizeof(tx_buffer[0]))`` to send the data in tx_buffer
- Call ``qcc74x_ir_swm_receive(irrx, rx_buffer, 30)`` to store the received data in rx_buffer

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Press the RST button on the development board and the serial port prints the received data.

