#include "qcc74x_gpio.h"
#include "board.h"

struct qcc74x_device_s *gpio;

int main(void)
{
    board_init();

    gpio = qcc74x_device_get_by_name("gpio");
    printf("gpio output\r\n");
    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_OUTPUT | GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    while (1) {
        qcc74x_gpio_set(gpio, GPIO_PIN_0);
        printf("GPIO_PIN_0=%x\r\n", qcc74x_gpio_read(gpio, GPIO_PIN_0));
        qcc74x_mtimer_delay_ms(2000);

        qcc74x_gpio_reset(gpio, GPIO_PIN_0);
        printf("GPIO_PIN_0=%x\r\n", qcc74x_gpio_read(gpio, GPIO_PIN_0));
        qcc74x_mtimer_delay_ms(2000);
    }
}
