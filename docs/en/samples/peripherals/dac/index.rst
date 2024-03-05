====
DAC
====

.. toctree::
    :maxdepth: 1

    DAC - poll <dac_polling>
    DAC - dma <dac_dma>

The GPIO corresponding to each channel of the DAC is as follows:

.. table:: GPIO
    :widths: 25, 25, 25, 25
    :width: 80%
    :align: center

    +----------+-----------+-----------+---------------------------+
    | Channel  | Chip      | Accuracy  |           GPIO            |
    +==========+===========+===========+===========================+
    | ChannelA | QCC743    | 12-bit    | GPIO 3                    |
    +----------+-----------+-----------+---------------------------+
    | ChannelB | QCC743    | 12-bit    | GPIO 2                    |
    +----------+-----------+-----------+---------------------------+
