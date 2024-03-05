====
I2C
====

.. toctree::
    :maxdepth: 1

    I2C - 10-bit <i2c_10_bit>
    I2C - eeprom <i2c_eeprom>
    I2C - eeprom_dma <i2c_eeprom_dma>
    I2C - eeprom_interrupt <i2c_eeprom_interrupt>

The GPIO corresponding to the I2C signal pin is as follows:

.. table:: GPIO
    :widths: 30, 30, 40
    :width: 80%
    :align: center

    +----------+-----------+---------------------------+
    | Signal   | Chip      |           GPIO            |
    +==========+===========+===========================+
    | SCL      | QCC743    | GPIO 14                   |
    +----------+-----------+---------------------------+
    | SDA      | QCC743    | GPIO 15                   |
    +----------+-----------+---------------------------+
