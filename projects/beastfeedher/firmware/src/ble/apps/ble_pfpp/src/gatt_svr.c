// BLE
#include "nimble/ble.h"
#include "host/ble_hs.h"

#include "gatt_svr_svc_dev_info.h"
#include "gatt_svr_svc_cfg.h"
#include "gatt_svr_svc_stats.h"

// This file
#include "gatt_svr.h"


int gatt_svr_init(void)
{
    int rc = gatt_svr_svc_dev_info_init();
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = gatt_svr_svc_cfg_init();
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = gatt_svr_svc_stats_init();
    SYSINIT_PANIC_ASSERT(rc == 0);

    return rc;
}


void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN] = {};

    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        DFLT_LOG_DEBUG(
            "registered service %s with handle=%d\n",
            ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
            ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        DFLT_LOG_DEBUG(
            "registering characteristic %s with def_handle=%d val_handle=%d\n",
            ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
            ctxt->chr.def_handle,
            ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        DFLT_LOG_DEBUG(
            "registering descriptor %s with handle=%d\n",
            ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
            ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}
