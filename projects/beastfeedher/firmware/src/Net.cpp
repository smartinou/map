// *****************************************************************************
//
// Project: PFPP
//
// Module: Network.
//
// *****************************************************************************

//! \file
//! \brief Network class.
//! \ingroup application

// *****************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// Standard Library.
#include <string.h>

// Common Library.
#include <db/CalendarRec.h>
#include <db/FeedCfgRec.h>
#include <db/NetIFRec.h>

// Util Library.
#include <date/Date.h>
#include <time/Time.h>
#include <net/EthernetAddress.h>
#include <net/IPAddress.h>

// LwIP stack.
#include "lwip/apps/httpd.h"
#include "lwip/stats.h"

// AOs.
#include "LwIPDrv.h"
#include "PFPP_Events.h"
#include "RTCC_AOs.h"
#include "RTCC_Events.h"

// This project.
#include "FWVersionGenerated.h"
#include "LwIP_AOs.h"
#include "LwIP_Events.h"
#include "Signals.h"

#include "Net.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

enum {
    SSI_TAG_IX_ZERO = 0,
    SSI_TAG_IX_EMPTY,

    // Info.
    SSI_TAG_IX_INFO_FIRST,
    SSI_TAG_IX_INFO_FW_VERSION = SSI_TAG_IX_INFO_FIRST,
    SSI_TAG_IX_INFO_BUILD_DATE,
    SSI_TAG_IX_INFO_BUILD_TIME,
    SSI_TAG_IX_INFO_GIT_HASH,
    SSI_TAG_IX_INFO_DB_STATUS,
    SSI_TAG_IX_INFO_RTCC_TEMP,
    SSI_TAG_IX_INFO_LAST = SSI_TAG_IX_INFO_RTCC_TEMP,

    // Global: Time and Date.
    SSI_TAG_IX_CFG_GLOBAL_DATE,
    SSI_TAG_IX_CFG_GLOBAL_TIME,

    // Configuration.
    SSI_TAG_IX_CFG_FIRST,
    SSI_TAG_IX_CFG_PAD_ENABLE = SSI_TAG_IX_CFG_FIRST,
    SSI_TAG_IX_CFG_PAD_DISABLE,
    SSI_TAG_IX_CFG_LAST = SSI_TAG_IX_CFG_PAD_DISABLE,

    SSI_TAG_IX_CFG_FEED_TIME,

    // Calendar.
    SSI_TAG_IX_CFG_CALENDAR_FIRST,
    SSI_TAG_IX_CFG_CALENDAR_06_00 = SSI_TAG_IX_CFG_CALENDAR_FIRST,
    SSI_TAG_IX_CFG_CALENDAR_07_00,
    SSI_TAG_IX_CFG_CALENDAR_08_00,
    SSI_TAG_IX_CFG_CALENDAR_09_00,
    SSI_TAG_IX_CFG_CALENDAR_10_00,
    SSI_TAG_IX_CFG_CALENDAR_11_00,
    SSI_TAG_IX_CFG_CALENDAR_12_00,
    SSI_TAG_IX_CFG_CALENDAR_13_00,
    SSI_TAG_IX_CFG_CALENDAR_14_00,
    SSI_TAG_IX_CFG_CALENDAR_15_00,
    SSI_TAG_IX_CFG_CALENDAR_16_00,
    SSI_TAG_IX_CFG_CALENDAR_17_00,
    SSI_TAG_IX_CFG_CALENDAR_18_00,
    SSI_TAG_IX_CFG_CALENDAR_19_00,
    SSI_TAG_IX_CFG_CALENDAR_20_00,
    SSI_TAG_IX_CFG_CALENDAR_21_00,
    SSI_TAG_IX_CFG_CALENDAR_LAST = SSI_TAG_IX_CFG_CALENDAR_21_00,

    // Network configuration.
    SSI_TAG_IX_NET_MAC_ADDR,
    SSI_TAG_IX_NET_IPV4_ADD,
    SSI_TAG_IX_NET_SUBNET_MASK,
    SSI_TAG_IX_NET_GW_ADD,

