GPIO - input/output
====================

This demo mainly introduces the GPIO 0 output and GPIO 1 input functions.

Hardware Connection
-----------------------------

Connect the GPIO 0 and GPIO 1 pins using Dupont wire.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/gpio/gpio_input_output**

.. code-block:: C
    :linenos:

    board_init();

- Turn on the clock in ``board_init``

.. code-block:: C
    :linenos:

    gpio = qcc74x_device_get_by_name("gpio");

    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    qcc74x_gpio_init(gpio, GPIO_PIN_1, GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

- Configure GPIO 0 as GPIO_OUTPUT function and GPIO 1 as GPIO_INPUT function

.. code-block:: C
    :linenos:

    while (1) {
        qcc74x_gpio_set(gpio, GPIO_PIN_0);
        printf("GPIO_PIN_1=%x\r\n", qcc74x_gpio_read(gpio, GPIO_PIN_1));
        qcc74x_mtimer_delay_ms(2000);

        qcc74x_gpio_reset(gpio, GPIO_PIN_0);
        printf("GPIO_PIN_1=%x\r\n", qcc74x_gpio_read(gpio, GPIO_PIN_1));
        qcc74x_mtimer_delay_ms(2000);
    }

- ``qcc74x_gpio_set(gpio, GPIO_PIN_0)`` sets the GPIO 0 pin
- ``qcc74x_gpio_read(gpio, GPIO_PIN_1)`` read GPIO 1 pin level
- ``qcc74x_gpio_reset(gpio, GPIO_PIN_0)`` sets the GPIO 0 pin to 0

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Print GPIO 1 pin level.
