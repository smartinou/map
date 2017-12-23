// *****************************************************************************
//
// Project: Active Object Library
//
// Module: LwIP manager QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group
//!
//! This is the main AO of the system.

// *****************************************************************************
//
//        Copyright (c) 2015-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#define LWIP_ALLOWED

// Standard Libraries.
#include <string.h>
#include <stdio.h>

// QP-port.
#include "qpcpp.h"

using namespace QP;

// lwIP stack.
#include "lwip/autoip.h"
#include "lwip/def.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/etharp.h"
#include "lwip/apps/httpd.h"
#include "lwip/priv/tcp_priv.h"

// lwIP application.
#ifdef __cplusplus
extern "C" {
} // extern "C"
#include "netif/eth_driver.h"
//#include "netif/EthDrv2.h"
#endif // __cplusplus


// TI Library.
#ifdef __cplusplus
extern "C" {
#include "hw_types.h"
#include "flash.h"
} // extern "C"
#endif // __cplusplus

// This project.
#include "BSP.h"
#include "LwIPMgr_AO.h"
#include "LwIPMgr_Evt.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

Q_DEFINE_THIS_FILE

// Has to be set to the fastest interval to be serviced in the stack.
#define LWIP_SLOW_TICK_MS AUTOIP_TMR_INTERVAL

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

#if LWIP_HTTPD_SSI
static uint16_t SSIHandler(int aIx, char *aInsertPtr, int aInsertLen);
#endif //LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
// Common Gateway Iinterface (CG) demo.
static char const *CGIDisplay(int   aIx,
                              int   aParamQty,
                              char *aParamPtr[],
                              char *aValPtr[]);
#endif //LWIP_HTTPD_CGI

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// Application signals cannot overlap the device-driver signals.
//Q_ASSERT_COMPILE(SIG_QTY < DEV_DRIVER_SIG);

#if LWIP_HTTPD_SSI
// Server-Side Include (SSI) demo.
static char const *sSSITags[] = {
  "s_xmit",
  "s_recv",
  "s_fw",
  "s_drop",
  "s_chkerr",
  "s_lenerr",
  "s_memerr",
  "s_rterr",
  "s_proerr",
  "s_opterr",
  "s_err",
};
#endif //LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
static tCGI const CGIHandlers[] = {
  { "/display.cgi", &CGIDisplay },
};
#endif //LWIP_HTTPD_CGI

// The single instance of LwIPMgr_AO.
LwIPMgr_AO *LwIPMgr_AO::mInstancePtr = static_cast<LwIPMgr_AO *>(0);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

LwIPMgr_AO::LwIPMgr_AO() :
  QActive(Q_STATE_CAST(&LwIPMgr_AO::Initial))
  , mSlowTickTimer(this, LWIP_SLOW_TICK_SIG, 0U)
  //, mEthDrvPtr(0)
  , mNetIFPtr(static_cast<struct netif *>(0))
  //  , mPCBPtr(static_cast<struct udp_pcb *>(0))
  , mIPAddr(IPADDR_ANY)
#if LWIP_TCP
  , mTCPTimer(0)
#endif
#if LWIP_ARP
  , mARPTimer(0)
#endif
#if LWIP_DHCP
  , mDHCPFineTimer(0)
  , mDHCPCoarseTimer(0)
#endif
#if LWIP_AUTOIP
  , mAutoIPTimer(0)
#endif
{

  // Singleton.
  mInstancePtr = this;
}


LwIPMgr_AO * const LwIPMgr_AO::GetInstancePtr(void) const {
  return mInstancePtr;
}