    SSI_TAG_IX_NET_USE_DHCP,
    SSI_TAG_IX_NET_USE_MANUAL,
    SSI_TAG_IX_NET_STATIC_FIRST,
    SSI_TAG_IX_NET_STATIC_IPV4_ADD_0 = SSI_TAG_IX_NET_STATIC_FIRST,
    SSI_TAG_IX_NET_STATIC_IPV4_ADD_1,
    SSI_TAG_IX_NET_STATIC_IPV4_ADD_2,
    SSI_TAG_IX_NET_STATIC_IPV4_ADD_3,
    SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0,
    SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1,
    SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2,
    SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3,
    SSI_TAG_IX_NET_STATIC_GW_ADD_0,
    SSI_TAG_IX_NET_STATIC_GW_ADD_1,
    SSI_TAG_IX_NET_STATIC_GW_ADD_2,
    SSI_TAG_IX_NET_STATIC_GW_ADD_3,
    SSI_TAG_IX_NET_STATIC_LAST = SSI_TAG_IX_NET_STATIC_GW_ADD_3,

    // Network statistics.
    SSI_TAG_IX_STATS_FIRST,
    SSI_TAG_IX_STATS_TX = SSI_TAG_IX_STATS_FIRST,
    SSI_TAG_IX_STATS_RX,
    SSI_TAG_IX_STATS_FW,
    SSI_TAG_IX_STATS_DROP,
    SSI_TAG_IX_STATS_CHK_ERR,
    SSI_TAG_IX_STATS_LEN_ERR,
    SSI_TAG_IX_STATS_MEM_ERR,
    SSI_TAG_IX_STATS_RT_ERR,
    SSI_TAG_IX_STATS_PRO_ERR,
    SSI_TAG_IX_STATS_OPT_ERR,
    SSI_TAG_IX_STATS_ERR,
    SSI_TAG_IX_STATS_LAST = SSI_TAG_IX_STATS_ERR,

    // Network interface.
    SSI_TAG_IX_LAST = SSI_TAG_IX_STATS_ERR,
    SSI_TAG_IX_STATS_QTY
};

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

namespace Net {

#if LWIP_HTTPD_SSI
static uint16_t SSIHandler(int aTagIx, char *aInsertStr, int aInsertStrLen);

static int SSIRadioButtonHandler(
    int                aTagIx,
    char       * const aInsertStr,
    int                aInsertStrLen,
    char const * const aNameValStr,
    bool               aIsChecked
);

static int SSICalendarHandler(
    int          aTagIx,
    char * const aInsertStr,
    int          aInsertStrLen,
    unsigned int aHour
);

static int FullIPAddressHandler(char * const aInsertStr, IPAddress const &aAddress);

static int SSINetworkHandler(
    int                aTagIx,
    char       * const aInsertStr,
    int                aInsertStrLen,
    char const * const aTagNameStr,
    uint8_t aValue
);

static int SSIStatsHandler(
    int          aTagIx,
    char * const aInsertStr,
    int          aInsertStrLen
);

static char const *FindTagVal(
    char const * const aTagNameStr,
    int                aParamsQty,
    char const * const aParamsVec[],
    char const * const aValsVec[]
);
#endif // LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
static char const *DispIndex(
    int   aIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
);

static char const *DispCfg(
    int   aIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
);

static char const *DispNet(
    int   aIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
);
#endif // LWIP_HTTPD_CGI

} // namespace Net

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

