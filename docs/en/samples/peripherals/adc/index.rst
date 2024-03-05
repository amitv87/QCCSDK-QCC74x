=====
ADC
=====

.. toctree::
    :maxdepth: 1

    ADC - poll <adc_poll>
    ADC - dma <adc_dma>
    ADC - interrupt <adc_int>
    ADC - poll_diff_mode <adc_poll_diff_mode>
    ADC - temperature sensor <adc_tsen>
    ADC - vbat <adc_vbat>


The GPIO corresponding to each channel of the ADC is as follows:

.. table:: GPIO
    :widths: 30, 30, 40
    :width: 80%
    :align: center

    +----------+-----------+---------------------------+
    |Channel   | Chip      |           GPIO            |
    +==========+===========+===========================+
    | Channel0 | QCC743    | GPIO 20                   |
    +----------+-----------+---------------------------+
    | Channel1 | QCC743    | GPIO 19                   |
    +----------+-----------+---------------------------+
    | Channel2 | QCC743    | GPIO 2(Bootstrap pin)     |
    +----------+-----------+---------------------------+
    | Channel3 | QCC743    | GPIO 3                    |
    +----------+-----------+---------------------------+
    | Channel4 | QCC743    | GPIO 14                   |
    +----------+-----------+---------------------------+
    | Channel5 | QCC743    | GPIO 13                   |
    +----------+-----------+---------------------------+
    | Channel6 | QCC743    | GPIO 12                   |
    +----------+-----------+---------------------------+
    | Channel7 | QCC743    | GPIO 10                   |
    +----------+-----------+---------------------------+
    | Channel8 | QCC743    | GPIO 1                    |
    +----------+-----------+---------------------------+
    | Channel9 | QCC743    | GPIO 0                    |
    +----------+-----------+---------------------------+
    | Channel10| QCC743    | GPIO 27                   |
    +----------+-----------+---------------------------+
    | Channel11| QCC743    | GPIO 28                   |
    +----------+-----------+---------------------------+
