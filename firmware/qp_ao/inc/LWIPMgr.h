#ifndef LWIP_MGR_H_
#define LWIP_MGR_H_
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: LWIP manager QP Active Object.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class EthDrv2;


//! \brief Brief description.
//! Details follow...
//! ...here.
class LWIPMgr : public QP::QActive {
 public:
  LWIPMgr();
  LWIPMgr * const GetInstancePtr(void) const;
  QP::QActive * const GetOpaqueAOInstancePtr(void) const;

 protected:
  static QP::QState Initial(LWIPMgr         * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
  static QP::QState Running(LWIPMgr         * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
private:
  QP::QTimeEvt    mSlowTickTimer;

  EthDrv2         *mEthDrvPtr;
  struct netif   *mNetIFPtr;
  //struct udp_pcb *mPCBPtr;
  // IP address in the native host byte order.
  uint32_t        mIPAddr;

#if LWIP_TCP
  uint32_t mTCPTimer;
#endif
#if LWIP_ARP
  uint32_t mARPTimer;
#endif
#if LWIP_DHCP
  uint32_t mDHCPFineTimer;
  uint32_t mDHCPCoarseTimer;
#endif
#if LWIP_AUTOIP
  uint32_t mAutoIPTimer;
#endif

  // The single instance of LWIPMgr Active Object.
  static LWIPMgr *mInstancePtr;
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
#endif // LWIP_MGR_H_