namespace Net {

#if LWIP_HTTPD_SSI || LWIP_HTTPD_CGI
// QP AOs.
static std::shared_ptr<RTCC::AO::RTCC_AO> sRTCC_AO(nullptr);
#endif

#if LWIP_HTTPD_SSI
// Server-Side Include (SSI) demo.
static char const * sSSITags[] = {
    // Common/misc. tags.
    "_zero",     // SSI_TAG_IX_ZERO
    "_empty",    // SSI_TAG_IX_EMPTY

    // Info.
    "i_ver",     // SSI_TAG_IX_INFO_FW_VERSION
    "i_date",    // SSI_TAG_IX_INFO_BUILD_DATE
    "i_time",    // SSI_TAG_IX_INFO_BUILD_TIME
    "i_hash",    // SSI_TAG_IX_INFO_GIT_HASH
    "i_status",  // SSI_TAG_IX_INFO_DB_STATUS
    "i_temp",    // SSI_TAG_IX_INFO_RTCC_TEMP

    // Global: Time and Date.
    "g_date",    // SSI_TAG_IX_CFG_GLOBAL_DATE
    "g_time",    // SSI_TAG_IX_CFG_GLOBAL_TIME

    // Configuration.
    "c_pad_en",  // SSI_TAG_IX_CFG_PAD_ENABLE
    "c_pad_di",  // SSI_TAG_IX_CFG_PAD_DISABLE
    "c_time",    // SSI_TAG_IX_CFG_FEED_TIME
    "c_cal_06",  // SSI_TAG_IX_CFG_CALENDAR_06_00
    "c_cal_07",  // SSI_TAG_IX_CFG_CALENDAR_07_00
    "c_cal_08",  // SSI_TAG_IX_CFG_CALENDAR_09_00
    "c_cal_09",  // SSI_TAG_IX_CFG_CALENDAR_09_00
    "c_cal_10",  // SSI_TAG_IX_CFG_CALENDAR_10_00
    "c_cal_11",  // SSI_TAG_IX_CFG_CALENDAR_11_00
    "c_cal_12",  // SSI_TAG_IX_CFG_CALENDAR_12_00
    "c_cal_13",  // SSI_TAG_IX_CFG_CALENDAR_13_00
    "c_cal_14",  // SSI_TAG_IX_CFG_CALENDAR_14_00
    "c_cal_15",  // SSI_TAG_IX_CFG_CALENDAR_15_00
    "c_cal_16",  // SSI_TAG_IX_CFG_CALENDAR_16_00
    "c_cal_17",  // SSI_TAG_IX_CFG_CALENDAR_17_00
    "c_cal_18",  // SSI_TAG_IX_CFG_CALENDAR_18_00
    "c_cal_19",  // SSI_TAG_IX_CFG_CALENDAR_19_00
    "c_cal_20",  // SSI_TAG_IX_CFG_CALENDAR_20_00
    "c_cal_21",  // SSI_TAG_IX_CFG_CALENDAR_21_00

    // Network configuration.
    "n_mac",     // SSI_TAG_IX_NET_MAC_ADDR
    "n_ipv4",    // SSI_TAG_IX_NET_IPV4_ADD
    "n_subnet",  // SSI_TAG_IX_NET_SUBNET_MASK
    "n_dfltgw",  // SSI_TAG_IX_NET_GW_ADD
    "n_dhcp",    // SSI_TAG_IX_NET_USE_DHCP
    "n_manual",  // SSI_TAG_IX_NET_USE_MANUAL
    "n_sip0",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_0
    "n_sip1",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_1
    "n_sip2",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_2
    "n_sip3",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_3
    "n_ssn0",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0
    "n_ssn1",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1
    "n_ssn2",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2
    "n_ssn3",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3
    "n_sgw0",    // SSI_TAG_IX_NET_STATIC_GW_ADD_0
    "n_sgw1",    // SSI_TAG_IX_NET_STATIC_GW_ADD_1
    "n_sgw2",    // SSI_TAG_IX_NET_STATIC_GW_ADD_2
    "n_sgw3",    // SSI_TAG_IX_NET_STATIC_GW_ADD_3

    // Network statistics.
    "s_xmit",    // SSI_TAG_IX_STATS_TX
    "s_recv",    // SSI_TAG_IX_STATS_RX
    "s_fw",      // SSI_TAG_IX_STATS_FW
    "s_drop",    // SSI_TAG_IX_STATS_DROP
    "s_chkerr",  // SSI_TAG_IX_STATS_CHK_ERR
    "s_lenerr",  // SSI_TAG_IX_STATS_LEN_ERR
    "s_memerr",  // SSI_TAG_IX_STATS_MEM_ERR
    "s_rterr",   // SSI_TAG_IX_STATS_RT_ERR
    "s_proerr",  // SSI_TAG_IX_STATS_PRO_ERR
    "s_opterr",  // SSI_TAG_IX_STATS_OPT_ERR
    "s_err",     // SSI_TAG_IX_STATS_ERR
};

#endif // LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
static tCGI const sCGIEntries[] = {
    {"/index.cgi",  DispIndex},
    {"/config.cgi", DispCfg},
    {"/network.cgi", DispNet}
};

static CalendarRec *sCalendarRec = nullptr;
static NetIFRec *sNetIFRec = nullptr;
static FeedCfgRec *sFeedCfgRec = nullptr;

#endif // LWIP_HTTPD_CGI

} // namespace Net

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace Net {

    // Vraiment, ceci est plutot un setup de HTTP.
void InitCallback(
    std::shared_ptr<RTCC::AO::RTCC_AO> const aRTCC_AO,
    CalendarRec * const aCalendarRec,
    NetIFRec * const aNetIFRec,
    FeedCfgRec * const aFeedCfgRec
) {

    httpd_init();
#if LWIP_HTTPD_SSI
    http_set_ssi_handler(
        SSIHandler,
        Net::sSSITags,
        sizeof(Net::sSSITags)
    );
#endif // LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
    http_set_cgi_handlers(
        Net::sCGIEntries,
        Q_DIM(Net::sCGIEntries)
    );
#endif // LWIP_HTTPD_CGI

#if LWIP_HTTPD_SSI || LWIP_HTTPD_CGI
    sRTCC_AO = aRTCC_AO;
    sCalendarRec = aCalendarRec;
    sNetIFRec = aNetIFRec;
    sFeedCfgRec = aFeedCfgRec;
#else
    static_cast<void>(aRTCC_AO);
    static_cast<void *>(aCalendarRec);
    static_cast<void *>(aNetIFRec);
    static_cast<void *>(aFeedCfgRec);
#endif
}

} // namespace Net

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

