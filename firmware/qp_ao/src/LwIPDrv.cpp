// *******************************************************************************
//
// Project: Larger project scope.
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

// Standard Libraries.
#include <cstddef>
#include <string.h>

// QP-port.
#include "qpcpp.h"

// LwIP stack.
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

#include "netif/etharp.h"

// Stellaris drivers.
#include "inc/hw_ethernet.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/ethernet.h"


// This file.
#include "LwIPDrv.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// ******************************************************************************
//                            FUNCTION PROTOTYPES
// ******************************************************************************

// ******************************************************************************
//                             GLOBAL VARIABLES
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

LwIPDrv::LwIPDrv()
  : mPBufQPtr(nullptr)
  , mNetIF{} {

  // Ctor body left intentionally empty.
}


struct netif *LwIPDrv::Init(u8_t          const aMACAddr[NETIF_MAX_HWADDR_LEN],
                            unsigned int  const aPBufQSize) {

  // Initialize LwIP.
  lwip_init();

  // Set MAC address in the network interface.
  mNetIF.hwaddr_len = NETIF_MAX_HWADDR_LEN;
  memcpy(&mNetIF.hwaddr[0], aMACAddr, NETIF_MAX_HWADDR_LEN);

  // Initialize interface hostname.
#if LWIP_NETIF_HOSTNAME
  mNetIF.hostname = "lwIP";
#endif // LWIP_NETIF_HOSTNAME
  mNetIF.name[0] = 'Q';
  mNetIF.name[1] = 'P';


  // Initialize the snmp variables and counters inside the struct netif.
  // The last argument should be replaced with your link speed,
  // in units of bits per second.
  NETIF_INIT_SNMP(&mNetIF, snmp_ifType_ethernet_csmacd, 1000000);

  // We directly use etharp_output() here to save a function call.
  // You can instead declare your own function an call etharp_output()
  // from it if you have to do some checks before sending (e.g. if link is available...).
  mNetIF.output     = &etharp_output;
  mNetIF.linkoutput = &EtherIFLinkOut;

  // Initialize the TX pbuf queue.
  mPBufQPtr = new LwIPDrv::PBufQ(aPBufQSize);
  if ((nullptr == mPBufQPtr)) {
    return nullptr;
  }

  ip_addr_t aIPAddr;
  ip_addr_t aNetMask;
  ip_addr_t aGateway;

#if (LWIP_DHCP == 0) && (LWIP_AUTOIP == 0)
  // No mechanism of obtaining IP address specified, use static IP:
  IP4_ADDR(&aIPAddr,
           STATIC_IPADDR0,
           STATIC_IPADDR1,
           STATIC_IPADDR2,
           STATIC_IPADDR3);
  IP4_ADDR(&aNetMask,
           STATIC_NET_MASK0,
           STATIC_NET_MASK1,
           STATIC_NET_MASK2,
           STATIC_NET_MASK3);
  IP4_ADDR(&aGateway,
           STATIC_GW_IPADDR0,
           STATIC_GW_IPADDR1,
           STATIC_GW_IPADDR2,
           STATIC_GW_IPADDR3);
#else
  // Either DHCP or AUTOIP are configured, start with zero IP addresses:
  IP4_ADDR(&aIPAddr,  0, 0, 0, 0);
  IP4_ADDR(&aNetMask, 0, 0, 0, 0);
  IP4_ADDR(&aGateway, 0, 0, 0, 0);
#endif

  // Add and configure the Ethernet interface with default settings.
  netif_add(&mNetIF,
            &aIPAddr,            // Configured IP addresses.
            &aNetMask,
            &aGateway,
            this,                // Use this LwIPDrv object as the state.
            &EtherIFInit,        // Ethernet interface initialization.
            &ip_input);          // Standard IP input processing.

  // Set as defaut network IF and bring the interface up.
  netif_set_default(&mNetIF);
  netif_set_up(&mNetIF);

  // start DHCP if configured in lwipopts.h.
#if (LWIP_DHCP != 0)
  dhcp_start(&mNetIF);
  // NOTE: If LWIP_AUTOIP is configured in lwipopts.h and
  // LWIP_DHCP_AUTOIP_COOP is set as well, the DHCP process will start
  // AutoIP after DHCP fails for 59 seconds.
#elif (LWIP_AUTOIP != 0)
  // start AutoIP if configured in lwipopts.h.
  autoip_start(&mNetIF);
#endif

  // Enable Ethernet TX and RX Packet Interrupts.
  HWREG(ETH_BASE + MAC_O_IM) |= (ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
  HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RXOF;
#endif

  return &mNetIF;
}


void LwIPDrv::Rd(void) {

  // New packet received into the pbuf?
  struct pbuf *lPBufPtr = LowLevelRx();
  if (lPBufPtr != nullptr) {
    // pbuf handled?
    if (ethernet_input(lPBufPtr, &mNetIF)) {
      // Free the pbuf.
      pbuf_free(lPBufPtr);
    }
    // Try to output a packet if TX fifo is empty and pbuf is available.
    Wr();
  }

  // Re-enable the RX interrupt.
  HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RX;
}


void LwIPDrv::Wr(void) {

  // TX fifo empty? Should be since we likely got here by TxEmpty int.
  if ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0) {
    struct pbuf *lPBufPtr = mPBufQPtr->Get();
    // pbuf found in the queue?
    if (lPBufPtr != nullptr) {
      // Send and free the pbuf: lwIP knows nothing of it.
      LowLevelTx(lPBufPtr);
      pbuf_free(lPBufPtr);
    }
  }
}


