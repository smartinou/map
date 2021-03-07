
// Standard libraries.
#include <assert.h>

// Core
#include "bsp/bsp.h"
#include "console/console.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include "hal/hal_system.h"
//#include "config/config.h"
#include "os/mynewt.h"
#include "os/os.h"
#include "sysinit/sysinit.h"

// BLE
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

// This project
#include "gatt_svr.h"
#include "spi_cmd.h"


/**
 * Depending on the type of package, there are different
 * compilation rules for this directory.  This comment applies
 * to packages of type "app."  For other types of packages,
 * please view the documentation at http://mynewt.apache.org/.
 *
 * Put source files in this directory.  All files that have a *.c
 * ending are recursively compiled in the src/ directory and its
 * descendants.  The exception here is the arch/ directory, which
 * is ignored in the default compilation.
 *
 * The arch/<your-arch>/ directories are manually added and
 * recursively compiled for all files that end with either *.c
 * or *.a.  Any directories in arch/ that don't match the
 * architecture being compiled are not compiled.
 *
 * Architecture is set by the BSP/MCU combination.
 */


// Define task stack and task object.
// Task priorities should be unique!
#define SPI_CMD_TASK_PRIO (MYNEWT_VAL(OS_MAIN_TASK_PRIO) + 1)

#define BLINKY_TASK_PRIO (MYNEWT_VAL(OS_MAIN_TASK_PRIO) + 2)
#define BLINKY_STACK_SIZE OS_STACK_ALIGN (64)
#define BLINKY_LED (LED_2)
#define RUNNING_LED (LED_BLINK_PIN)

// GATT server.
// Locally define as it seems impossible to include the following file:
//#include "nimble/host/services/ans/include/services/ans/ble_svc_ans.h"
// ... and use BLE_SVC_ANS_UUID16
#define GATT_SVR_SVC_ALERT_UUID 0x1811


static void blinky_task_init(void);
static void blinky_task_func(void *arg);
static void set_connection_state(bool aIsConnected);
static bool get_connection_state(void);

static void print_addr(const void *addr);
static void print_conn_desc(struct ble_gap_conn_desc *desc);

static void ble_on_reset(int reason);
static void ble_on_sync(void);
static void ble_set_addr(void);
static void ble_advertise(void);
static int ble_gap_event(struct ble_gap_event *event, void *arg);

static struct os_task blinky_task;
//static os_stack_t blinky_task_stack[BLINKY_STACK_SIZE];
OS_TASK_STACK_DEFINE(blinky_task_stack, BLINKY_STACK_SIZE);

static struct os_mutex connect_state_mutex;
static bool connect_state = false;


int main(int argc, char **argv)
{
    // Initialize all packages.
    hal_gpio_init_out(RUNNING_LED, 1);

    // Initialize OS.
    sysinit();

    // Initialize mutex.
    os_error_t err = os_mutex_init(&connect_state_mutex);
    assert(err == 0);

    // Initialize the NimBLE host configuration.
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Initialize the GATT server.
    int rc = gatt_svr_init();
    assert(rc == 0);

    // Set the default device name.
    rc = ble_svc_gap_device_name_set("PFPP");
    assert(rc == 0);

    // Initialize the blinky task.
    blinky_task_init();

    // Initialize the SPI command task.
    spi_cmd_init(SPI_CMD_TASK_PRIO);

    // As the last thing, process events from default event queue.
    while (1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }

    return 0;
}


static void blinky_task_init(void)
{
    // Initialize the blinky task.
    os_task_init(
        &blinky_task,
        "blinky_task",
        blinky_task_func,
        NULL,
        BLINKY_TASK_PRIO,
        OS_WAIT_FOREVER,
        blinky_task_stack,
        BLINKY_STACK_SIZE
    );
}


static void blinky_task_func(void *arg)
{
    hal_gpio_init_out(BLINKY_LED, 1);
    while (1)
    {
        // Wait one second.
        os_time_delay(OS_TICKS_PER_SEC * 1);

        if (get_connection_state())
        {
            // Keep LED lit.
            hal_gpio_write(BLINKY_LED, 1);
        }
        else
        {
            // Toggle the LED.
            hal_gpio_toggle(BLINKY_LED);
        }
    }
}


static void set_connection_state(bool aIsConnected)
{
    os_mutex_pend(&connect_state_mutex, OS_TIMEOUT_NEVER);
    connect_state = aIsConnected;
    os_mutex_release(&connect_state_mutex);
}


static bool get_connection_state(void)
{
    os_mutex_pend(&connect_state_mutex, OS_TIMEOUT_NEVER);
    bool l_connect_state = connect_state;
    os_mutex_release(&connect_state_mutex);

    return l_connect_state;
}