namespace Net {

#if LWIP_HTTPD_SSI
// HTTPD customizations.
// Server-Side Include (SSI) handler.
static uint16_t SSIHandler(
    int   aTagIx,
    char *aInsertStr,
    int   aInsertStrLen
) {

    static constexpr size_t sDefaultNetIF = 0;

    switch (aTagIx) {
    case SSI_TAG_IX_ZERO:
        return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%u", 0);
    default:
    case SSI_TAG_IX_EMPTY:
        return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", "");

    // Info.
    case SSI_TAG_IX_INFO_FW_VERSION: {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s",
            FWVersionGenerated::VerStr
        );
    } break;
    case SSI_TAG_IX_INFO_BUILD_DATE: {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s",
            FWVersionGenerated::BuildDate
        );
    } break;
    case SSI_TAG_IX_INFO_BUILD_TIME: {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s",
            FWVersionGenerated::BuildTime
        );
    } break;
    case SSI_TAG_IX_INFO_GIT_HASH: {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s",
            FWVersionGenerated::GitHash
        );
    } break;
    case SSI_TAG_IX_INFO_DB_STATUS: {
        if (DBRec::GetDBRecCount() && DBRec::IsDBSane()) {
            return snprintf(
                aInsertStr,
                LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                "Passed"
            );
        } else {
            return snprintf(
                aInsertStr,
                LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                "Failed"
            );
        }
    } break;
    case SSI_TAG_IX_INFO_RTCC_TEMP: {
        float lTemperature = sRTCC_AO->GetTemperature();
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%2.2f",
            lTemperature
        );
    } break;

    // Global.
    case SSI_TAG_IX_CFG_GLOBAL_DATE: {
        static constexpr auto lDateInputStr =
            "<input type=\"date\" name=\"date\" min=\"2018-01-01\" value=\"";
        char lDateBuf[16]{0};
        Date const &lDate = sRTCC_AO->GetDate();
        char const * const lDateStr = DateHelper::ToStr(lDate, &lDateBuf[0]);
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%s\">",
            lDateInputStr,
            lDateStr
        );
    } break;

    case SSI_TAG_IX_CFG_GLOBAL_TIME: {
        static constexpr auto lTimeInputStr =
            "<input type=\"time\" name=\"time\" value=\"";
        char lTimeBuf[16]{0};
        Time const &lTime = sRTCC_AO->GetTime();
        char const * const lTimeStr = TimeHelper::ToStr(lTime, &lTimeBuf[0]);
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%s\">",
            lTimeInputStr,
            lTimeStr
        );
    } break;

    // Configuration.
    case SSI_TAG_IX_CFG_PAD_ENABLE: {
        return SSIRadioButtonHandler(
            aTagIx,
            aInsertStr,
            aInsertStrLen,
            "feeding_pad\" value=\"y\"",
            Net::sFeedCfgRec->IsTimedFeedEnable()
        );
    } break;
    case SSI_TAG_IX_CFG_PAD_DISABLE: {
        return SSIRadioButtonHandler(
            aTagIx,
            aInsertStr,
            aInsertStrLen,
            "feeding_pad\" value=\"n\"",
            !Net::sFeedCfgRec->IsTimedFeedEnable()
        );
    } break;
    case SSI_TAG_IX_CFG_FEED_TIME: {
        static constexpr auto sFeedingTimeStr =
            "<input type=\"number\" name=\"feeding_time_sec\" "
            "min=\"1\" max=\"10\" value=\"";
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%u\">",
            sFeedingTimeStr,
            Net::sFeedCfgRec->GetTimedFeedPeriod()
        );
    } break;

    // Calendar.
    case SSI_TAG_IX_CFG_CALENDAR_06_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 6);
    case SSI_TAG_IX_CFG_CALENDAR_07_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 7);
    case SSI_TAG_IX_CFG_CALENDAR_08_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 8);
    case SSI_TAG_IX_CFG_CALENDAR_09_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 9);
    case SSI_TAG_IX_CFG_CALENDAR_10_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 10);
    case SSI_TAG_IX_CFG_CALENDAR_11_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 11);
    case SSI_TAG_IX_CFG_CALENDAR_12_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 12);
    case SSI_TAG_IX_CFG_CALENDAR_13_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 13);
    case SSI_TAG_IX_CFG_CALENDAR_14_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 14);
    case SSI_TAG_IX_CFG_CALENDAR_15_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 15);
    case SSI_TAG_IX_CFG_CALENDAR_16_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 16);
    case SSI_TAG_IX_CFG_CALENDAR_17_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 17);
    case SSI_TAG_IX_CFG_CALENDAR_18_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 18);
    case SSI_TAG_IX_CFG_CALENDAR_19_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 19);
    case SSI_TAG_IX_CFG_CALENDAR_20_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 20);
    case SSI_TAG_IX_CFG_CALENDAR_21_00:
        return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 21);


    // Network configuration.
    case SSI_TAG_IX_NET_MAC_ADDR: {
        EthernetAddress const &lMACAddress = LwIPDrv::StaticGetMACAddress(sDefaultNetIF);
        char lMACAddressStr[32]{0};
        lMACAddress.GetString(&lMACAddressStr[0]);
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s",
            &lMACAddressStr[0]
        );
    } break;

    case SSI_TAG_IX_NET_IPV4_ADD:
        return FullIPAddressHandler(
            aInsertStr,
            LwIPDrv::StaticGetIPAddress(sDefaultNetIF)
        );
    case SSI_TAG_IX_NET_SUBNET_MASK:
        return FullIPAddressHandler(
            aInsertStr,
            LwIPDrv::StaticGetSubnetMask(sDefaultNetIF)
        );
    case SSI_TAG_IX_NET_GW_ADD:
        return FullIPAddressHandler(
            aInsertStr,
            LwIPDrv::StaticGetDefaultGW(sDefaultNetIF)
        );

    case SSI_TAG_IX_NET_USE_DHCP: {
        return SSIRadioButtonHandler(
            aTagIx,
            aInsertStr,
            aInsertStrLen,
            "use_dhcp\" value=\"y\"",
            sNetIFRec->UseDHCP()
        );
    } break;

    case SSI_TAG_IX_NET_USE_MANUAL: {
        return SSIRadioButtonHandler(
            aTagIx,
            aInsertStr,
            aInsertStrLen,
            "use_dhcp\" value=\"n\"",
            !sNetIFRec->UseDHCP()
        );
    } break;

    case SSI_TAG_IX_NET_STATIC_IPV4_ADD_0: {
        uint32_t lIPAddress = sNetIFRec->GetIPAddr();
        IPAddress lTempIPAddress(lIPAddress);
        uint8_t lByte = lTempIPAddress.GetByte(0);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_0", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_IPV4_ADD_1: {
        uint32_t lIPAddress = sNetIFRec->GetIPAddr();
        IPAddress lTempIPAddress(lIPAddress);
        uint8_t lByte = lTempIPAddress.GetByte(1);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_1", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_IPV4_ADD_2: {
        uint32_t lIPAddress = sNetIFRec->GetIPAddr();
        IPAddress lTempIPAddress(lIPAddress);
        uint8_t lByte = lTempIPAddress.GetByte(2);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_2", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_IPV4_ADD_3: {
        uint32_t lIPAddress = sNetIFRec->GetIPAddr();
        IPAddress lTempIPAddress(lIPAddress);
        uint8_t lByte = lTempIPAddress.GetByte(3);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_3", lByte);
    } break;

    case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0: {
        uint32_t lSubnetMask = sNetIFRec->GetSubnetMask();
        IPAddress lTempSubnetMask(lSubnetMask);
        uint8_t lByte = lTempSubnetMask.GetByte(0);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_0", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1: {
        uint32_t lSubnetMask = sNetIFRec->GetSubnetMask();
        IPAddress lTempSubnetMask(lSubnetMask);
        uint8_t lByte = lTempSubnetMask.GetByte(1);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_1", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2: {
        uint32_t lSubnetMask = sNetIFRec->GetSubnetMask();
        IPAddress lTempSubnetMask(lSubnetMask);
        uint8_t lByte = lTempSubnetMask.GetByte(2);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_2", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3: {
        uint32_t lSubnetMask = sNetIFRec->GetSubnetMask();
        IPAddress lTempSubnetMask(lSubnetMask);
        uint8_t lByte = lTempSubnetMask.GetByte(3);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_3", lByte);
    } break;

    case SSI_TAG_IX_NET_STATIC_GW_ADD_0: {
        uint32_t lGateway = sNetIFRec->GetGWAddr();
        IPAddress lTempGateway(lGateway);
        uint8_t lByte = lTempGateway.GetByte(0);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_0", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_GW_ADD_1: {
        uint32_t lGateway = sNetIFRec->GetGWAddr();
        IPAddress lTempGateway(lGateway);
        uint8_t lByte = lTempGateway.GetByte(1);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_1", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_GW_ADD_2: {
        uint32_t lGateway = sNetIFRec->GetGWAddr();
        IPAddress lTempGateway(lGateway);
        uint8_t lByte = lTempGateway.GetByte(2);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_2", lByte);
    } break;
    case SSI_TAG_IX_NET_STATIC_GW_ADD_3: {
        uint32_t lGateway = sNetIFRec->GetGWAddr();
        IPAddress lTempGateway(lGateway);
        uint8_t lByte = lTempGateway.GetByte(3);
        return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_3", lByte);
    } break;

    case SSI_TAG_IX_STATS_TX:
    case SSI_TAG_IX_STATS_RX:
    case SSI_TAG_IX_STATS_FW:
    case SSI_TAG_IX_STATS_DROP:
    case SSI_TAG_IX_STATS_CHK_ERR:
    case SSI_TAG_IX_STATS_LEN_ERR:
    case SSI_TAG_IX_STATS_MEM_ERR:
    case SSI_TAG_IX_STATS_RT_ERR:
    case SSI_TAG_IX_STATS_PRO_ERR:
    case SSI_TAG_IX_STATS_OPT_ERR:
    case SSI_TAG_IX_STATS_ERR: {
        // Sub-handler for network stats.
        STAT_COUNTER lVal = SSIStatsHandler(aTagIx, aInsertStr, aInsertStrLen);
        return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_NAME_LEN, "%u", lVal);
    } break;

    }

    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_NAME_LEN, "%u", 0);
}


