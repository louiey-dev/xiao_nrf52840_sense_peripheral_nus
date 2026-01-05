/*

*/
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
// #include <soc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
// #include <nrfx.h>

#define USER_FUNC __FUNCTION__

/**
 * @brief LOG definitions
 *
 */
#define DBG LOG_DBG
#define LOG printk
#define WRN LOG_WRN
#define ERR LOG_ERR
#define INF LOG_INF
/*******************************************************/

/**
 * @brief definition for global project
 *
 */
#define BSP_CLI_ENABLED
#define BSP_PRD_TASK_ENABLED
#define BSP_LCD_SSD1306_ENABLED
/******************************************/

/**** LEDs ****/
#define BSP_NO_LEDS_MSK (0)
#define BSP_LED_RED 0
#define BSP_LED_GREEN 1
#define BSP_LED_BLUE 2
/*********************************************************/

/**
 * @brief Basic Default Params
 *
 */
#define BSP_DEFAULT_PRD_TICK_COUNT 1000 // ms

#define BSP_MAX_MSG_LEN 128 // used to communicate with app via NUS

/*********************************************************/
typedef struct LED_S
{
    uint8_t led_red;
    uint8_t led_green;
    uint8_t led_blue;
    uint8_t reserved;

    uint32_t pwm_led_width;
} LED_ST;

typedef struct BATT_ADC_S
{
    int value;
} BATT_ADC_ST;

typedef struct BSP_S
{
    uint8_t isInit;

    uint16_t prdTick; // periodic task tick count in ms

    LED_ST led_status;

    BATT_ADC_ST batt_adc;

} BSP_ST;

/* Define message structure */
struct nus_msg_packet
{
    uint16_t id;
    uint16_t len; // total received length of message includes id
    char message[BSP_MAX_MSG_LEN];
};
/*********************************************************/

/**
 * @brief NUS Message list
 *
 */
enum NUS_MSG_EN
{
    NUS_MSG_NONE = 0,
    NUS_MSG_LED_CTRL = 1,
    NUS_MSG_GET_BATT_ADC = 2,
    NUS_MSG_SET_PWM_LED_WIDTH = 3,
    NUS_MSG_SET_PRD_TICK = 4,
    NUS_MSG_03 = 5,
    NUS_MSG_04 = 6,
    NUS_MSG_05 = 7,
    NUS_MSG_06 = 8,
    NUS_MSG_07 = 9,
    NUS_MSG_08 = 10,
    NUS_MSG_09 = 11,
    NUS_MSG_10 = 12,
    NUS_MSG_11 = 13,
    NUS_MSG_12 = 14,
    NUS_MSG_13 = 15,
    NUS_MSG_14 = 16,
    NUS_MSG_15 = 17,
    NUS_MSG_16 = 18,
    NUS_MSG_17 = 19,
    NUS_MSG_18 = 20,
    NUS_MSG_19 = 21,
    NUS_MSG_20 = 22,
};
/*********************************************************/

/**** APIs ****/
int bsp_init(void);
int bsp_gpio_init(void);
int bsp_led_init(void);
int bsp_led_ctrl(int led, int val);
int bsp_led_toggle(int led);
int bsp_pwm_led_ctrl(uint32_t pulse_width);
int bsp_key_init(void);

void cliTask(void *pvParameters);

void bsp_sleep_sec(int sec);
void bsp_sleep_ms(int ms);
void bsp_sleep_us(int us);

int bsp_nus_msg_send_to_rcv_task(struct nus_msg_packet *p, int len);
void ble_nus_send_data(char *p, int len);
/**************/
