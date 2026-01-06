/*
    Accel: Default unit is m/s². (1 G ≈ 9.80665 m/s²)
    Gyro: Default unit is radians/second.
*/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

#include "bsp.h"

#define IMU_RAW_DATA_FORMAT

extern BSP_ST g_Bsp;

/* * CRITICAL: The XIAO BLE Sense controls sensor power via P1.08.
 * We must drive this HIGH to enable the IMU.
 */
#define SENSOR_POWER_PIN 8
#define SENSOR_POWER_PORT &gpio1 // Note: It is on Port 1, not Port 0

LOG_MODULE_REGISTER(imu, LOG_LEVEL_INF);

/* Get the sensor device from the overlay alias */
const struct device *imu_dev = DEVICE_DT_GET(DT_ALIAS(imu));
static void imu_task(void);
static int16_t convert_to_int16(struct sensor_value *val, double scale_factor);

/* Semaphore to signal data ready */
K_SEM_DEFINE(imu_sem, 0, 1);
K_THREAD_DEFINE(thread_imu, 2048, imu_task, NULL, NULL, NULL, 7, 0, 0);

/* * This function is called by the system thread when the interrupt triggers.
 * Keep it fast. Just signal the main loop.
 */
static void trigger_handler(const struct device *dev,
                            const struct sensor_trigger *trig)
{
    /* Signal the main loop that data is ready */
    k_sem_give(&imu_sem);
}

typedef struct __attribute__((packed))
{
    uint16_t id;
    uint16_t len; // total length of message includes id + len

    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} sensor_packet_t;

static void imu_task(void)
{
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
    sensor_packet_t packet;

    while (1)
    {
        /* Wait here until the interrupt fires */
        k_sem_take(&imu_sem, K_FOREVER);

        /* Fetch and Print Data (Safe to do I2C here) */
        if (sensor_sample_fetch(imu_dev) < 0)
        {
            LOG_ERR("Sample fetch failed");
            continue;
        }

        sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
        sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ, gyro);

#ifdef IMU_RAW_DATA_FORMAT
        // Scaling Factor:
        // Zephyr returns m/s^2. We want to send compact integers.
        // Let's multiply by 100 so 9.81 m/s^2 becomes 981.
        // Max int16 is 32767, so 327.67 m/s^2 (~33 Gs) is our max range. Sufficient.
        double scale = 100.0;

        packet.id = NUS_MSG_NOTIFY_IMU;
        packet.len = 16;
        packet.acc_x = convert_to_int16(&accel[0], scale);
        packet.acc_y = convert_to_int16(&accel[1], scale);
        packet.acc_z = convert_to_int16(&accel[2], scale);

        // Do the same for Gyro (Zephyr returns radians/sec)
        // 1 rad/sec ~ 57 degrees/sec. Multiply by 100 to keep precision.
        packet.gyro_x = convert_to_int16(&gyro[0], scale);
        packet.gyro_y = convert_to_int16(&gyro[1], scale);
        packet.gyro_z = convert_to_int16(&gyro[2], scale);

        g_Bsp.imu.acc_x = packet.acc_x;
        g_Bsp.imu.acc_y = packet.acc_y;
        g_Bsp.imu.acc_z = packet.acc_z;
        g_Bsp.imu.gyro_x = packet.gyro_x;
        g_Bsp.imu.gyro_y = packet.gyro_y;
        g_Bsp.imu.gyro_z = packet.gyro_z;

        ble_nus_send_data(&packet, sizeof(packet));
#else

        g_Bsp.imu.accel[0] = accel[0];
        g_Bsp.imu.accel[1] = accel[1];
        g_Bsp.imu.accel[2] = accel[2];

        g_Bsp.imu.gyro[0] = gyro[0];
        g_Bsp.imu.gyro[1] = gyro[1];
        g_Bsp.imu.gyro[2] = gyro[2];

        LOG_INF("MOTION! | A: X=%.2f Y=%.2f Z=%.2f | G: X=%.2f Y=%.2f Z=%.2f\n",
                sensor_value_to_double(&g_Bsp.imu.accel[0]),
                sensor_value_to_double(&g_Bsp.imu.accel[1]),
                sensor_value_to_double(&g_Bsp.imu.accel[2]),
                sensor_value_to_double(&g_Bsp.imu.gyro[0]),
                sensor_value_to_double(&g_Bsp.imu.gyro[1]),
                sensor_value_to_double(&g_Bsp.imu.gyro[2]));
#endif
    }
}

/**
 * @brief Enable/Disable IMU power source via GPIO control
 *
 * @param conoff    0 : OFF, 1 : ON
 * @return int      0 : OK, 0 > : ERROR
 */
static int enable_sensor_power(int conoff)
{
    /* Get the GPIO Port 1 device */
    const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

    if (!device_is_ready(gpio_dev))
    {
        LOG_ERR("GPIO port 1 not ready");
        return -1;
    }

    /* Configure P1.08 as Output */
    int ret = gpio_pin_configure(gpio_dev, SENSOR_POWER_PIN, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure power pin");
        return ret;
    }

    /* Set High to turn ON the sensor */
    gpio_pin_set(gpio_dev, SENSOR_POWER_PIN, conoff);

    /* Give the sensor a moment to wake up */
    k_msleep(50);

    LOG_INF("IMU Power %s", conoff ? "ON" : "OFF");

    return 0;
}