static int SSIRadioButtonHandler(
    int                aTagIx,
    char       * const aInsertStr,
    int                aInsertStrLen,
    char const * const aNameValStr,
    bool               aIsChecked
) {

    static constexpr auto sInputRadioStr = "<input type=\"radio\" name=\"";
    if (aIsChecked) {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%s checked>",
            sInputRadioStr,
            aNameValStr
        );
    } else {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%s>",
            sInputRadioStr,
            aNameValStr
        );
    }
}


static int SSICalendarHandler(
    int          aTagIx,
    char * const aInsertStr,
    int          aInsertStrLen,
    unsigned int aHour
) {

    static constexpr auto sFeedingCalStr =
        "<input type=\"checkbox\" name=\"feed_time\" value=\"";

    const Time lTime(aHour, 0, 0, true, false);
    if (Net::sCalendarRec->IsEntrySet(lTime)) {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%02d\" checked>%02d:00",
            sFeedingCalStr,
            aHour,
            aHour
        );
    } else {
        return snprintf(
            aInsertStr,
            LWIP_HTTPD_MAX_TAG_INSERT_LEN,
            "%s%02d\">%02d:00",
            sFeedingCalStr,
            aHour,
            aHour
        );
    }
}


static int FullIPAddressHandler(char * const aInsertStr, IPAddress const &aAddress) {
    char lAddressStr[32]{0};
    aAddress.GetString(&lAddressStr[0]);
    return snprintf(
        aInsertStr,
        LWIP_HTTPD_MAX_TAG_INSERT_LEN,
        "%s",
        &lAddressStr[0]
    );
}


