#ifndef LWIP_MGR_AO_H_
#define LWIP_MGR_AO_H_
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: LWIP manager QP Active Object.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2017, Martin Garon, All rights reserved.
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

class LwIPDrv;


//! \brief Brief description.
//! Details follow...
//! ...here.
class LwIPMgr_AO : public QP::QActive {
 public:
  //LwIPMgr_AO();
  LwIPMgr_AO(LwIPDrv &aLwIPDrv);
  LwIPMgr_AO  * const GetInstancePtr(void) const;
  QP::QActive * const GetOpaqueAOInstancePtr(void) const;

 protected:
  static QP::QState Initial(LwIPMgr_AO         * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
  static QP::QState Running(LwIPMgr_AO         * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
private:
  QP::QTimeEvt    mSlowTickTimer;

  // TODO: TRY TO USE NEW DRIVER WITH PRIMARY INTERFACE.
  // THEN MAKE IT ARRAY OF INTERFACES.
  LwIPDrv        &mEthDrvPtr;
  //struct netif   *mNetIFPtr;
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

  // The single instance of LwIPMgr_AO Active Object.
  static LwIPMgr_AO *mInstancePtr;
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
#endif // LWIP_MGR_AO_H_
