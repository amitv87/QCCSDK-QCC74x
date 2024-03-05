I2C - 10-bit
====================

This demo mainly introduces I2C 10-bit slave mode data transmission.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_i2c0_gpio_init``. Connect the USB to I2C module to the development board. The specific pin connection method is as follows (taking QCC743 as an example):

.. table:: Hardware connections
    :widths: 50, 50
    :width: 80%
    :align: center

    +-------------------+------------------+
    | Signal            | USB to I2C module|
    +===================+==================+
    | SCL(GPIO14)       | SCL              |
    +-------------------+------------------+
    | SDA(GPIO15)       | SDA              |
    +-------------------+------------------+

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/i2c/i2c_10_bit**

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

    struct qcc74x_i2c_msg_s msgs[2];
    uint8_t subaddr[2] = { 0x00, 0x04};
    uint8_t write_data[I2C_10BIT_TRANSFER_LENGTH];

    /* Write buffer init */
    write_data[0] = 0x55;
    write_data[1] = 0x11;
    write_data[2] = 0x22;
    for (size_t i = 3; i < I2C_10BIT_TRANSFER_LENGTH; i++) {
        write_data[i] = i;
    }

    /* Write data */
    msgs[0].addr = I2C_10BIT_SLAVE_ADDR;
    msgs[0].flags = I2C_M_NOSTOP | I2C_M_TEN;
    msgs[0].buffer = subaddr;
    msgs[0].length = 2;

    msgs[1].addr = I2C_10BIT_SLAVE_ADDR;
    msgs[1].flags = 0;
    msgs[1].buffer = write_data;
    msgs[1].length = I2C_10BIT_TRANSFER_LENGTH;

    qcc74x_i2c_transfer(i2c0, msgs, 2);

- Initialize sending data (write_data) and configure slave device information (msgs)
- ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` enables i2c transfer

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Send the ``04 00 06 01 03 55`` command to the USB to I2C module through the serial port (baud rate is greater than 115200) to set the I2C slave 10-bit mode data transmission.
Press the RST button on the development board, and the serial port prints the write_data data sent by the development board.
