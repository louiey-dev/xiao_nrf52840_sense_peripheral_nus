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
#include <zephyr/drivers/sensor.h>
// #include <nrfx.h>

#define USER_FUNC __FUNCTION__

#define PACKED __attribute__((packed))

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
typedef struct PACKED LSM6DS3TR_S
{
    struct sensor_value accel[3];
    struct sensor_value gyro[3];

    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    uint8_t isInit;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
} LSM6DS3TR_ST;

typedef struct PACKED LED_S
{
    uint8_t led_red;
    uint8_t led_green;
    uint8_t led_blue;
    uint8_t reserved;

    uint32_t pwm_led_width;
} LED_ST;

typedef struct PACKED BATT_ADC_S
{
    int value;
} BATT_ADC_ST;

typedef struct PACKED RTC_TIME_S
{
    uint8_t year; // Years since 2000
    uint8_t mon;
    uint8_t day;
    uint8_t weekday;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} RTC_TIME_ST;

typedef struct PACKED BSP_S
{
    uint8_t isInit;

    uint16_t prdTick; // periodic task tick count in ms

    LED_ST led_status;

    BATT_ADC_ST batt_adc;

    LSM6DS3TR_ST imu;

    RTC_TIME_ST rtc;
} BSP_ST;

/* Define message structure */
struct PACKED nus_msg_packet
{
    uint16_t id;
    uint16_t len; // total received length of message includes id + len
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
    NUS_MSG_LED_CTRL = 1, // ID(2) | LEN(2) | LED_NUM(1) | LED_ONOFF(1)
    NUS_MSG_GET_BATT_ADC = 2,
    NUS_MSG_SET_PWM_LED_WIDTH = 3, // ID(2) | LEN(2) | PULSE_WIDTH(4)
    NUS_MSG_SET_PRD_TICK = 4,      // ID(2) | LEN(2) | PRD_TICK(2)
    NUS_MSG_GET_RTC = 5,
    NUS_MSG_SET_RTC = 6,
    NUS_MSG_05 = 7,
    NUS_MSG_06 = 8,
    NUS_MSG_07 = 9,
    NUS_MSG_08 = 10,
    NUS_MSG_09 = 11,
    NUS_MSG_10 = 12,
    NUS_MSG_11 = 13,
    NUS_MSG_12 = 14,
    NUS_MSG_13 = 15,
    NUS_MSG_NOTIFY_IMU = 16, // ID(2) | LEN(2) | ACC_X(2) | ACC_Y(2) | ACC_Z(2) | GYRO_X(2) | GYRO_Y(2) | GYRO_Z(2)
    NUS_MSG_NOTIFY_RTC = 17, // ID(2) | LEN(2) | YEAR(2) | MON(2) | DAY(2) | WEEKDAY(2) | HOUR(2) | MIN(2) | SEC(2)
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

int bsp_lsm6ds3tr_init(void *p);
int bsp_lsm6ds3tr_read(void *p);

int bsp_rtc_set_time(RTC_TIME_ST *time);
int bsp_rtc_get_time(RTC_TIME_ST *time);
/**************/
