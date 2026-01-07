/*

*/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/audio/dmic.h>

#include "bsp.h"

LOG_MODULE_REGISTER(audio_mq_app, LOG_LEVEL_INF);

/* --- Configuration --- */
#define AUDIO_STACK_SIZE 1024
#define AUDIO_PRIORITY 5

#define PCM_BLOCK_SIZE 320 // 160 samples * 2 bytes
#define QUEUE_DEPTH 10     // Can hold 10 pending audio buffers

/* 1. Define the Memory Slab (The pool of raw data buffers) */
K_MEM_SLAB_DEFINE(mem_slab, PCM_BLOCK_SIZE, 12, 4);

/* 2. Define the Message Queue
 * We are passing pointers (void*), so item_size is sizeof(void*)
 */
K_MSGQ_DEFINE(audio_mq, sizeof(void *), QUEUE_DEPTH, 4);

const struct device *dmic_dev;

/* --- The Audio Thread (Producer) --- */
static void audio_thread_entry(void *p1, void *p2, void *p3)
{
    void *buffer;
    uint32_t size;
    int ret;

    if (!device_is_ready(dmic_dev))
        return;

    LOG_INF("Audio Producer Thread Started");

    while (1)
    {
        /* Read from hardware (Blocking) */
        ret = dmic_read(
            dmic_dev, 0, &buffer, &size, K_FOREVER);

        if (ret == 0)
        {
            /* Send the POINTER to the main thread via Queue.
             * K_NO_WAIT: If the queue is full (Main thread is stuck),
             * we drop the packet immediately to keep the hardware running.
             */
            ret = k_msgq_put(&audio_mq, &buffer, K_NO_WAIT);

            if (ret != 0)
            {
                LOG_WRN("Queue full! Dropping audio packet.");
                /* CRITICAL: If we don't send it, WE must free it here
                 * or we run out of memory.
                 */
                k_mem_slab_free(&mem_slab, &buffer);
            }
        }
    }
}

/* Create the Audio Thread */
K_THREAD_DEFINE(audio_tid, AUDIO_STACK_SIZE,
                audio_thread_entry, NULL, NULL, NULL,
                AUDIO_PRIORITY, 0, 0);

/* --- The Main Thread (Consumer) --- */
static int audio_push_thread(void)
{
    void *pcm_buffer;

    dmic_dev = DEVICE_DT_GET(DT_NODELABEL(pdm0));
    if (!device_is_ready(dmic_dev))
        return 0;

    /* DMIC Config */
    struct pcm_stream_cfg stream_cfg = {
        .pcm_width = 16,
        .mem_slab = &mem_slab,
    };
    struct dmic_cfg cfg = {
        .io = {.min_pdm_clk_freq = 1000000, .max_pdm_clk_freq = 3500000},
        .streams = &stream_cfg,
        .channel = {
            .req_num_chan = 1,
            .req_num_streams = 1,
            .req_chan_map_lo = dmic_build_channel_map(0, 0, PDM_CHAN_LEFT),
        },
    };

    dmic_configure(dmic_dev, &cfg);
    dmic_trigger(dmic_dev, DMIC_TRIGGER_START);

    LOG_INF("Main Thread waiting for audio...");

    while (1)
    {
        /* 3. Wait for a message in the queue
         * This sleeps the main thread until audio arrives.
         */
        if (k_msgq_get(&audio_mq, &pcm_buffer, K_FOREVER) == 0)
        {

            /* --- PROCESS DATA HERE --- */
            int16_t *samples = (int16_t *)pcm_buffer;

            // Example: Check first sample
            LOG_INF("Received Audio! Sample[0]: %d", samples[0]);

            /* Example: Send to BLE or write to Flash here */

            /* 4. CRITICAL: Free the buffer
             * The Audio Thread allocated it, we are responsible for cleaning it up.
             */
            k_mem_slab_free(&mem_slab, &pcm_buffer);
        }
    }
    return 0;
}

K_THREAD_DEFINE(audio_pusht, AUDIO_STACK_SIZE,
                audio_push_thread, NULL, NULL, NULL,
                AUDIO_PRIORITY, 0, 0);