static int SSINetworkHandler(
    int                aTagIx,
    char       * const aInsertStr,
    int                aInsertStrLen,
    char const * const aTagNameStr,
    uint8_t            aValue
) {

    static constexpr auto sInputTagStr = "<input name=\"";
    static constexpr auto sInputValueStr =
        "\" type=\"text\" size=\"2\" maxlength=\"3\" value=\"";
    return snprintf(
        aInsertStr,
        LWIP_HTTPD_MAX_TAG_INSERT_LEN,
        "%s%s%s%u\">",
        sInputTagStr,
        aTagNameStr,
        sInputValueStr,
        aValue
    );
}


static int SSIStatsHandler(
    int          aTagIx,
    char * const aInsertStr,
    int          aInsertStrLen
) {

    struct stats_proto *lStatsPtr = &lwip_stats.link;

    switch (aTagIx) {
    case SSI_TAG_IX_STATS_TX:      return lStatsPtr->xmit;
    case SSI_TAG_IX_STATS_RX:      return lStatsPtr->recv;
    case SSI_TAG_IX_STATS_FW:      return lStatsPtr->fw;
    case SSI_TAG_IX_STATS_DROP:    return lStatsPtr->drop;
    case SSI_TAG_IX_STATS_CHK_ERR: return lStatsPtr->chkerr;
    case SSI_TAG_IX_STATS_LEN_ERR: return lStatsPtr->lenerr;
    case SSI_TAG_IX_STATS_MEM_ERR: return lStatsPtr->memerr;
    case SSI_TAG_IX_STATS_RT_ERR:  return lStatsPtr->rterr;
    case SSI_TAG_IX_STATS_PRO_ERR: return lStatsPtr->proterr;
    case SSI_TAG_IX_STATS_OPT_ERR: return lStatsPtr->opterr;
    case SSI_TAG_IX_STATS_ERR:     return lStatsPtr->err;
    default:                       return 0;
    }

    return 0;
}

