I2C - eeprom_interrupt
===========================

This demo mainly introduces how I2C uses interrupts to read and write eeprom.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_i2c0_gpio_init``. Connect the eeprom module to the development board. The specific pin connection method is as follows (taking QCC743 as an example):

.. table:: Hardware connections
    :widths: 50, 50
    :width: 80%
    :align: center

    +-------------------+------------------+
    | I2C signal        | eeprom signal    |
    +===================+==================+
    | SCL(GPIO14)       | SCL              |
    +-------------------+------------------+
    | SDA(GPIO15)       | SDA              |
    +-------------------+------------------+
    | GND               | GND              |
    +-------------------+------------------+
    | VCC               | VCC              |
    +-------------------+------------------+

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/i2c/i2c_eeprom_interrupt**

.. code-block:: C
    :linenos:

    board_init();

- ``board_init`` will turn on the I2C IP clock and select the I2C clock source and frequency division.

.. code-block:: C
    :linenos:

    board_i2c0_gpio_init();

- Configure related pins as ``I2C`` function

.. code-block:: C
    :linenos:

    i2c0 = qcc74x_device_get_by_name("i2c0");

    qcc74x_i2c_init(i2c0, 400000);

- Get the ``i2c0`` handle and initialize the i2c0 frequency to 400K

.. code-block:: C
    :linenos:

    /* Set i2c interrupt */
    qcc74x_i2c_int_mask(i2c0, I2C_INTEN_END | I2C_INTEN_TX_FIFO | I2C_INTEN_RX_FIFO | I2C_INTEN_NACK | I2C_INTEN_ARB | I2C_INTEN_FER, false);
    qcc74x_irq_attach(i2c0->irq_num, i2c_isr, NULL);
    qcc74x_irq_enable(i2c0->irq_num);

- Call ``qcc74x_i2c_int_mask(i2c0, I2C_INTEN_END | I2C_INTEN_TX_FIFO | I2C_INTEN_RX_FIFO | I2C_INTEN_NACK | I2C_INTEN_ARB | I2C_INTEN_FER, false)`` to turn on the I2C interrupt
- Register I2C interrupt

.. code-block:: C
    :linenos:

    uint8_t write_data[256];
    uint8_t read_data[256];

    /* Write and read buffer init */
    for (size_t i = 0; i < 256; i++) {
        write_data[i] = i;
        read_data[i] = 0;
    }

- Initialize send and receive buffers

.. code-block:: C
    :linenos:

    /* Write page 0 */
    subaddr[1] = EEPROM_SELECT_PAGE0;

    msgs[0].addr = 0x50;
    msgs[0].flags = I2C_M_NOSTOP;
    msgs[0].buffer = subaddr;
    msgs[0].length = 2;

    msgs[1].addr = 0x50;
    msgs[1].flags = 0;
    msgs[1].buffer = write_data;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;

    qcc74x_i2c_transfer(i2c0, msgs, 2);
    if (txFifoFlag) {
        printf("TX FIFO Ready interrupt generated\r\n");
        txFifoFlag = 0;
    }
    if (rxFifoFlag) {
        printf("RX FIFO Ready interrupt generated\r\n");
        rxFifoFlag = 0;
    }
    printf("write over\r\n\r\n");
    qcc74x_mtimer_delay_ms(100);

- ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` enables i2c transfer

.. code-block:: C
    :linenos:

    /* Unmask interrupt */
    qcc74x_i2c_int_mask(i2c0, I2C_INTEN_END | I2C_INTEN_TX_FIFO | I2C_INTEN_RX_FIFO | I2C_INTEN_NACK | I2C_INTEN_ARB | I2C_INTEN_FER, false);

    /* Write page 1 */
    subaddr[1] = EEPROM_SELECT_PAGE1;

    msgs[1].addr = 0x50;
    msgs[1].flags = 0;
    msgs[1].buffer = write_data + EEPROM_TRANSFER_LENGTH;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;

    qcc74x_i2c_transfer(i2c0, msgs, 2);
    if (txFifoFlag) {
        printf("TX FIFO Ready interrupt generated\r\n");
        txFifoFlag = 0;
    }
    if (rxFifoFlag) {
        printf("RX FIFO Ready interrupt generated\r\n");
        rxFifoFlag = 0;
    }
    printf("write over\r\n\r\n");
    qcc74x_mtimer_delay_ms(100);

- Turn on I2C interrupt for second data transmission

.. code-block:: C
    :linenos:

    /* Unmask interrupt */
    qcc74x_i2c_int_mask(i2c0, I2C_INTEN_END | I2C_INTEN_TX_FIFO | I2C_INTEN_RX_FIFO | I2C_INTEN_NACK | I2C_INTEN_ARB | I2C_INTEN_FER, false);

    /* Read page 0 */
    subaddr[1] = EEPROM_SELECT_PAGE0;

    msgs[1].addr = 0x50;
    msgs[1].flags = I2C_M_READ;
    msgs[1].buffer = read_data;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;
    qcc74x_i2c_transfer(i2c0, msgs, 2);
    if (txFifoFlag) {
        printf("TX FIFO Ready interrupt generated\r\n");
        txFifoFlag = 0;
    }
    if (rxFifoFlag) {
        printf("RX FIFO Ready interrupt generated\r\n");
        rxFifoFlag = 0;
    }
    printf("read over\r\n\r\n");

- Read eeprom data

.. code-block:: C
    :linenos:

    /* Unmask interrupt */
    qcc74x_i2c_int_mask(i2c0, I2C_INTEN_END | I2C_INTEN_TX_FIFO | I2C_INTEN_RX_FIFO | I2C_INTEN_NACK | I2C_INTEN_ARB | I2C_INTEN_FER, false);

    /* Read page 1 */
    subaddr[1] = EEPROM_SELECT_PAGE1;

    msgs[1].addr = 0x50;
    msgs[1].flags = I2C_M_READ;
    msgs[1].buffer = read_data + EEPROM_TRANSFER_LENGTH;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;
    qcc74x_i2c_transfer(i2c0, msgs, 2);
    if (txFifoFlag) {
        printf("TX FIFO Ready interrupt generated\r\n");
        txFifoFlag = 0;
    }
    if (rxFifoFlag) {
        printf("RX FIFO Ready interrupt generated\r\n");
        rxFifoFlag = 0;
    }

- Read data for the second time

.. code-block:: C
    :linenos:

    /* Check read data */
    for (uint8_t i = 0; i < 2 * EEPROM_TRANSFER_LENGTH; i++) {
        if (write_data[i] != read_data[i]) {
            printf("check fail, %d write: %02x, read: %02x\r\n", i, write_data[i], read_data[i]);
        }
    }

- Check whether the data sent and read are consistent

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Press the RST button and after the data transfer is completed, "write over", "read over" and "check over" are printed.
