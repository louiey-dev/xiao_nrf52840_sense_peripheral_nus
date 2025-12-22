/*

*/
#include <nrfx_gpiote.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/dt-bindings/gpio/nordic-nrf-gpio.h>
#include <zephyr/irq.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/drivers/gpio/gpio_utils.h>

#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <nrfx.h>

#include "bsp.h"

LOG_MODULE_REGISTER(bsp_gpio);

#define BUTTONS_NODE DT_PATH(buttons)
#define LEDS_NODE DT_PATH(leds)

#define GPIO0_DEV DEVICE_DT_GET(DT_NODELABEL(gpio0))
#define GPIO1_DEV DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpio1))

/* GPIO0, GPIO1 and GPIO expander devices require different interrupt flags. */
#define FLAGS_GPIO_0_1_ACTIVE GPIO_INT_LEVEL_ACTIVE
#define FLAGS_GPIO_EXP_ACTIVE (GPIO_INT_EDGE | GPIO_INT_HIGH_1 | GPIO_INT_LOW_0 | GPIO_INT_ENABLE)

#define GPIO_SPEC_AND_COMMA(button_or_led) GPIO_DT_SPEC_GET(button_or_led, gpios),

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_EXISTS(BUTTONS_NODE)
    DT_FOREACH_CHILD(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static const struct gpio_dt_spec leds[] = {
#if DT_NODE_EXISTS(LEDS_NODE)
    DT_FOREACH_CHILD(LEDS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

/**
 * @brief initialize gpio pins, LEDs/Buttons/IOs
 *
 * @return int
 */
int bsp_gpio_init(void)
{
    bsp_led_init();

    return 0;
}

/**
 * @brief LED init, BSP_LED_RED/GREEN/BLUE
 *
 * @return int
 */
int bsp_led_init(void)
{
    int err;

    for (size_t i = 0; i < ARRAY_SIZE(leds); i++)
    {
        err = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT);
        if (err)
        {
            printk("Cannot configure LED gpio\n");
            return err;
        }
    }
    return 0;
}

/**
 * @brief control led on/off
 *
 * @param led, BSP_LED_RED/GREEN/BLUE
 * @param val , 0 : OFF 1 : ON
 * @return int. 0 : OK, !0 : ERROR
 */
int bsp_led_ctrl(int led, int val)
{
    int err = gpio_pin_set_dt(&leds[led], val);
    if (err)
    {
        printk("Cannot write LED[%d] %s\n", led, val ? "ON" : "OFF");
        return err;
    }
    printk("LED[%d] %s\n", led, val ? "ON" : "OFF");

    return 0;
}

int bsp_led_toggle(int led)
{
    int err = gpio_pin_toggle_dt(&leds[led]);
    if (err)
    {
        printk("Cannot toggle LED[%d]\n", led);
        return err;
    }
    printk("LED[%d] toggle\n", led);

    return 0;
}