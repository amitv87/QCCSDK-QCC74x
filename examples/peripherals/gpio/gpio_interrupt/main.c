#include "qcc74x_gpio.h"
#include "qcc74x_mtimer.h"
#include "board.h"
#include "shell.h"
#include "qcc74x_uart.h"

struct qcc74x_device_s *gpio;
struct qcc74x_device_s *uart0;

void gpio0_isr(uint8_t pin)
{
    static uint32_t i = 0;
    if (pin == GPIO_PIN_0) {
        printf("i:%d\r\n", i++);
    }
}

int set_int_mode(int argc, char **argv)
{
    printf("Set gpio interrupt triggle mode\r\n");

    if ((argc != 2) || (atoi(argv[1]) > 3)) {
        printf("Usage: set_int_mode <value>\r\n"
               "    0: GPIO_INT_TRIG_MODE_SYNC_FALLING_EDGE\r\n"
               "    1: GPIO_INT_TRIG_MODE_SYNC_RISING_EDGE\r\n"
               "    2: GPIO_INT_TRIG_MODE_SYNC_LOW_LEVEL\r\n"
               "    3: GPIO_INT_TRIG_MODE_SYNC_HIGH_LEVEL\r\n");
        return 1;
    } else {
        printf("Set gpio interrupt triggle mode to %d\r\n", atoi(argv[1]));
    }

    qcc74x_irq_disable(gpio->irq_num);
    qcc74x_gpio_init(gpio, GPIO_PIN_0, GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN);
    qcc74x_gpio_int_init(gpio, GPIO_PIN_0, atoi(argv[1]));
    qcc74x_gpio_irq_attach(GPIO_PIN_0, gpio0_isr);
    qcc74x_irq_enable(gpio->irq_num);

    return 0;
}

int main(void)
{
    int ch;
    board_init();

    gpio = qcc74x_device_get_by_name("gpio");
    uart0 = qcc74x_device_get_by_name("uart0");
    printf("gpio interrupt\r\n");

    shell_init();

    while (1) {
        while ((ch = qcc74x_uart_getchar(uart0)) != -1) {
            shell_handler(ch);
        }
    }
}
SHELL_CMD_EXPORT_ALIAS(set_int_mode, set_int_mode, shell set_int_triggle_mode.);
