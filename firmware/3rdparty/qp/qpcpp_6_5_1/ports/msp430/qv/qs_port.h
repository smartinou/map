/// @file
/// @brief QS/C++ port to MSP340 and MSP430X CPUs and IAR-430 compiler
/// @cond
///***************************************************************************
/// Last updated for version 6.3.8
/// Last updated on  2019-01-24
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps, All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qs_port_h
#define qs_port_h

// QS time-stamp size in bytes
#define QS_TIME_SIZE     4

// object pointer size in bytes
#define QS_OBJ_PTR_SIZE  2

// function pointer size in bytes
#define QS_FUN_PTR_SIZE  4

//****************************************************************************
// NOTE:
// QS might be used with or without other QP components, in which case
// the separate definitions of the macros QF_CRIT_STAT_TYPE, QF_CRIT_ENTRY,
// and QF_CRIT_EXIT are needed. In this port QS is configured to be used with
// the QF framework, by simply including "qf_port.h" *before* "qs.h".
//
#include "qf_port.h"  // use QS with QF
#include "qs.h"       // QS platform-independent public interface

#endif // qs_port_h
