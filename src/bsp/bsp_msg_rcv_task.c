/*
        NUS Message Structure
        id      len     payload
        2 byte  2 byte  array length is BSP_MAX_MSG_LEN(128) and available is len - 4 (id + len)
        01 00   08 00   01 23 41 52
*/

#include "bsp.h"

extern BSP_ST g_Bsp;

static void msg_rcv_task(void);

LOG_MODULE_REGISTER(msg_rcv, LOG_LEVEL_INF);

/* Define the queue: (name, message_size, max_messages, alignment) */
K_MSGQ_DEFINE(nus_msgq, sizeof(struct nus_msg_packet), 10, 4);

K_THREAD_DEFINE(msg_rcv_id, 2048, msg_rcv_task, NULL, NULL, NULL, 7, 0, 0);

static void msg_rcv_task(void)
{
    struct nus_msg_packet received_data;

    while (true)
    {
        /* Wait forever (K_FOREVER) until a message arrives */
        k_msgq_get(&nus_msgq, &received_data, K_FOREVER);
        LOG_HEXDUMP_WRN(&received_data, received_data.len, "nus_msg_rcv:");

        INF("Receiver: Got ID 0x%x with len: %d\n",
            received_data.id, received_data.len);

        for (int i = 0; i < received_data.len - 4; i++)
        {
            INF("%02x ", received_data.message[i]);
        }
        {
            switch (received_data.id)
            {
            case NUS_MSG_LED_CTRL:
                uint8_t num = received_data.message[0];
                uint8_t onoff = received_data.message[1];

                bsp_led_ctrl(num, onoff);
                INF("LED[%d] %d", num, onoff);
                break;

            default:
                INF("0x%04x, %d", received_data.id, received_data.len);
                break;
            }
        }
    }
}

int bsp_nus_msg_send_to_rcv_task(struct nus_msg_packet *p, int len)
{
    INF("0x%x, %d", p->id, len);
    // LOG_HEXDUMP_INF(p, len, "nus_msg_send:");

    /* Send message to queue. Wait 100ms if queue is full. */
    int err = k_msgq_put(&nus_msgq, p, K_MSEC(100));

    if (err == 0)
    {
        INF("Sender: Message 0x%x put in queue\n", p->id);
    }
    else
    {
        ERR("Sender: Queue full!\n");
        err = -1;
    }
    return err;
}
