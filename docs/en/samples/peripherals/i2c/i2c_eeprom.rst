I2C - eeprom
====================

This demo mainly introduces I2C reading and writing eeprom.

Hardware Connection
-----------------------------

The gpio used in this demo refers to ``board_i2c0_gpio_init``. Connect the eeprom module to the development board. The specific pin connection method is as follows (taking QCC743 as an example):

.. table:: Hardware connections
    :widths: 50, 50
    :width: 80%
    :align: center

    +-------------------+------------------+
    | I2C single        | eeprom signal    |
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

For more detailed code, please refer to **examples/peripherals/i2c/i2c_eeprom**

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
    uint8_t subaddr[2] = { 0x00, EEPROM_SELECT_PAGE0};
    uint8_t write_data[256];

    /* Write and read buffer init */
    for (size_t i = 0; i < 256; i++) {
        write_data[i] = i;
        read_data[i] = 0;
    }

    /* Write page 0 */
    msgs[0].addr = 0x50;
    msgs[0].flags = I2C_M_NOSTOP;
    msgs[0].buffer = subaddr;
    msgs[0].length = 2;

    msgs[1].addr = 0x50;
    msgs[1].flags = 0;
    msgs[1].buffer = write_data;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;

    qcc74x_i2c_transfer(i2c0, msgs, 2);

- Initialize sending data (write_data), receiving buffer and configuring slave device information (msgs)
- ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` enables i2c transfer

.. code-block:: C
    :linenos:

    uint8_t read_data[256];

    /* Read page 0 */
    msgs[1].addr = 0x50;
    msgs[1].flags = I2C_M_READ;
    msgs[1].buffer = read_data;
    msgs[1].length = EEPROM_TRANSFER_LENGTH;
    qcc74x_i2c_transfer(i2c0, msgs, 2);

- Read the data in the slave device register address and store it in read_data
- ``qcc74x_i2c_transfer(i2c0, msgs, 2)`` enables i2c transfer

.. code-block:: C
    :linenos:

    /* Check read data */
    for (uint8_t i = 0; i < EEPROM_TRANSFER_LENGTH; i++) {
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