#endif // LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
// HTTPD customizations.
// CGI handlers.
static char const *DispIndex(
    int   aCGIIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
) {

    if (0 == strcmp(aParamsVec[0], "timed_feed")) {
        // Param found.
        // Send event with value as parameter.
        unsigned int lTime = 0;
        sscanf(aValsVec[0], "%u", &lTime);
        PFPP::Event::Mgr::TimedFeedCmd *lEvtPtr = Q_NEW(
            PFPP::Event::Mgr::TimedFeedCmd,
            FEED_MGR_TIMED_FEED_CMD_SIG,
            lTime
        );

        // Could use QF_Publish() to decouple from active object.
        // Here, there's only this well-known recipient.
        //BFHMgr_AO::AOInstance().POST(lEvtPtr, 0);
#ifdef Q_SPY
        static QP::QSpyId const sNetDispIndex = {0U};
#endif // Q_SPY
        QP::QF::PUBLISH(lEvtPtr, &sNetDispIndex);

        // Return where we're coming from.
        return "/index.shtml";
    }

    return nullptr;
}


static char const *DispCfg(
    int   aCGIIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
) {

    // Try to find the Time and Date apply button.
    char const *lSubmitVal = FindTagVal(
        "set_time",
        aParamsQty,
        aParamsVec,
        aValsVec
    );
    if (0 == strcmp(lSubmitVal, "Apply")) {
        // There's only time & date widgets here.
        char const *lDateVal = FindTagVal(
            "date",
            aParamsQty,
            aParamsVec,
            aValsVec
        );

        char const *lTimeVal = FindTagVal(
            "time",
            aParamsQty,
            aParamsVec,
            aValsVec
        );

        if ((lDateVal != nullptr) && (lTimeVal != nullptr)) {
            unsigned int lYear  = 0;
            unsigned int lMonth = 0;
            unsigned int lDayDate  = 0;
            sscanf(lDateVal, "%u-%u-%u", &lYear, &lMonth, &lDayDate);
            const Date lDate(lYear, Month::UIToName(lMonth), lDayDate);

            unsigned int lHours   = 0;
            unsigned int lMinutes = 0;
            static constexpr unsigned int sSeconds = 0;
            sscanf(lTimeVal, "%u%%3A%u", &lHours, &lMinutes);
            const Time lTime(lHours, lMinutes, sSeconds);

            // Send event to write new time.
            // Send event to write new date.
            RTCC::Event::TimeAndDate *lEvtPtr = Q_NEW(
                RTCC::Event::TimeAndDate,
                RTCC_SET_TIME_AND_DATE_SIG,
                lTime,
                lDate
            );
            sRTCC_AO->POST(lEvtPtr, 0);
        }

        // Return where we're coming from.
        return "/config.shtml";
    }

    // Try to find the Config and Calendar apply button.
    lSubmitVal = FindTagVal(
        "set_cfg",
        aParamsQty,
        aParamsVec,
        aValsVec
    );
    if (0 == strcmp(lSubmitVal, "Apply")) {
        // Make sure the calendar is cleared before setting new entries.
        // Must be done only once!
        Net::sCalendarRec->ClrAllEntries();
        for (int lIx = 0; lIx < aParamsQty; ++lIx) {
            if (0 == strcmp(aParamsVec[lIx], "feeding_button")) {
                if ('y' == *aValsVec[lIx]) {
                    Net::sFeedCfgRec->SetTimedFeedEnabled(true);
                } else {
                    Net::sFeedCfgRec->SetTimedFeedEnabled(false);
                }
            } else if (0 == strcmp(aParamsVec[lIx], "feeding_time_sec")) {
                unsigned int lFeedingTime = 0;
                sscanf(aValsVec[lIx], "%u", &lFeedingTime);
                Net::sFeedCfgRec->SetTimedFeedPeriod(static_cast<uint8_t>(lFeedingTime));
            } else if (0 == strcmp(aParamsVec[lIx], "feed_time")) {
                unsigned int lHour = 0;
                sscanf(aValsVec[lIx], "%u", &lHour);
                const Time lTime(lHour, 0, 0);
                Net::sCalendarRec->SetTimeEntry(lTime);
            }
        }

        // Send event to trigger updated DB writing.
        static constexpr bool sIsDataImpure = true;
        RTCC::Event::SaveToRAM * const lSaveEvtPtr = Q_NEW(
            RTCC::Event::SaveToRAM,
            RTCC_SAVE_TO_NV_MEMORY_SIG,
            sIsDataImpure
        );
        sRTCC_AO->POST(lSaveEvtPtr, 0);
    }

    // Return where we're coming from.
    return "/config.shtml";
}