void LwIPDrv::ISR(QP::QActive &aAO,
                  QP::QEvent const &aRxIntEvt,
                  QP::QEvent const &aTxIntEvt) {

  unsigned long lIntStatus = HWREG(ETH_BASE + MAC_O_RIS);

  // Clear the interrupt sources.
  // Mask only the enabled sources.
  HWREG(ETH_BASE + MAC_O_IACK) = lIntStatus;
  lIntStatus &= HWREG(ETH_BASE + MAC_O_IM);

  if ((lIntStatus & ETH_INT_RX) != 0) {
    // Send to the AO.
    // Disable further RX.
    aAO.POST(&aRxIntEvt, 0);
    HWREG(ETH_BASE + MAC_O_IM) &= ~ETH_INT_RX;
  }

  if ((lIntStatus & ETH_INT_TX) != 0) {
    // Send to the AO.
    aAO.POST(&aTxIntEvt, 0);
  }

#if LINK_STATS
  if ((lIntStatus & ETH_INT_RXOF) != 0) {
    // Send to the AO.
  }
#endif
}

#if 0
extern "C" u32_t sys_now() {
  return 0;
}
#endif
// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

// Driver init function to set the MAC controller.
err_t LwIPDrv::EtherIFInit(struct netif *aNetIFPtr) {

  // Retrieve the LwIPDrv object.
  //LwIPDrv * const lDrvPtr = reinterpret_cast<LwIPDrv * const>(aNetIFPtr);

  SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
  SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

  // Set the MAC address.
  // Set the maximum transfer unit.
  // Set device capabilities.
  EthernetMACAddrSet(ETH_BASE, &aNetIFPtr->hwaddr[0]);
  aNetIFPtr->mtu = 1500;
  aNetIFPtr->flags = (NETIF_FLAG_BROADCAST
                      | NETIF_FLAG_ETHARP
                      | NETIF_FLAG_LINK_UP);

  // Disable all Ethernet interrupts.
  HWREG(ETH_BASE + MAC_O_IM) &= ~(ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER
                                  | ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER
                                  | ETH_INT_RX);

  // Acknolwedge all interrupts.
  HWREG(ETH_BASE + MAC_O_IACK) = HWREG(ETH_BASE + MAC_O_RIS);

  // Initialize the Ethernet Controller.
  EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

  // Configure the Ethernet Controller for normal operation.
  // - Enable TX Duplex Mode
  // - Enable TX Padding
  // - Enable TX CRC Generation
  // - Enable RX Multicast Reception
  EthernetConfigSet(ETH_BASE,
                    (ETH_CFG_TX_DPLXEN
                     | ETH_CFG_TX_CRCEN
                     | ETH_CFG_TX_PADEN
                     | ETH_CFG_RX_AMULEN));

  // Enable Ethernet transmitter and receiver.
  EthernetEnable(ETH_BASE);

  return ERR_OK;
}


