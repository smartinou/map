#include <string.h>

// BLE
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

// This file
#include "gatt_svr_svc_stats.h"


struct stats sStats = {0};

// Stats service UUID: 9064b6f1-3159-4611-829c-9679a8aad922
static const ble_uuid128_t gatt_svr_svc_stats_uuid = 
    BLE_UUID128_INIT(
        0x22, 0xd9, 0xaa, 0xa8, 0x79, 0x96,
        0x9c, 0x82,
        0x11, 0x46,
        0x59, 0x31,
        0xf1, 0xb6, 0x64, 0x90
    );

// Packets sent UUID: 45019cef-61fb-4c8d-afb3-74e9dffd9fc0
static const ble_uuid128_t gatt_svr_chr_stats_packets_sent_uuid = 
    BLE_UUID128_INIT(
        0x0c, 0x9f, 0xfd, 0xdf, 0xe9, 0x74,
        0xb3, 0xaf,
        0x8d, 0x4c,
        0xfb, 0x61,
        0xef, 0x9c, 0x01, 0x45
    );

// Packets receied UUID: 045de9fa-b04e-46ab-92ad-a8ee9bf3f876
static const ble_uuid128_t gatt_svr_chr_stats_packets_received_uuid = 
    BLE_UUID128_INIT(
        0x76, 0xf8, 0xf3, 0x9b, 0xee, 0xa8,
        0xad, 0x92,
        0xab, 0x46,
        0x4e, 0xb0,
        0xfa, 0xe9, 0x5d, 0x04
    );

// Packets forwarded UUID: 60a46190-dd56-4de9-abfd-f631791ad4e5
static const ble_uuid128_t gatt_svr_chr_stats_packets_forwarded_uuid =
    BLE_UUID128_INIT(
        0xe5, 0xd4, 0x1a, 0x79, 0x31, 0xf6,
        0xfd, 0xab,
        0xe9, 0x4d,
        0x56, 0xdd,
        0x90, 0x61, 0xa4, 0x60
    );


static int gatt_svr_chr_access_stats(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg);


static struct ble_gatt_svc_def gatt_svr_stats_svc[] =
{
    {
        // Service: Statistics.
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_stats_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /*** Characteristic: packets sent. */
                .uuid      = &gatt_svr_chr_stats_packets_sent_uuid.u,
                .access_cb = gatt_svr_chr_access_stats,
                .flags     = BLE_GATT_CHR_F_READ,
            },
            {
                /*** Characteristic: packets received. */
                .uuid      = &gatt_svr_chr_stats_packets_received_uuid.u,
                .access_cb = gatt_svr_chr_access_stats,
                .flags     = BLE_GATT_CHR_F_READ,
            },
            {
                /*** Characteristic: packets forwarded. */
                .uuid      = &gatt_svr_chr_stats_packets_forwarded_uuid.u,
                .access_cb = gatt_svr_chr_access_stats,
                .flags     = BLE_GATT_CHR_F_READ,
            },
        },
    },
    {
        0, // No more services.
    },
};


int gatt_svr_svc_stats_set(struct stats *Stats)
{
    memcpy(&sStats, Stats, sizeof(struct stats));
    return 0;
}


int gatt_svr_svc_stats_init(void)
{
    int rc = ble_gatts_count_cfg(gatt_svr_stats_svc);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_stats_svc);
    SYSINIT_PANIC_ASSERT(rc == 0);
    return rc;
}


static int gatt_svr_chr_access_stats(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg)
{
    // Determine which characteristic is being accessed by examining its 128-bit UUID.
    int rc = 0;
    const ble_uuid_t *uuid = ctxt->chr->uuid;
    if (ble_uuid_cmp(uuid, &gatt_svr_chr_stats_packets_sent_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        rc = os_mbuf_append(ctxt->om, &sStats.NetStats.PacketsSent, sizeof(sStats.NetStats.PacketsSent));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_chr_stats_packets_received_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        rc = os_mbuf_append(ctxt->om, &sStats.NetStats.PacketsReceived, sizeof(sStats.NetStats.PacketsReceived));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_chr_stats_packets_forwarded_uuid.u) == 0)
    {
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        rc = os_mbuf_append(ctxt->om, &sStats.NetStats.PacketsForwarded, sizeof(sStats.NetStats.PacketsForwarded));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    // Unknown characteristic: the nimble stack should not have called this function.
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}