static char const *DispNet(
    int   aCGIIx,
    int   aParamsQty,
    char *aParamsVec[],
    char *aValsVec[]
) {
    // Try to find the apply button.
    char const * const lSubmitVal = FindTagVal("set_ip", aParamsQty, aParamsVec, aValsVec);
    if (0 == strcmp(lSubmitVal, "Apply")) {
        // Fields of the addresses have changed:
        // Read now and modify bytes.
        const uint32_t lIPAddress = sNetIFRec->GetIPAddr();
        IPAddress lNewIPAddress(lIPAddress);

        const uint32_t lSubnetMask = sNetIFRec->GetSubnetMask();
        IPAddress lNewSubnetMask(lSubnetMask);

        const uint32_t lGWAddress = sNetIFRec->GetGWAddr();
        IPAddress lNewGWAddress(lGWAddress);

        for (int lIx = 0; lIx < aParamsQty; ++lIx) {
            size_t lByteIx = 0;
            uint8_t lByteValue = 0;
            if (0 == strcmp(aParamsVec[lIx], "use_dhcp")) {
                if ('y' == *aValsVec[lIx]) {
                    sNetIFRec->SetUseDHCP(true);
                } else {
                    sNetIFRec->SetUseDHCP(false);
                }
            } else if (1 == sscanf(aValsVec[lIx], "%hhu", &lByteValue)) {
                // Everything else is a byte field of an IP address. Get it.
                if (1 == sscanf(aParamsVec[lIx], "ipaddr_%uz", &lByteIx)) {
                    lNewIPAddress.SetByte(lByteIx, lByteValue);
                    sNetIFRec->SetIPAddr(lNewIPAddress.GetValue());
                } else if (1 == sscanf(aParamsVec[lIx], "ipmask_%uz", &lByteIx)) {
                    lNewSubnetMask.SetByte(lByteIx, lByteValue);
                    sNetIFRec->SetSubnetMask(lNewSubnetMask.GetValue());
                } else if (1 == sscanf(aParamsVec[lIx], "ipgw_%uz", &lByteIx)) {
                    lNewGWAddress.SetByte(lByteIx, lByteValue);
                    sNetIFRec->SetGWAddr(lNewGWAddress.GetValue());
                }
            }
        }

        // Send event to trigger updated DB writing.
        static constexpr bool sIsDataImpure = true;
        RTCC::Event::SaveToRAM * const lSaveEvtPtr = Q_NEW(
            RTCC::Event::SaveToRAM,
            RTCC_SAVE_TO_NV_MEMORY_SIG,
            sIsDataImpure
        );
        sRTCC_AO->POST(lSaveEvtPtr, 0);
    }

    // Return where we're coming from.
    return "/network.shtml";
}


static char const *FindTagVal(
    char const * const aTagNameStr,
    int                aParamsQty,
    char const * const aParamsVec[],
    char const * const aValsVec[]
) {

    for (int lIx = 0; lIx < aParamsQty; lIx++) {
        if (0 == strcmp(aParamsVec[lIx], aTagNameStr)) {
            return aValsVec[lIx];
        }
    }

    return nullptr;
}
#endif // LWIP_HTTPD_CGI

} // namespace Net

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
