GPIO - interrupt
====================

This demo mainly introduces the synchronous low-level interrupt type of GPIO 0.

Hardware Connection
-----------------------------

Connect GPIO 0 to GND.

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/peripherals/gpio/gpio_interrupt**

.. code-block:: C
    :linenos:

    board_init();

- Turn on the clock in ``board_init``

.. code-block:: C
    :linenos:

    gpio = qcc74x_device_get_by_name("gpio");

    qcc74x_gpio_int_init(gpio, GPIO_PIN_0, GPIO_INT_TRIG_MODE_SYNC_LOW_LEVEL);

- Set the interrupt type of GPIO 0

.. code-block:: C
    :linenos:

    qcc74x_gpio_int_mask(gpio, GPIO_PIN_0, false);

    qcc74x_irq_attach(gpio->irq_num, gpio_isr, gpio);
    qcc74x_irq_enable(gpio->irq_num);

- ``qcc74x_gpio_int_mask(gpio, GPIO_PIN_0, false)`` turns on GPIO 0 interrupt
- ``qcc74x_irq_attach(gpio->irq_num, gpio_isr, gpio)`` register GPIO interrupt function
- ``qcc74x_irq_enable(gpio->irq_num)`` enables GPIO interrupt

Compile and Program
-----------------------------

Reference :ref:`get_started`

Experimental Phenomena
-----------------------------

Pull the GPIO 0 pin level low to enter the interrupt and print the number of interrupts.

