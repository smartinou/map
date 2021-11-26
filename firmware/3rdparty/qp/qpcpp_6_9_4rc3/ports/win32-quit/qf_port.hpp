/// @file
/// @brief QF/C++ port for QUIT unit internal test, Win32 with GNU/VisualC++
/// @ingroup ports
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2
/// Last updated on  2021-01-12
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond
///
#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// Win32 event queue and thread types
#define QF_EQUEUE_TYPE       QEQueue
// QF_OS_OBJECT_TYPE  not used
// QF_THREAD_TYPE     not used

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE        64U

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2U

// Activate the QF QActive::stop() API
#define QF_ACTIVE_STOP       1

// QF interrupt disable/enable
#define QF_INT_DISABLE()     (++QF_intNest)
#define QF_INT_ENABLE()      (--QF_intNest)

// QUIT critical section
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy) QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)  QF_INT_ENABLE()

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // Win32-QV needs event-queue
#include "qmpool.hpp"    // Win32-QV needs memory-pool
#include "qpset.hpp"     // Win32-QV needs priority-set
#include "qf.hpp"        // QF platform-independent public interface

namespace QP {

// interrupt nesting up-down counter
extern std::uint8_t volatile QF_intNest;

} // namespace QP

// special adaptations for QWIN GUI applications
#ifdef QWIN_GUI
    // replace main() with main_gui() as the entry point to a GUI app.
    #define main() main_gui()
    int_t main_gui(); // prototype of the GUI application entry point
#endif

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // Win32-QV specific scheduler locking, see NOTE2
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT((me_)->m_eQueue.m_frontEvt != nullptr)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QV_readySet_.insert((me_)->m_prio)); \
        (void)SetEvent(QV_win32Event_)

    // Win32-QV specific event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_)  ((p_).put((e_), (qs_id_)))

    #include "qf_pkg.hpp" // internal QF interface

#endif  // QP_IMPL

#endif // QF_PORT_HPP
