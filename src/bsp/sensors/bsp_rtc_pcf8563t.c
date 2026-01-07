/*

*/
#include <zephyr/drivers/i2c.h>
#include "bsp.h"

#define I2C_NODE DT_NODELABEL(pcf8563)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

LOG_MODULE_REGISTER(rtc_pcf8563, LOG_LEVEL_INF);

/* PCF8563 Register Map */
#define PCF8563_REG_SEC 0x02
#define PCF8563_REG_MIN 0x03
#define PCF8563_REG_HOUR 0x04
#define PCF8563_REG_DAY 0x05
#define PCF8563_REG_WEEKDAY 0x06
#define PCF8563_REG_MON 0x07
#define PCF8563_REG_YEAR 0x08

/* BCD Conversion Helpers */
static uint8_t dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd_to_dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

/* In main() check: */
// if (!device_is_ready(dev_i2c.bus))
// {
//     LOG_ERR("I2C Bus (I2C1) is not ready!");
//     return;
// }

/* * FUNCTION: Set Time
 * Writes the time to registers 0x02 to 0x08
 */
int bsp_rtc_set_time(RTC_TIME_ST *time)
{
    uint8_t buffer[8];

    // Start writing at Register 0x02 (Seconds)
    // The first byte in the buffer sent via I2C write is the Register Address
    buffer[0] = PCF8563_REG_SEC;

    // Note: PCF8563 uses Bit 7 of the 'Seconds' register as a Voltage Low flag.
    // We mask it off (0x7F) to ensure we are setting valid seconds.
    buffer[1] = dec_to_bcd(time->sec) & 0x7F;
    buffer[2] = dec_to_bcd(time->min);
    buffer[3] = dec_to_bcd(time->hour);
    buffer[4] = dec_to_bcd(time->day);
    buffer[5] = dec_to_bcd(time->weekday);
    buffer[6] = dec_to_bcd(time->mon);
    buffer[7] = dec_to_bcd(time->year);

    return i2c_write_dt(&dev_i2c, buffer, sizeof(buffer));
}

/* * FUNCTION: Get Time
 * Reads 7 bytes starting from register 0x02
 */
int bsp_rtc_get_time(RTC_TIME_ST *time)
{
    uint8_t regs[7];
    uint8_t start_addr = PCF8563_REG_SEC;

    // Write the register address we want to start reading from
    // Then restart and read 7 bytes
    int ret = i2c_write_read_dt(&dev_i2c, &start_addr, 1, regs, sizeof(regs));
    if (ret != 0)
    {
        return ret;
    }

    // Mask 0x7F to remove status bits (like VL bit in seconds)
    time->sec = bcd_to_dec(regs[0] & 0x7F);
    time->min = bcd_to_dec(regs[1] & 0x7F);
    time->hour = bcd_to_dec(regs[2] & 0x3F); // Hours mask 0x3F (removes unused bits)
    time->day = bcd_to_dec(regs[3] & 0x3F);
    time->weekday = bcd_to_dec(regs[4] & 0x07);
    time->mon = bcd_to_dec(regs[5] & 0x1F); // Month mask 0x1F (removes Century bit)
    time->year = bcd_to_dec(regs[6]);

    return 0;
}
