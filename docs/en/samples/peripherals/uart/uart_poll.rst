UART - poll
====================

This demo mainly demonstrates the UART polling mode sending and receiving function.

Hardware connection
-----------------------------

- Chip UART TX pin connected to USB2TTL module RX
- Chip UART RX pin connected to USB2TTL module TX

The gpio used in this demo is as follows:

.. table:: GPIO
    :widths: 30, 30, 40
    :width: 80%
    :align: center

    +----------+-----------+---------------------------+
    |   Signal | Chip      |           GPIO            |
    +==========+===========+===========================+
    | UART1_TX | QCC743    | GPIO 23                   |
    +----------+-----------+---------------------------+
    | UART1_RX | QCC743    | GPIO 24                   |
    +----------+-----------+---------------------------+

Software Implementation
-----------------------------

For specific software code, see **examples/peripherals/uart/uart_poll**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the UART IP clock and select the UART clock source and frequency division.

.. code-block:: C
    :linenos:

    board_uartx_gpio_init();

- Configure the relevant pins as ``UARTx TX`` and ``UARTx RX`` functions. By default, the demo uses the UART1 peripheral.

.. code-block:: C
    :linenos:

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    qcc74x_uart_init(uartx, &cfg);

- Get the ``DEFAULT_TEST_UART`` handle and initialize the UART

.. code-block:: C
    :linenos:

    int ch;
    while (1) {
        ch = qcc74x_uart_getchar(uartx);
        if (ch != -1) {
            qcc74x_uart_putchar(uartx, ch);
        }
    }

- Call ``qcc74x_uart_getchar`` to read data from uart rx fifo. If -1 is returned, it means there is no data.
- Call ``qcc74x_uart_putchar`` to fill data ``ch`` into uart tx fifo

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Connect the UART1 TX, RX, and GND pins to the USB2TTL module RX, TX, and GND respectively, and press the reset button.
Use the serial port to send ``0123456789`` to UART1, and the USB2TTL module can receive the same data.



