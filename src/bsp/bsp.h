/*

*/
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <soc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <nrfx.h>

#define BSP_CLI_ENABLED 1

/**** LEDs ****/
#define BSP_NO_LEDS_MSK (0)
#define BSP_LED_RED 0
#define BSP_LED_GREEN 1
#define BSP_LED_BLUE 2

/*********************************************************/

typedef struct BSP_S
{
    bool isInit;

    uint32_t prdTick;
} BSP_ST;

/**** APIs ****/
int bsp_gpio_init(void);
int bsp_led_init(void);
int bsp_led_ctrl(int led, int val);
int bsp_led_toggle(int led);

void cliTask(void *pvParameters);

void bsp_sleep_sec(int sec);
void bsp_sleep_ms(int ms);
void bsp_sleep_us(int us);

/**************/