/**
 * @brief lsm6ds3tr-c sensor init
 *
 * @param p void for future use
 * @return int 0 : OK, -1 : ERROR
 */
int bsp_lsm6ds3tr_init(void *p)
{
    /* 1. Turn on the hardware power for the sensor */
    if (enable_sensor_power(1) < 0)
    {
        LOG_ERR("Failed to enable sensor power");
        return -1;
    }

    /* 2. Get the sensor device from the overlay alias */
    // const struct device *imu_dev = DEVICE_DT_GET(DT_ALIAS(imu));

    if (!device_is_ready(imu_dev))
    {
        LOG_ERR("Sensor device not ready. Check overlay and power pin.");
        return -1;
    }

    /* 3. Configure the Data Ready Trigger */
    struct sensor_trigger trig = {
        .type = SENSOR_TRIG_DELTA, // SENSOR_TRIG_DATA_READY,
        .chan = SENSOR_CHAN_ACCEL_XYZ,
    };

    /* Register the callback function */
    if (sensor_trigger_set(imu_dev, &trig, trigger_handler) < 0)
    {
        LOG_ERR("Could not set trigger. Check GPIO/IRQ config.");
        return -1;
    }

    /* * Optional: Set Output Data Rate (ODR)
     * If ODR is 0, the sensor might not generate interrupts.
     * Set to 26 Hz for this test.
     */
    struct sensor_value odr_attr;
    odr_attr.val1 = 26; // 26 Hz
    odr_attr.val2 = 0;
    sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);

    /* Set Gyroscope ODR to 26 Hz (CRITICAL STEP)
     * If you skip this, the Gyro remains off, and you will read zeros.
     */
    sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);

    /* Configure Sensitivity (Slope Threshold)
     * This controls how hard you have to shake it.
     * 0.12 G is a good middle ground. Increase to 0.5 G to make it less sensitive.
     */
    sensor_value_from_double(&odr_attr, 0.5);
    if (sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SLOPE_TH, &odr_attr) < 0)
    {
        LOG_ERR("Cannot set slope threshold");
        return -1;
    }

    /* Configure Duration (Filter)
     * Require 1 sample over threshold to trigger (Instant reaction)
     */
    odr_attr.val1 = 1;
    odr_attr.val2 = 0;
    sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SLOPE_DUR, &odr_attr);

    LOG_INF("IMU (Accel + Gyro) running at 26Hz. Waiting for data...");

    g_Bsp.imu.isInit = 1;

    return 0;
}

/**
 * @brief Read 6D sensor
 *
 * @param p void for future use
 * @return int 0 : OK, -1 : ERROR
 */
int bsp_lsm6ds3tr_read(void *p)
{
    // const struct device *imu_dev = DEVICE_DT_GET(DT_ALIAS(imu));

    struct sensor_value accel[3];
    struct sensor_value gyro[3];

    /* Fetch the sample from the hardware */
    if (sensor_sample_fetch(imu_dev) < 0)
    {
        LOG_ERR("Sensor sample update error");
        return -1;
    }

    /* Get the data (Accel and Gyro) */
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ, gyro);

    g_Bsp.imu.accel[0] = accel[0];
    g_Bsp.imu.accel[1] = accel[1];
    g_Bsp.imu.accel[2] = accel[2];

    g_Bsp.imu.gyro[0] = gyro[0];
    g_Bsp.imu.gyro[1] = gyro[1];
    g_Bsp.imu.gyro[2] = gyro[2];

    /* 5. Print Data using sensor_value_to_double helper */
    LOG_INF("AX: %.2f  AY: %.2f  AZ: %.2f (m/s^2)\n",
            sensor_value_to_double(&g_Bsp.imu.accel[0]),
            sensor_value_to_double(&g_Bsp.imu.accel[1]),
            sensor_value_to_double(&g_Bsp.imu.accel[2]));

    LOG_INF("GX: %.2f  GY: %.2f  GZ: %.2f (rad/s)\n\n",
            sensor_value_to_double(&g_Bsp.imu.gyro[0]),
            sensor_value_to_double(&g_Bsp.imu.gyro[1]),
            sensor_value_to_double(&g_Bsp.imu.gyro[2]));

    return 0;
}

// Helper: Convert Zephyr sensor_value (m/s^2) back to int16 raw-like scale
// This saves bandwidth. We reverse the driver's conversion essentially.
static int16_t convert_to_int16(struct sensor_value *val, double scale_factor)
{
    // sensor_value_to_double is a Zephyr helper
    double real_val = sensor_value_to_double(val);

    // Scale it back up.
    // Example: If 1G = 9.8m/s^2, and we want 1G = 1000 in our int16, scale = 1000/9.8
    // Or simpler: Just map m/s^2 directly to int16 with a multiplier (e.g. x100)
    return (int16_t)(real_val * scale_factor);
}
