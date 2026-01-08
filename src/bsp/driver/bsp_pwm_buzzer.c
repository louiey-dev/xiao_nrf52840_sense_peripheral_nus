/*

*/
#include <zephyr/drivers/pwm.h>
#include "bsp.h"

LOG_MODULE_REGISTER(pwm_buzzer, LOG_LEVEL_INF);

/* Get the buzzer configuration from Device Tree alias */
static const struct pwm_dt_spec buzzer = PWM_DT_SPEC_GET(DT_ALIAS(buzzer_pwm));

extern BSP_ST g_Bsp;

/**
 * @brief   control buzzer via pwm
 *          after on buzzer, wait for duration time and pwm stop
 * 
 * @param frequency_hz pwm freq in hz
 * @param duration_ms   how long will wait after send pwm data to buzzer
 * @return int 
 */
int bsp_pwm_buzzer(uint16_t frequency_hz, uint16_t duration_ms)
{
    int ret;

    if (!pwm_is_ready_dt(&buzzer)) {
        LOG_ERR("PWM device not ready");
        return -1;
    }

    if (frequency_hz == 0) {
        /* Silence */
        pwm_set_dt(&buzzer, 0, 0);
        k_sleep(K_MSEC(duration_ms));
        return -1;
    }

    /* Calculate period in nanoseconds.
     * Formula: 1 sec (in ns) / Frequency
     */
    uint32_t period_ns = 1000000000U / frequency_hz;

    /* * Set Duty Cycle to 50% (Period / 2) for maximum volume
     */
    uint32_t pulse_ns = period_ns / 2;

    ret = pwm_set_dt(&buzzer, period_ns, pulse_ns);
    if (ret) {
        LOG_ERR("Error setting PWM: %d", ret);
        return -1;
    }

    /* Wait for the duration of the note */
    k_sleep(K_MSEC(duration_ms));

    /* Turn off optionally between notes to articulate them */
    pwm_set_dt(&buzzer, 0, 0);
    k_sleep(K_MSEC(50));

    return 0;
}