QP::QActive * const LwIPMgr_AO::GetOpaqueAOInstancePtr(void) const {
  return static_cast<QP::QActive * const>(mInstancePtr);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

QP::QState LwIPMgr_AO::Initial(LwIPMgr_AO     * const me,  //aMePtr,
                               QP::QEvt const * const e) { //aEvtPtr

  // Suppress the compiler warning about unused parameter.
  bool     lUseDHCP    = false;
  uint32_t lIPAddr     = 0x00000000;
  uint32_t lSubnetMask = 0x00000000;
  uint32_t lGWAddr     = 0x00000000;
  if (nullptr != e) {
    LwIPInitEvt const *lLwIPInitEvtPtr = static_cast<LwIPInitEvt const *>(e);
    lUseDHCP    = lLwIPInitEvtPtr->mUseDHCP;
    lIPAddr     = lLwIPInitEvtPtr->mIPAddr;
    lSubnetMask = lLwIPInitEvtPtr->mSubnetMask;
    lGWAddr     = lLwIPInitEvtPtr->mGWAddr;
  }
  // Configure the hardware MAC address for the Ethernet Controller
  //
  // For the Stellaris Eval Kits, the MAC address will be stored in the
  // non-volatile USER0 and USER1 registers.  These registers can be read
  // using the FlashUserGet function, as illustrated below.
  unsigned long lUser0 = 0;
  unsigned long lUser1 = 0;
  FlashUserGet(&lUser0, &lUser1);

  // The MAC address must have been programmed!
  Q_ASSERT((lUser0 != 0xFFFFFFFF) && (lUser1 != 0xFFFFFFFF));

#if 1
  // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
  // address needed to program the hardware registers, then program the MAC
  // address into the Ethernet Controller registers.
  // Initialize the Ethernet Driver.
  // Initialize IP address to impossible value.
  uint8_t lMACAddr[NETIF_MAX_HWADDR_LEN];
  lMACAddr[0] = (uint8_t)lUser0; lUser0 >>= 8;
  lMACAddr[1] = (uint8_t)lUser0; lUser0 >>= 8;
  lMACAddr[2] = (uint8_t)lUser0; lUser0 >>= 8;
  lMACAddr[3] = (uint8_t)lUser1; lUser1 >>= 8;
  lMACAddr[4] = (uint8_t)lUser1; lUser1 >>= 8;
  lMACAddr[5] = (uint8_t)lUser1;
#else
  uint8_t const lMACAddr[NETIF_MAX_HWADDR_LEN] = {0x00, 0x50, 0x1d, 0xc2, 0x70, 0xff};
#endif
  //me->mEthDrvPtr = EthDrv2::EthDrvInstance(8);
  //me->mEthDrvPtr = new EthDrv2(8);
  //me->mNetIFPtr = me->mEthDrvPtr->Init((QP::QActive *)me, &lMACAddr[0]);
  me->mNetIFPtr = eth_driver_init((QP::QActive *)me,
                                  lUseDHCP,
                                  lIPAddr,
                                  lSubnetMask,
                                  lGWAddr,
                                  &lMACAddr[0]);

  // Initialize the lwIP applications...
  // Initialize the simple HTTP-Deamon (web server).
  httpd_init();
#if LWIP_HTTPD_SSI
  http_set_ssi_handler(SSIHandler, sSSITags, Q_DIM(sSSITags));
#endif //LWIP_HTTPD_SSI
#if LWIP_HTTPD_CGI
  http_set_cgi_handlers(CGIHandlers, Q_DIM(CGIHandlers));
#endif //LWIP_HTTPD_CGI

#if 0
  QS_OBJ_DICTIONARY(&l_lwIPMgr);
  QS_OBJ_DICTIONARY(&l_lwIPMgr.mSlowTickTimer);
  QS_FUN_DICTIONARY(&QP::QHsm_top);
  QS_FUN_DICTIONARY(&LwIPMgr_AO::Initial);
  QS_FUN_DICTIONARY(&LwIPMgr_AO::Running);

  QS_SIG_DICTIONARY(SEND_UDP_SIG,        static_cast<QP::QActive *>(me));
  QS_SIG_DICTIONARY(LWIP_SLOW_TICK_SIG,  static_cast<QP::QActive *>(me));
  QS_SIG_DICTIONARY(LWIP_RX_READY_SIG,   static_cast<QP::QActive *>(me));
  QS_SIG_DICTIONARY(LWIP_TX_READY_SIG,   static_cast<QP::QActive *>(me));
  QS_SIG_DICTIONARY(LWIP_RX_OVERRUN_SIG, static_cast<QP::QActive *>(me));
#endif
  return Q_TRAN(&LwIPMgr_AO::Running);
}


QP::QState LwIPMgr_AO::Running(LwIPMgr_AO       * const me,  //aMePtr,
                               QP::QEvent const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG: {
    me->mSlowTickTimer.armX((LWIP_SLOW_TICK_MS * BSP_TICKS_PER_SEC) / 1000,
                            (LWIP_SLOW_TICK_MS * BSP_TICKS_PER_SEC) / 1000);
    return Q_HANDLED();
  }

  case Q_EXIT_SIG: {
    me->mSlowTickTimer.disarm();
    return Q_HANDLED();
  }

  case LWIP_RX_READY_SIG: {
    eth_driver_read();
    //me->mEthDrvPtr->Rd();
    return Q_HANDLED();
  }

  case LWIP_TX_READY_SIG: {
    eth_driver_write();
    //me->mEthDrvPtr->Wr();
    return Q_HANDLED();
  }

  case LWIP_SLOW_TICK_SIG: {
    // Has IP address changed?
    if (me->mIPAddr != me->mNetIFPtr->ip_addr.addr) {
      // IP address in the network byte order.
      // Save the IP addr.
      me->mIPAddr = me->mNetIFPtr->ip_addr.addr;
      uint32_t lIPAddrNet = ntohl(me->mIPAddr);
      (void)lIPAddrNet;
      // Publish the text event to display the new IP address.
#if 0
      TextEvt *lTextEvtPtr = Q_NEW(TextEvt, DISPLAY_IPADDR_SIG);
      snprintf(te->text,
               Q_DIM(lTextEvtPtr->text),
               "%d.%d.%d.%d",
               ((lIPAddrNet) >> 24) & 0xFF,
               ((lIPAddrNet) >> 16) & 0xFF,
               ((lIPAddrNet) >>  8) & 0xFF,
               ((lIPAddrNet) >>  0) & 0xFF);
      QP::QF::publish(static_cast<QP::QEvt *)>(TextEvtPtr));
#endif
    }

#if LWIP_TCP
    me->mTCPTimer += LWIP_SLOW_TICK_MS;
    if (me->mTCPTimer >= TCP_TMR_INTERVAL) {
      me->mTCPTimer = 0;
      tcp_tmr();
    }
#endif
#if LWIP_ARP
    me->mARPTimer += LWIP_SLOW_TICK_MS;
    if (me->mARPTimer >= ARP_TMR_INTERVAL) {
      me->mARPTimer = 0;
      etharp_tmr();
    }
#endif
#if LWIP_DHCP
    me->mDHCPFineTimer += LWIP_SLOW_TICK_MS;
    if (me->mDHCPFineTimer >= DHCP_FINE_TIMER_MSECS) {
      me->mDHCPFineTimer = 0;
      dhcp_fine_tmr();
    }
    me->mDHCPCoarseTimer += LWIP_SLOW_TICK_MS;
    if (me->mDHCPCoarseTimer >= DHCP_COARSE_TIMER_MSECS) {
      me->mDHCPCoarseTimer = 0;
      dhcp_coarse_tmr();
    }
#endif
#if LWIP_AUTOIP
    me->mAutoIPTimer += LWIP_SLOW_TICK_MS;
    if (me->mAutoIPTimer >= AUTOIP_TMR_INTERVAL) {
      me->mAutoIPTimer = 0;
      autoip_tmr();
    }
#endif
    return Q_HANDLED();
  }

  case LWIP_RX_OVERRUN_SIG: {
    LINK_STATS_INC(link.err);
    return Q_HANDLED();
  }

  }

  return Q_SUPER(&QP::QHsm::top);
}


