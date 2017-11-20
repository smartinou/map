#ifndef LWIP_DRV_H_
#define LWIP_DRV_H_
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2017, Pleora Technologies, All rights reserved.
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

// Forward declarations.


//! \brief Brief description.
//! Details follow...
//! ...here.
class LwIPDrv {
 public:
  LwIPDrv();

  struct netif *Init(u8_t          const aMACAddr[NETIF_MAX_HWADDR_LEN],
		     unsigned int  const aQSize);
  void Rd(void);
  void Wr(void);

  void ISR(QP::QActive &aAO,
	   QP::QEvent const &aRxIntEvt,
	   QP::QEvent const &aTxIntEvt);

 private:
  // Static functions for assigning to netif.
  static err_t EtherIFInit(struct netif *aNetIFPtr);
  static err_t EtherIFLinkOut(struct netif *aNetIFPtr,
			      struct pbuf  *aPBufPtr);

  // Low-level driver methods.
  void         LowLevelTx(struct pbuf * const aPBufPtr);
  struct pbuf *LowLevelRx(void);

  // Internal PBuf Q-ring class.
  class PBufQ {
  public:
    PBufQ(unsigned int aQSize);
    bool         IsEmpty(void);
    bool         Put(struct pbuf *aPBufPtr);
    struct pbuf *Get(void);
  private:
    struct pbuf **mRingPtr;
    unsigned int mRingSize;
    unsigned int mQWrIx;
    unsigned int mQRdIx;
    unsigned int mQOverflow;
  };

 private:
  // Queue of pbufs for transmission.
  PBufQ       *mPBufQPtr;
  struct netif mNetIF;
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
#endif // LWIP_DRV_H_
