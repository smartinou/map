/// @file
/// @brief QK/C++ port to AVRmega, GNU-AVR toolset
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-03-14
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qk_port_h
#define qk_port_h

// QK interrupt entry and exit
#define QK_ISR_ENTRY()    (++QK_intNest_)

#define QK_ISR_EXIT()     do { \
    --QK_intNest_; \
    if (QK_intNest_ == static_cast<uint_fast8_t>(0)) { \
        uint_fast8_t p = QK_schedPrio_(); \
        if (p != static_cast<uint_fast8_t>(0)) { \
            QK_sched_(p); \
        } \
    } \
} while (false)

// QK sleep mode...
#define QK_CPU_SLEEP()    do { \
    __asm__ __volatile__ ("sleep" ::); \
    SMCR = 0U; \
} while (false)

#include "qk.h" // QK platform-independent public interface

#endif // qk_port_h
