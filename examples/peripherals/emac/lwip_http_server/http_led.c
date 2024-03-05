
#include "qcc74x_mtimer.h"
#include "qcc74x_gpio.h"
#include "board.h"
#include "log.h"

#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/opt.h"

#include "http_server.h"
#include "http_led.h"

#define LED_OFF  0
#define LED_ON   1
#define LED_GPIO GPIO_PIN_18 /* do not used flash or uart io */
struct qcc74x_device_s *gpio;

void bsp_led_config(void)
{
    printf("led gpio init\r\n");
    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_init(gpio, LED_GPIO, (0xB << GPIO_FUNC_SHIFT) | GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    qcc74x_gpio_reset(gpio, LED_GPIO);
}

void led_on(void)
{
    qcc74x_gpio_set(gpio, LED_GPIO);
}

void led_off(void)
{
    qcc74x_gpio_reset(gpio, LED_GPIO);
}

void led_task(void *pvParameters)
{
    const TickType_t x_delay = pdMS_TO_TICKS(50);

    bsp_led_config();

    s_x_btn_semaphore = xSemaphoreCreateBinary();

    while (1) {
        if (xSemaphoreTake(s_x_btn_semaphore, x_delay) == pdTRUE) {
            if (button_clicked_flag) {
                printf("***************************************\r\n");
                printf("LED ON\r\n");
                printf("***************************************\r\n");
                led_on();
            } else {
                led_off();
                printf("***************************************\r\n");
                printf("LED OFF\r\n");
                printf("***************************************\r\n");
            }
        }
    }
}
