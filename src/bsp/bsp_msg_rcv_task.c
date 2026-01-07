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

static struct nus_msg_packet nus_data;

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
                uint8_t num = received_data.message[0]; // 0 : RED, 1 : GREEN, 2 : BLUE
                uint8_t onoff = received_data.message[1];

                switch (num)
                {
                case 0:
                    g_Bsp.led_status.led_red = onoff;
                    break;
                case 1:
                    g_Bsp.led_status.led_green = onoff;
                    break;
                case 2:
                    g_Bsp.led_status.led_blue = onoff;
                    break;
                default:
                    break;
                }

                bsp_led_ctrl(num, onoff);
                INF("LED[%d] %d", num, onoff);
                break;

            case NUS_MSG_GET_BATT_ADC:

                break;

            case NUS_MSG_SET_PWM_LED_WIDTH:
                uint32_t pulse_width = received_data.message[0] << 24 | received_data.message[1] << 16 | received_data.message[2] << 8 | received_data.message[3];
                bsp_pwm_led_ctrl(pulse_width);
                g_Bsp.led_status.pwm_led_width = pulse_width;
                INF("LED pulse_width : %d nsec", g_Bsp.led_status.pwm_led_width);
                break;

            case NUS_MSG_SET_PRD_TICK:
                uint16_t tick = received_data.message[0] << 8 | received_data.message[1];
                g_Bsp.prdTick = tick;
                INF("PRD tick : %d ms", g_Bsp.prdTick);
                break;

            case NUS_MSG_SET_RTC:
                RTC_TIME_ST date = {0};
                date.year = received_data.message[0];
                date.mon = received_data.message[1];
                date.day = received_data.message[2];
                date.weekday = received_data.message[3];
                date.hour = received_data.message[4];
                date.min = received_data.message[5];
                date.sec = received_data.message[6];
                bsp_rtc_set_time(&date);
                INF("RTC set 20%02d-%02d-%02d, %02d:%02d:%02d", date.year, date.mon, date.day, date.hour, date.min, date.sec);
                break;

            case NUS_MSG_GET_RTC:
                RTC_TIME_ST gdate = {0};
                bsp_rtc_get_time(&gdate);

                nus_data.id = NUS_MSG_NOTIFY_RTC;
                nus_data.len = sizeof(RTC_TIME_ST);
                memcpy(nus_data.message, &gdate, sizeof(RTC_TIME_ST));
                ble_nus_send_data((char *)&nus_data, sizeof(RTC_TIME_ST) + 4);

                INF("RTC get 20%02d-%02d-%02d, %02d:%02d:%02d", gdate.year, gdate.mon, gdate.day, gdate.hour, gdate.min, gdate.sec);
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
