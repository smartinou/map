/*****************************************************************************
* Product: Ethernet driver for lwIP with QP
* Last Updated for Version: 4.0.03
* Date of the Last Update:  Mar 23, 2009
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web site:  http://www.quantum-leaps.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef eth_driver_h
#define eth_driver_h

using namespace QP;

struct netif *eth_driver_init(QActive *active,
			      bool     aUseDHCP,
			      uint32_t aIPAddr,
			      uint32_t aSubnetMask,
			      uint32_t aGWAddr,
                              u8_t     macaddr[NETIF_MAX_HWADDR_LEN]);
void eth_driver_read(void);
void eth_driver_write(void);

#if 0
enum EthDriverSignals {
    LWIP_SLOW_TICK_SIG = DEV_DRIVER_SIG,
    LWIP_RX_READY_SIG,
    LWIP_TX_READY_SIG,
    LWIP_RX_OVERRUN_SIG
};
#endif
#endif                                                      /* eth_driver_h */