#if LWIP_HTTPD_SSI
// HTTPD customizations.
// Server-Side Include (SSI) handler.
static uint16_t SSIHandler(int aIx, char *aInsertPtr, int aInsertLen) {

  struct stats_proto *lStatsPtr = &lwip_stats.link;
  STAT_COUNTER lVal = 0;

  switch (aIx) {
  case 0:
    // s_xmit
    lVal = lStatsPtr->xmit;
    break;

  case 1:
    // s_recv
    lVal = lStatsPtr->recv;
    break;

  case 2:
    // s_fw
    lVal = lStatsPtr->fw;
    break;

  case 3:
    // s_drop
    lVal = lStatsPtr->drop;
    break;

  case 4:
    // s_chkerr
    lVal = lStatsPtr->chkerr;
    break;

  case 5:
    // s_lenerr
    lVal = lStatsPtr->lenerr;
    break;

  case 6:
    // s_memerr
    lVal = lStatsPtr->memerr;
    break;

  case 7:
    // s_rterr
    lVal = lStatsPtr->rterr;
    break;

  case 8:
    // s_proerr
    lVal = lStatsPtr->proterr;
    break;

  case 9:
    // s_opterr
    lVal = lStatsPtr->opterr;
    break;

  case 10:
    // s_err
    lVal = lStatsPtr->err;
    break;
  }

  return snprintf(aInsertPtr, LWIP_HTTPD_MAX_TAG_NAME_LEN, "%d", lVal);
}
#endif //LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
// Common Gateway Iinterface (CG) handler.
static char const *CGIDisplay(int   aIx,
                              int   aParamQty,
                              char *aParamPtr[],
                              char *aValPtr[]) {

  for (int lIx = 0; lIx < aParamQty; ++lIx) {
    if (strstr(aParamPtr[lIx], "text") != static_cast<char *>(0)) {
      // Param text found?
#if 0
      TextEvt *lTextEvtPtr = Q_NEW(TextEvt, DISPLAY_CGI_SIG);
      strncpy(lTextEvtPtr->text,
              aValptr[lIx],
              Q_DIM(lTextEvtPtr->text));
      QF_publish(static_cast<QP::QEvt *>(lTextEvtPtr));
#endif
      return "/thank_you.htm";
    }
  }

  // No URI, HTTPD will send 404 error page to the browser.
  return static_cast<char const *>(0);
}
#endif //LWIP_HTTPD_CGI

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
