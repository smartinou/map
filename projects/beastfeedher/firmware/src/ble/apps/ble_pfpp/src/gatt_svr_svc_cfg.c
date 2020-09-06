#include <string.h>

// BLE
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

// This file
#include "gatt_svr_svc_cfg.h"


struct cfg sCfg = 
{
    .CurrentTime =
    {
        .Hours = 12,
        .Minutes = 0,
        .Seconds = 0
    },
    .CurrentDate =
    {
        .Year = 00,
        .Month = 1,
        .Date = 1,
    },
    .Feeding =
    {
        .Time = 3,
        .PadEnable = false,
        .TimesOfDay = {0}
    }
};

// Config service UUID: 442386d5-af0d-46fc-ac45-8f90dbf45bb1
static const ble_uuid128_t gatt_svr_svc_cfg_uuid = 
    BLE_UUID128_INIT(
        0xb1, 0x5b, 0xf4, 0xdb, 0x90, 0x8f,
        0x45, 0xac,
        0xfc, 0x46,
        0x0d, 0xaf,
        0xd5, 0x86, 0x23, 0x44
    );

// Current date UUID: 4bf552a0-7049-48f5-9345-086c39365c0c
static const ble_uuid128_t gatt_svr_chr_cfg_current_date_uuid = 
    BLE_UUID128_INIT(
        0x0c, 0x5c, 0x36, 0x39, 0x6c, 0x08,
        0x45, 0x93,
        0xf5, 0x48,
        0x49, 0x70,
        0xa0, 0x52, 0xf5, 0x4b
    );

// Current time UUID: 0bb1bec6-fcdb-4b2a-a1e3-0a5812df04a5
static const ble_uuid128_t gatt_svr_chr_cfg_current_time_uuid = 
    BLE_UUID128_INIT(
        0xa5, 0x04, 0xdf, 0x12, 0x58, 0x0a,
        0x3e, 0xa1,
        0x2a, 0x4b,
        0xdb, 0xfc,
        0xc6, 0xbe, 0xb1, 0x0b
    );

// Feeding time UUID: bfd10a78-6f84-414c-8e39-d56da785ae5d
static const ble_uuid128_t gatt_svr_chr_cfg_feeding_time_uuid = 
    BLE_UUID128_INIT(
        0x5d, 0xae, 0x85, 0xa7, 0x6d, 0xd5,
        0x39, 0x8e,
        0x4c, 0x41,
        0x84, 0x6f,
        0x78, 0x0a, 0xd1, 0xbf
    );

// Feeding calendar UUID: cfceb82c-24e9-4744-94c2-831633bdf442
static const ble_uuid128_t gatt_svr_chr_cfg_feeding_calendar_uuid = 
    BLE_UUID128_INIT(
        0x42, 0xf4, 0xbd, 0x33, 0x16, 0x83,
        0xc2, 0x94,
        0x44, 0x47,
        0xe9, 0x24,
        0x2c, 0xb8, 0xce, 0xcf
    );


static int gatt_svr_chr_access_cfg(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg);


static struct ble_gatt_svc_def gatt_svr_cfg_svc[] =
{
    {
        // Service: Configuration.
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_cfg_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /*** Characteristic: current date. */
                .uuid      = &gatt_svr_chr_cfg_current_date_uuid.u,
                .access_cb = gatt_svr_chr_access_cfg,
                .flags     = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                /*** Characteristic: current time. */
                .uuid      = &gatt_svr_chr_cfg_current_time_uuid.u,
                .access_cb = gatt_svr_chr_access_cfg,
                .flags     = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                /*** Characteristic: feeding time. */
                .uuid      = &gatt_svr_chr_cfg_feeding_time_uuid.u,
                .access_cb = gatt_svr_chr_access_cfg,
                .flags     = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                /*** Characteristic: feeding calendar. */
                .uuid      = &gatt_svr_chr_cfg_feeding_calendar_uuid.u,
                .access_cb = gatt_svr_chr_access_cfg,
                .flags     = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                0, /* No more characteristics in this service. */
            }
        }
    },
    {
        0, /* No more services. */
    },
};


int gatt_svr_svc_cfg_set(struct cfg *Cfg)
{
    return 0;
}


int gatt_svr_svc_cfg_init(void)
{
    int rc = ble_gatts_count_cfg(gatt_svr_cfg_svc);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_cfg_svc);
    SYSINIT_PANIC_ASSERT(rc == 0);
    return rc;
}


static int gatt_svr_chr_access_cfg(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg)
{
    // Determine which characteristic is being accessed by examining its 128-bit UUID.
    int rc = 0;
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    if (ble_uuid_cmp(uuid, &gatt_svr_chr_cfg_current_date_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        rc = os_mbuf_append(ctxt->om, &sCfg.CurrentDate, sizeof(sCfg.CurrentDate));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return 0;    
}