//
// Logs information about a connection to the console.
//
static void print_conn_desc(struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(
        INFO,
        "handle=%d our_ota_addr_type=%d our_ota_addr=",
        desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);

    MODLOG_DFLT(
        INFO,
        " our_id_addr_type=%d our_id_addr=",
        desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);

    MODLOG_DFLT(
        INFO,
        " peer_ota_addr_type=%d peer_ota_addr=",
        desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);

    MODLOG_DFLT(
        INFO,
        " peer_id_addr_type=%d peer_id_addr=",
        desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);

    MODLOG_DFLT(
        INFO,
        " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
        "encrypted=%d authenticated=%d bonded=%d\n",
        desc->conn_itvl, desc->conn_latency,
        desc->supervision_timeout,
        desc->sec_state.encrypted,
        desc->sec_state.authenticated,
        desc->sec_state.bonded);
}


static void print_addr(const void *addr)
{
    const uint8_t * const u8p = addr;
    MODLOG_DFLT(
        INFO,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}


static void ble_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}


static void ble_on_sync(void)
{
    // Make sure we have proper identity address set (public preferred)
    //int rc = ble_hs_util_ensure_addr(0);
    //assert(rc == 0);
    // Generate a non-resolvable private address.
    ble_set_addr();

    // Begin advertising.
    ble_advertise();
}


static void ble_set_addr(void)
{
    ble_addr_t addr = {0};
    int rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}


static void ble_advertise(void)
{
    // Figure out address to use while advertising (no privacy for now).
    uint8_t own_addr_type = 0;
    int rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    //  Set the advertisement data included in our advertisements:
    //     o Flags (indicates advertisement type and other general info).
    //     o Advertising tx power.
    //     o Device name.
    //     o service UUID.
    struct ble_hs_adv_fields fields = {0};
    //memset(&fields, 0, sizeof fields);

    // Advertise two flags:
    //     o Discoverability in forthcoming advertisement (general)
    //     o BLE-only (BR/EDR unsupported).
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // Indicate that the TX power level field should be included;
    // have the stack fill this value automatically.
    // This is done by assiging the special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    const char *name = NULL;
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[])
    {
        BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        DFLT_LOG_ERROR("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    // Begin advertising.
    struct ble_gap_adv_params adv_params = {0};
    //memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(
        own_addr_type /*BLE_OWN_ADDR_PUBLIC*/,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        ble_gap_event,
        NULL);
    if (rc != 0)
    {
        DFLT_LOG_ERROR("error enabling advertisement; rc=%d\n", rc);
    }
}


//
// The nimble host executes this callback when a GAP event occurs.  The
// application associates a GAP event callback with each connection that forms.
// bleprph uses the same callback for all connections.
//
// @param event                 The type of event being signalled.
// @param arg                   Application-specified argument; unuesd by
//                                  bleprph.
//
// @return                      0 if the application successfully handled the
//                                  event; nonzero on failure.  The semantics
//                                  of the return code is specific to the
//                                  particular GAP event being signalled.
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc = {0};
    int rc = 0;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        // A new connection was established or a connection attempt failed.
        MODLOG_DFLT(
            INFO,
            "connection %s; status=%d\n",
            event->connect.status == 0 ? "established" : "failed",
            event->connect.status);
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            print_conn_desc(&desc);
        }

        if (event->connect.status != 0)
        {
            // Connection failed: resume advertising.
            ble_advertise();
        }
        // Signal connection is established.
        set_connection_state(true);
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d\n", event->disconnect.reason);
        print_conn_desc(&event->disconnect.conn);

        // Connection terminated; resume advertising.
        set_connection_state(false);
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        // The central has updated the connection parameters.
        MODLOG_DFLT(INFO, "connection updated; status=%d\n", event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);
        print_conn_desc(&desc);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT(INFO, "advertise complete; reason=%d\n", event->adv_complete.reason);
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        // Encryption has been enabled or disabled for this connection.
        MODLOG_DFLT(INFO, "encryption change event; status=%d\n", event->enc_change.status);
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
        assert(rc == 0);
        print_conn_desc(&desc);
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        MODLOG_DFLT(
            INFO,
            "subscribe event; conn_handle=%d attr_handle=%d "
            "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
            event->subscribe.conn_handle,
            event->subscribe.attr_handle,
            event->subscribe.reason,
            event->subscribe.prev_notify,
            event->subscribe.cur_notify,
            event->subscribe.prev_indicate,
            event->subscribe.cur_indicate);
        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(
            INFO,
            "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
            event->mtu.conn_handle,
            event->mtu.channel_id,
            event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        // We already have a bond with the peer, but it is attempting to establish a new secure link.
        // This app sacrifices security for convenience:
        // just throw away the old bond and accept the new link.

        // Delete the old bond.
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);

        // Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
        // continue with the pairing operation.
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    return 0;
}
