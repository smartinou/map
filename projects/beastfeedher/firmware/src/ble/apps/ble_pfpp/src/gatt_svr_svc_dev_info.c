#include <string.h>

// BLE
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

// This file
#include "gatt_svr_svc_dev_info.h"


static int gatt_svr_chr_access_dev_info(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg);


struct device_info sDeviceInfo = {
    .mFirmwareVersion = "1.0.0",
    .BuildTime = "11h11:11",
    .BuildDate = "2020-08-05",
    .GitHash = "githash",
    .DataBaseStatus = "good/bad",
    .Temperature = (float)22.25,
};

// Device info service UUID: 64b2aede-6f66-4728-875d-40191b59e664
static const ble_uuid128_t gatt_svr_svc_dev_info_uuid = 
    BLE_UUID128_INIT(
        0x64, 0xe6, 0x59, 0x1b, 0x19, 0x40,
        0x5d, 0x87,
        0x28, 0x47,
        0x66, 0x6f,
        0xde, 0xae, 0xb2, 0x64
    );

// Characteristic: Firmware version (maps to Firmware Revision String).
static const ble_uuid16_t gatt_svr_chr_dev_info_fw_version_uuid = BLE_UUID16_INIT(0x2A26);

// Build date UUID: ff8a4fcf-9838-4ea7-9c77-d7c2e5e6e49f
static const ble_uuid128_t gatt_svr_chr_dev_info_build_date_uuid =
    BLE_UUID128_INIT(
        0x9f, 0xe4, 0xe6, 0xe5,0xc2, 0xd7,
        0x77, 0x9c,
        0xa7, 0x4e,
        0x38, 0x98,
        0xcf, 0x4f, 0x8a, 0xff
    );

// Build time UUID: b9031fac-531c-4320-bb5c-9d38a8e9aaef
static const ble_uuid128_t gatt_svr_chr_dev_info_build_time_uuid =
    BLE_UUID128_INIT(
        0xef, 0xaa, 0xe9, 0xa8, 0xa8, 0x9d,
        0x5c, 0xbb,
        0x20, 0x43,
        0x1c, 0x53,
        0xac, 0x1f, 0x03, 0xb9
    );

// Build date UUID: 8bdaac18-a346-4e7b-bf09-1169dc851feb
static const ble_uuid128_t gatt_svr_chr_dev_info_git_hash_uuid =
    BLE_UUID128_INIT(
        0xeb, 0x1f, 0x85, 0xdc, 0x69, 0x11,
        0x09, 0xbf,
        0x7b, 0x4e,
        0x46, 0xa3,
        0x18, 0xac, 0xda, 0x8b
    );

// Database status UUID: 7ea11641-8740-4975-b1a0-9bec65ed6f3b
static const ble_uuid128_t gatt_svr_chr_dev_info_database_status_uuid =
    BLE_UUID128_INIT(
        0x3b, 0x6f, 0xed, 0x65, 0xec, 0x9b,
        0xa0, 0xb1,
        0x75, 0x49,
        0x40, 0x87,
        0x41, 0x16, 0xa1, 0x7e
    );

// Temperatur UUID: 6665eeb7-9326-4659-ada0-89428b87e921
// TODO: PUT BACK 16BIT UUID FOR TEMPERATURE C.
static const ble_uuid128_t gatt_svr_chr_dev_info_temperature_uuid =
    BLE_UUID128_INIT(
        0x21, 0xe9, 0x87, 0x8b, 0x42, 0x89,
        0xa0, 0xad,
        0x59, 0x46,
        0x26, 0x93,
        0xb7, 0xee, 0x65, 0x66
    );


static struct ble_gatt_svc_def gatt_svr_svcs_dev_info[] =
{
    {
        // Service: Device info.
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_dev_info_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                // Characteristic: Firmware version (maps to Firmware Revision String).
                .uuid = &gatt_svr_chr_dev_info_fw_version_uuid.u,//BLE_UUID16_DECLARE(0x2A26),
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ,// | BLE_GATT_CHR_F_READ_ENC,
            },
            {
                // Characteristic: Build date.
                .uuid = &gatt_svr_chr_dev_info_build_date_uuid.u,
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                // Characteristic: Build time.
                .uuid = &gatt_svr_chr_dev_info_build_time_uuid.u,//BLE_UUID16_DECLARE(0x2A08),
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                // Characteristic: git hash (maps to Database Hash).
                .uuid = &gatt_svr_chr_dev_info_git_hash_uuid.u, //BLE_UUID16_DECLARE(0x2B2A),
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            },
            {
                // Characteristic: database status.
                .uuid = &gatt_svr_chr_dev_info_database_status_uuid.u,//BLE_UUID16_DECLARE(0x1234),
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
            },
            {
                // Characteristic: temperature (maps to Temperature Celsius).
                .uuid = &gatt_svr_chr_dev_info_temperature_uuid.u,//BLE_UUID16_DECLARE(0x2A1F),
                .access_cb = gatt_svr_chr_access_dev_info,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
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


int gatt_svr_svc_dev_info_init(void)
{
    int rc = ble_gatts_count_cfg(gatt_svr_svcs_dev_info);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_svcs_dev_info);
    SYSINIT_PANIC_ASSERT(rc == 0);

    return rc;
}


int gatt_svr_svc_dev_info_set(struct device_info *DevInfo)
{
    memcpy(&sDeviceInfo, DevInfo, sizeof(struct device_info));
    return 0;
}


static int gatt_svr_chr_access_dev_info(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg)
{
    // Determine which characteristic is being accessed by examining its 128-bit UUID.
    int rc = 0;
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    if (ble_uuid_cmp(uuid, &gatt_svr_chr_dev_info_fw_version_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        // Respond with Firmware Version.
        size_t len = strnlen(&sDeviceInfo.mFirmwareVersion[0], 32);
        rc = os_mbuf_append(ctxt->om, &sDeviceInfo.mFirmwareVersion[0], len);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_chr_dev_info_build_time_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        // Respond with Build Date.
        size_t len = strnlen(&sDeviceInfo.BuildDate[0], 32);
        rc = os_mbuf_append(ctxt->om, &sDeviceInfo.BuildDate[0], len);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_chr_dev_info_git_hash_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        // Respond with Git Hash.
        size_t len = strnlen(&sDeviceInfo.GitHash[0], 32);
        rc = os_mbuf_append(ctxt->om, &sDeviceInfo.GitHash[0], len);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return 0;    
}
