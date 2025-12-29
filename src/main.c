#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <bluetooth/services/nus.h>

#include <zephyr/logging/log.h>

#include "bsp.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static struct bt_conn *current_conn;
static uint32_t message_count = 0;

#ifdef BSP_CLI_ENABLED
K_THREAD_DEFINE(thread_cli, 2048, cliTask, NULL, NULL, NULL, 7, 0, 0);
#endif

/* --- Callbacks --- */

// 1. Triggered when data arrives from the Phone/Central
static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	struct nus_msg_packet send_data;
	static uint32_t count = 0;

	LOG_INF("Received %u bytes over BLE. First byte: 0x%02x", len, data[0]);

	send_data.id = ((data[0] << 8) | data[1]);
	send_data.len = len + 2;
	// memset(send_data.message, 0, BSP_MAX_MSG_LEN);
	memcpy(send_data.message, &data[2], len);

	int err = bsp_nus_msg_send_to_rcv_task(&send_data, len); // len + sizeof id + sizeof len
	if (err < 0)
	{
		LOG_ERR("Failed to send data (err %d)", err);
	}
	else
	{
		LOG_INF("Sent: %s", send_data.message);
	}
}

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

// 2. Track connection status
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err)
	{
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}
	LOG_INF("Connected!");
	current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason %u)", reason);
	if (current_conn)
	{
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

/* --- Data Sending Logic --- */

void send_test_data(void)
{
	if (!current_conn)
	{
		LOG_WRN("No device connected, skipping send.");
		return;
	}

	char buffer[32];
	int len = snprintk(buffer, sizeof(buffer), "Test Msg: %u", message_count++);

	int err = bt_nus_send(current_conn, (uint8_t *)buffer, len);
	if (err)
	{
		LOG_ERR("Failed to send data (err %d)", err);
	}
	else
	{
		LOG_INF("Sent: %s", buffer);
	}
}

void ble_nus_send_data(char *p, int len)
{
	if (!current_conn)
	{
		LOG_WRN("No device connected, skipping send.");
		return;
	}

	int err = bt_nus_send(current_conn, p, len);
	if (err)
	{
		LOG_ERR("Failed to send data (err %d)", err);
	}
	else
	{
		// LOG_INF("Sent: %s", p);
		LOG_HEXDUMP_WRN(p, len, "Sent:");
	}
}

/* --- Bluetooth Initialization --- */

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

int main(void)
{
	int err;
	int led_offset = 0;
	char buffer[32];

	LOG_INF("Starting NUS Simple Example (No UART)");

	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return -1;
	}

	err = bt_nus_init(&nus_cb);
	if (err)
	{
		LOG_ERR("Failed to init NUS (err %d)", err);
		return -1;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err)
	{
		LOG_ERR("Advertising failed (err %d)", err);
		return -1;
	}

	LOG_INF("Advertising started. Waiting for connection...");

	bsp_init();
	bsp_led_init();

	/* Main Loop: Sends data every 5 seconds */
	while (1)
	{
		k_sleep(K_SECONDS(5));
		// send_test_data();

		sprintf(buffer, "NUS send %d", led_offset++);
		ble_nus_send_data(buffer, strlen(buffer));
	}

	return 0;
}
