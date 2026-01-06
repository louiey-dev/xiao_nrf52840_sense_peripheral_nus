/*

*/

#include "bsp.h"

BSP_ST g_Bsp = {0};

int bsp_init(void)
{
    g_Bsp.prdTick = BSP_DEFAULT_PRD_TICK_COUNT;

    bsp_gpio_init();
    bsp_key_init();
    bsp_lsm6ds3tr_init(NULL);

    return 0;
}

void bsp_sleep_sec(int sec)
{
    k_sleep(K_SECONDS(sec));
}

void bsp_sleep_ms(int ms)
{
    k_sleep(K_MSEC(ms));
}

void bsp_sleep_us(int us)
{
    k_sleep(K_USEC(us));
}