err_t LwIPDrv::EtherIFLinkOut(struct netif *aNetIFPtr, struct pbuf *aPBufPtr) {

  // Retrieve the LwIPDrv object.
  LwIPDrv * const lDrvPtr = reinterpret_cast<LwIPDrv * const>(aNetIFPtr);

  // Check state of Tx queue and TX register.
  if (lDrvPtr->mPBufQPtr->IsEmpty() &&
      ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0)) {

    // Send the pbuf right away.
    // The pbuf will be freed by the lwIP code.
    lDrvPtr->LowLevelTx(aPBufPtr);
  } else {
    // Otherwise post the pbuf to the transmit queue.
    // Can the TX queue take the pbuf?
    if (lDrvPtr->mPBufQPtr->Put(aPBufPtr)) {
      // Reference the pbuf to spare it from freeing.
      pbuf_ref(aPBufPtr);
    } else {
      // No room in the queue.
      // The pbuf will be freed by the lwIP code.
      return ERR_MEM;
    }
  }
  return ERR_OK;
}


void LwIPDrv::LowLevelTx(struct pbuf * const aPBufPtr) {

  // Fill in the first two bytes of the payload data (configured as padding
  // with ETH_PAD_SIZE = 2) with the total length of the payload data
  // (minus the Ethernet MAC layer header).
  *(reinterpret_cast<unsigned short *>(aPBufPtr->payload)) = aPBufPtr->tot_len - 16;

  // Initialize the gather register.
  unsigned int   lByteGatherIx  = 0;
  unsigned long  lWordGather    = 0UL;
  unsigned char *lByteGatherPtr = (unsigned char *)&lWordGather;

  // Copy data from the pbuf(s) into the TX Fifo.
  for (struct pbuf *lQPBufPtr = aPBufPtr; lQPBufPtr != nullptr; lQPBufPtr = lQPBufPtr->next) {
    // Intialize a char pointer and index to the pbuf payload data.
    unsigned char *lByteBufPtr = reinterpret_cast<unsigned char *>(lQPBufPtr->payload);
    unsigned int   lByteBufIx  = 0;

    // If the gather buffer has leftover data from a previous pbuf
    // in the chain, fill it up and write it to the Tx FIFO.
    while ((lByteBufIx < lQPBufPtr->len) && (lByteGatherIx != 0)) {
      // Copy a byte from the pbuf into the gather buffer.
      lByteGatherPtr[lByteGatherIx] = lByteBufPtr[lByteBufIx++];

      // Increment the gather buffer index modulo 4.
      lByteGatherIx = ((lByteGatherIx + 1) % 4);
    }

    // If the gather index is 0 and the pbuf index is non-zero,
    // we have a gather buffer to write into the Tx FIFO.
    if ((lByteGatherIx == 0) && (lByteBufIx != 0)) {
      HWREG(ETH_BASE + MAC_O_DATA) = lWordGather;
      lWordGather = 0;
    }

    // Initialze a long pointer into the pbuf for 32-bit access.
    unsigned long *lWordBufPtr = reinterpret_cast<unsigned long *>(&lByteBufPtr[lByteBufIx]);

    // Copy words of pbuf data into the Tx FIFO, but don't go past the end of the pbuf.
    while ((lByteBufIx + 4) <= lQPBufPtr->len) {
      HWREG(ETH_BASE + MAC_O_DATA) = *lWordBufPtr++;
      lByteBufIx += 4;
    }

    // Check if leftover data in the pbuf and save it in the gather
    // buffer for the next time.
    while (lByteBufIx < lQPBufPtr->len) {
      // Copy a byte from the pbuf into the gather buffer.
      lByteGatherPtr[lByteGatherIx] = lByteBufPtr[lByteBufIx++];

      // Increment the gather buffer index modulo 4.
      lByteGatherIx = ((lByteGatherIx + 1) % 4);
    }
  }

  // Send any leftover data to the FIFO.
  // Wakeup the transmitter.
  HWREG(ETH_BASE + MAC_O_DATA) = lWordGather;
  HWREG(ETH_BASE + MAC_O_TR)   = MAC_TR_NEWTX;
  LINK_STATS_INC(link.xmit);
}



// This function will read a single packet from the Stellaris ethernet
// interface, if available, and return a pointer to a pbuf.
// The timestamp of the packet will be placed into the pbuf structure.
//
// @return pointer to pbuf packet if available, NULL otherswise.
struct pbuf *LwIPDrv::LowLevelRx(void) {

#if LWIP_PTPD
  u32_t time_s  = 0;
  u32_t time_ns = 0;

