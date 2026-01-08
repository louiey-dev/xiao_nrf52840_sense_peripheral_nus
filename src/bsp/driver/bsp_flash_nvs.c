/*

*/
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include "bsp.h"

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

#define CONFIG_ID 1

LOG_MODULE_REGISTER(nvs_sample, LOG_LEVEL_INF);

/* Define the NVS File System structure
 * We use the "storage_partition" defined in the nRF52840 device tree.
 */
static struct nvs_fs m_fs = {0};
static bool m_nvs_ready = false;

extern BSP_ST g_Bsp;

/**
 * @brief initialize nRF flash NVS function area
 * 
 * @return int 0 : OK, -1 : ERROR
 */
int bsp_nvs_init(void)
{
    int rc = 0;
    struct flash_pages_info info;

    /* 3. Setup NVS Parameters */
    m_fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(m_fs.flash_device))
    {
        LOG_ERR("Flash device %s is not ready", m_fs.flash_device->name);
        return -1;
    }

    m_fs.offset = NVS_PARTITION_OFFSET;

    /* Get flash info to determine sector size */
    rc = flash_get_page_info_by_offs(m_fs.flash_device, m_fs.offset, &info);
    if (rc)
    {
        LOG_ERR("Unable to get page info");
        return -1;
    }
    LOG_INF("Flash page info : offset 0x%lx, %ld, size %d, index %d", info.start_offset, info.start_offset, info.size, info.index);

    /* Configure sector size and count based on the partition size */
    m_fs.sector_size = info.size;
    /* We use the sector count of the storage partition (usually 4KB or 4096 bytes per page on nRF) */
    m_fs.sector_count = 3U;

    /* Mount the NVS file system */
    rc = nvs_mount(&m_fs);
    if (rc)
    {
        LOG_ERR("Flash Init failed");
        return -1;
    }

    m_nvs_ready = true;

    // g_Bsp.nvs.boot_count++;

    LOG_INF("Flash init done");
    return 0;
}

/**
 * @brief nRF flash NVS stored data read
 * 
 * @param p data pointer to read
 * @return int 0 : OK, -1 : ERROR
 */
int bsp_nvs_read(NVS_INFO_ST *p)
{
    int rc = 0;

    if (m_nvs_ready == false)
    {
        LOG_ERR("NVS not ready");
        return -1;
    }

    // Attempt to read existing data
    rc = nvs_read(&m_fs, CONFIG_ID, p, sizeof(NVS_INFO_ST));

    if (rc > 0)
    {
        /* Data found */
        LOG_INF("Found data in NVS!");
        LOG_INF("unique_id : 0x%04x", p->unique_id);
        LOG_INF("boot_count : %d, %d", p->boot_count, g_Bsp.nvs.boot_count);
        LOG_INF("prdTick : %d", p->prdTick);

        g_Bsp.prdTick = p->prdTick;
        p->boot_count++;
    }
    else
    {
        /* No data found (First boot), initialize defaults */
        LOG_INF("No data found. Initializing defaults...");
        p->unique_id = BSP_DEFAULT_UNIQUE_ID;
        p->prdTick = BSP_DEFAULT_PRD_TICK_COUNT;
        p->boot_count = BSP_DEFAULT_BOOT_COUNT;

        // update BSP structure
        g_Bsp.prdTick = p->prdTick;
        /***************************************************/

        bsp_nvs_write(p);
        LOG_INF("NVS default data stored!!");
    }

    return 0;
}

/**
 * @brief nRF flash NVS data store to flash
 * 
 * @param p     data to store
 * @return int  data length to store
 */
int bsp_nvs_write(NVS_INFO_ST *p)
{
    int rc = 0;

    if (m_nvs_ready == false)
    {
        LOG_ERR("NVS not ready");
        return -1;
    }

    p->prdTick = g_Bsp.prdTick;

    // nvs_write returns the number of bytes written, or error code
    rc = nvs_write(&m_fs, CONFIG_ID, p, sizeof(NVS_INFO_ST));

    if (rc < 0)
    {
        LOG_ERR("Failed to write to NVS (Err: %d)", rc);
        return -1;
    }
    else
    {
        LOG_INF("NVS Stored");
        LOG_INF("unique_id :  0x%04x", p->unique_id);
        LOG_INF("boot_count : %d, %d", p->boot_count, g_Bsp.nvs.boot_count);
        LOG_INF("prdTick : %d", p->prdTick);

        LOG_INF("Saved configuration to NVS");
    }

    return 0;
}

/**
 * @brief erase flash NVS area to clean/reset
 * 
 * @return int 0 : OK, -1 : ERROR
 */
int bsp_nvs_reset(void)
{
    int rc = 0;

    if (m_nvs_ready == false)
    {
        LOG_ERR("NVS not ready");
        return -1;
    }

    rc = nvs_clear(&m_fs);
    if(rc != 0)
    {
        LOG_ERR("Failed to clear NVS (Err: %d)", rc);
        return -1;
    }
    else
    {
        LOG_INF("NVS Cleared");
    }

    m_nvs_ready = false;

    bsp_nvs_init();

    return 0;
}
