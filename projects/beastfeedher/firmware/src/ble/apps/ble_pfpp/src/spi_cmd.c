// Standard libraries.
#include <assert.h>

// Core
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include "os/os.h"

// This project
#include "gatt_svr_svc_dev_info.h"
#include "spi_defs.h"
#include "spi_cmd.h"


enum l_spi_state
{
    SPI_STATE_IDLE = 0,
    SPI_STATE_INIT_DEV_INFO,
    SPI_STATE_WRITE_CFG,
    SPI_STATE_WRITE_STATS,
};


static void spi_irqs_handler(void *arg, int len);
static void spis_task_handler(void *arg);
//static void spi_proc_pkt(struct spi_pkt *a_spi_pkt);

// Looks like this can be done on local stack too.
static struct hal_spi_settings s_spi_settings =
{
    .data_mode = HAL_SPI_MODE0,
    .data_order = HAL_SPI_MSB_FIRST,
    .word_size = HAL_SPI_WORD_SIZE_8BIT,
    .baudrate = 500 // Don't care in slave mode.
};


// The spi txrx callback.
struct _spi_cb_arg
{
    int transfers;
    int txlen;
    uint32_t tx_rx_bytes;
};

struct _spi_cb_arg s_spi_cb;

uint8_t g_spi_tx_buf[32];
uint8_t g_spi_rx_buf[255];
uint32_t g_spi_xfr_num;

// Local?
//struct spi_pkt s_spi_pkt;

// SPI slave task.
#define SPI_TASK_STACK_SIZE OS_STACK_ALIGN(1024)
struct os_task s_spi_task;
OS_TASK_STACK_DEFINE(s_spi_task_stack, SPI_TASK_STACK_SIZE);

// Global SPI semaphore.
struct os_sem s_spi_sem;


int spi_cmd_init(int task_prio)
{
    //os_stack_t *pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    //assert(pstack);

    os_task_init(
        &s_spi_task,
        "spis",
        spis_task_handler,
        NULL,
        task_prio,
        OS_WAIT_FOREVER,
        s_spi_task_stack,//pstack,
        SPI_TASK_STACK_SIZE
    );

    return 0;
}


// Follows signature of hal_spi_txrx_cb.
// This callback is executed in IRQ context.
// Signal the waiting thread and exit asap.
static void spi_irqs_handler(void *arg, int len)
{
    struct _spi_cb_arg *l_cb_args = (struct _spi_cb_arg *)arg;
    if (l_cb_args == &s_spi_cb)//spi_cb_arg)
    {
        //cb = (struct sblinky_spi_cb_arg *)arg;
        ++l_cb_args->transfers;
        l_cb_args->tx_rx_bytes += len;
        l_cb_args->txlen = len;
    }

    // Post semaphore to task waiting for SPI slave.
    os_sem_release(&s_spi_sem);
}


static void spis_task_handler(void *arg)
{
    (void)arg;

    static const int l_spi_num = 0;
    static unsigned int l_spi_state = SPI_STATE_IDLE;
    assert(hal_spi_config(l_spi_num, &s_spi_settings) == 0);
    hal_spi_set_txrx_cb(l_spi_num, spi_irqs_handler, (void*)&s_spi_cb);
    hal_spi_enable(l_spi_num);
    // Make the default character 0x77.
    hal_spi_slave_set_def_tx_val(l_spi_num, 0x77);

    // COMPREND PAS CE QUI SUIT...
    /*
     * Fill buffer with 0x77 for first transfer. This should be a 0xdeadbeef
     * transfer from master to start things off
     */
    //memset(g_spi_tx_buf, 0x77, 32);
    // Wait for a 1st command to happen. Allow maximum length.
    int rc = hal_spi_txrx_noblock(l_spi_num, NULL /*g_spi_tx_buf*/, g_spi_rx_buf, 255);

    //struct spi_pkt s_spi_pkt;
    while (1)
    {
        // Wait for semaphore from ISR.
        os_sem_pend(&s_spi_sem, OS_TIMEOUT_NEVER);

        // Callback executed: pump some data out?
        switch (l_spi_state)
        {
        case SPI_STATE_IDLE: {
            // We are expecting a command byte to 
            // determine what is coming or have to be sent back.
            //rc = hal_spi_txrx_noblock(l_spi_num, g_spi_tx_buf, (void *)&s_spi_pkt, sizeof(s_spi_pkt));//g_spi_rx_buf, 1);
            //rc = hal_spi_txrx_noblock(l_spi_num, g_spi_tx_buf, (void *)&g_spi_rx_buf[0], s_spi_cb->len);
            //assert(rc == 0);
            // Decode command packet.
            //spi_process_cmd();
            struct spi_pkt *l_spi_pkt = (struct spi_pkt *)&g_spi_rx_buf[0];
            if (l_spi_pkt->cmd == SPI_CMD_INIT_DEV_INFO)
            {
                gatt_svr_svc_dev_info_set((struct device_info *)&g_spi_rx_buf[1]);
            }
            else if (l_spi_pkt->cmd == SPI_CMD_INIT_STATS)
            {
                //gatt_svr_svc_dev_init_stats((struct device_info *)&g_spi_rx_buf[1]);
            }
            break;
        }

        case SPI_STATE_INIT_DEV_INFO:
            // Init the Device Info structure.
            rc = hal_spi_txrx_noblock(l_spi_num, g_spi_tx_buf, g_spi_rx_buf, 1);
            assert(rc == 0);
            break;
        }
        ++g_spi_xfr_num;
    }
}

#if 0
static void spi_proc_pkt(struct spi_pkt *a_spi_pkt)
{
    switch (a_spi_pkt->cmd)
    {
    case SPI_CMD_INIT_DEV_INFO:
    //case SPI_CMD_UPDATE_STATS:
    break;
    }
}
#endif
