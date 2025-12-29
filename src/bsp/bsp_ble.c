/*

*/
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/services/nus.h>
#include <zephyr/logging/log.h>

#include "bsp.h"

static struct bt_conn *current_conn;

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* NUS callback, handling received data */
static void nus_receive_cb(struct bt_conn *conn,
                           const uint8_t *data, uint16_t len)
{
    LOG_HEXDUMP_INF(data, len, "NUS RX");
    // ? Do what you have to
}

/* NUS callback struncture */
static struct bt_nus_cb nus_cb = {
    .received = nus_receive_cb,
};