  // Get the current timestamp if PTPD is enabled.
  lwIPHostGetTime(&time_s, &time_ns);
#endif

  // Check if a packet is available, if not, return NULL packet.
  if ((HWREG(ETH_BASE + MAC_O_NP) & MAC_NP_NPR_M) == 0) {
    return nullptr;
  }


  // Obtain the size of the packet and put it into the "len" variable.
  // Note:  The length returned in the FIFO length position includes the
  // two bytes for the length + the 4 bytes for the FCS.
  u32_t lTmp = HWREG(ETH_BASE + MAC_O_DATA);
  u16_t lLen = lTmp & 0xFFFF;

  // We allocate a pbuf chain of pbufs from the pool.
  struct pbuf *lPBufPtr = pbuf_alloc(PBUF_RAW, lLen, PBUF_POOL);

  // If a pbuf was allocated, read the packet into the pbuf.
  if (lPBufPtr != nullptr) {
    // Place the first word into the first pbuf location.
    *(unsigned long *)lPBufPtr->payload = lTmp;
    lPBufPtr->payload = (char *)(lPBufPtr->payload) + 4;
    lPBufPtr->len -= 4;

    // Process all but the last buffer in the pbuf chain.
    struct pbuf *lQBufPtr = lPBufPtr;
    do {
      // Setup a byte pointer into the payload section of the pbuf.
      unsigned long *lPayloadPtr = reinterpret_cast<unsigned long *>(lQBufPtr->payload);

      // Read data from FIFO into the current pbuf (assume pbuf length is modulo 4).
      for (unsigned int lByteIx = 0; lByteIx < lQBufPtr->len; lByteIx += 4) {
        *lPayloadPtr++ = HWREG(ETH_BASE + MAC_O_DATA);
      }

      // Link in the next pbuf in the chain.
      lQBufPtr = lQBufPtr->next;
    } while (lQBufPtr != nullptr);

    // Restore the first pbuf parameters to their original values.
    lPBufPtr->payload = (char *)(lPBufPtr->payload) - 4;
    lPBufPtr->len += 4;

    // Adjust the link statistics.
    LINK_STATS_INC(link.recv);

#if LWIP_PTPD
    // Place the timestamp in the PBUF.
    lPBufPtr->time_s  = time_s;
    lPBufPtr->time_ns = time_ns;
#endif
  } else {
    // If no pbuf available, just drain the RX fifo.
    for (unsigned int lByteIx = 4; lByteIx < lLen; lByteIx += 4) {
      lTmp = HWREG(ETH_BASE + MAC_O_DATA);
    }

    // Adjust the link statistics.
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
  }

  return lPBufPtr;
}


LwIPDrv::PBufQ::PBufQ(unsigned int aQSize)
  : mRingPtr(nullptr )
    , mRingSize(aQSize)
    , mQWrIx(0)
    , mQRdIx(0)
    , mQOverflow(0) {

  // Ctor body.
  mRingPtr = new struct pbuf * [aQSize];
}


bool LwIPDrv::PBufQ::IsEmpty(void) {
  return (mQWrIx == mQRdIx);
}


bool LwIPDrv::PBufQ::Put(struct pbuf *aPBufPtr) {

  unsigned int lNextQWr = mQWrIx + 1;

  if (lNextQWr == Q_DIM(mRingPtr)) {
    lNextQWr = 0;
  }

  if (lNextQWr != mQRdIx) {
    // The queue isn't full so we add the new frame at the current
    // write position and move the write pointer.
    mRingPtr[mQWrIx] = aPBufPtr;
    if ((++mQWrIx) == Q_DIM(mRingPtr)) {
      mQWrIx = 0;
    }

    // Successfully posted the pbuf.
    return true;
  } else {
    // The stack is full so we are throwing away this value.
    // Keep track of the number of times this happens.
    mQOverflow++;
    // Could not post the pbuf.
    return false;
  }
}


struct pbuf *LwIPDrv::PBufQ::Get(void) {

  struct pbuf *lPBufPtr = nullptr;

  if (!IsEmpty()) {
    // The queue is not empty so return the next frame from it.
    // Adjust the read pointer accordingly.
    lPBufPtr = mRingPtr[mQRdIx];
    if ((++mQRdIx) == Q_DIM(mRingPtr)) {
      mQRdIx = 0;
    }
  }

  return lPBufPtr;
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
