/*

*/

#include "bsp.h"

BSP_ST g_Bsp = {0};

/**
 * @brief Initialize bsp functions
 * 
 * @return int 0 : OK
 */
int bsp_init(void)
{
    g_Bsp.prdTick = BSP_DEFAULT_PRD_TICK_COUNT;

    bsp_gpio_init();
    bsp_key_init();
    bsp_lsm6ds3tr_init(NULL);

    return 0;
}

/**
 * @brief sleep function for sec
 * 
 * @param sec 
 */
void bsp_sleep_sec(int sec)
{
    k_sleep(K_SECONDS(sec));
}

/**
 * @brief sleep function for msec
 * 
 * @param ms 
 */
void bsp_sleep_ms(int ms)
{
    k_sleep(K_MSEC(ms));
}

/**
 * @brief sleep function for usec
 * 
 * @param us 
 */
void bsp_sleep_us(int us)
{
    k_sleep(K_USEC(us));
}
