/*

*/
#include "bsp.h"

#ifdef BSP_PRD_TASK_ENABLED

extern BSP_ST g_Bsp;

static void prd_task(void);

LOG_MODULE_REGISTER(bsp_prd, LOG_LEVEL_INF);

K_THREAD_DEFINE(thread_prd, 2048, prd_task, NULL, NULL, NULL, 7, 0, 0);
static uint16_t prd_count = 0;

static void prd_task(void)
{
    static int led_offset = 0;
    while (1)
    {
        INF("prd_task %d", prd_count++);

        // bsp_led_toggle(led_offset++);
        // bsp_sleep_ms(g_Bsp.prdTick);
        k_sleep(K_MSEC(g_Bsp.prdTick));

        if (led_offset > 2)
        {
            led_offset = 0;
        }
    }
}
#endif
