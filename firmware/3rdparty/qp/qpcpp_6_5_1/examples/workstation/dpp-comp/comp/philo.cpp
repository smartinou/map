//$file${Comp::.::philo.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp.qm
// File:  ${Comp::.::philo.cpp}
//
// This code has been generated by QM 4.5.0 (https://www.state-machine.com/qm).
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//$endhead${Comp::.::philo.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

namespace DPP {

// helper function to provide a randomized think time for Philos
inline QP::QTimeEvtCtr think_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + (BSP::TICKS_PER_SEC/2U));
}

// helper function to provide a randomized eat time for Philos
inline QP::QTimeEvtCtr eat_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + BSP::TICKS_PER_SEC);
}

} // namespace DPP

// Philo definition ----------------------------------------------------------
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 650U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.5.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${Comp::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace DPP {

//${Comp::Philo} .............................................................
//${Comp::Philo::Philo} ......................................................
Philo::Philo()
  : QHsm(&initial),
    m_timeEvt(AO_Table, this, TIMEOUT_SIG, 0U)
{}

//${Comp::Philo::SM} .........................................................
Q_STATE_DEF(Philo, initial) {
    //${Comp::Philo::SM::initial}
    static bool registered = false; // starts off with 0, per C-standard
    (void)e; // suppress the compiler warning about unused parameter
    if (!registered) {
        registered = true;
        QS_FUN_DICTIONARY(&initial);
        QS_FUN_DICTIONARY(&thinking);
        QS_FUN_DICTIONARY(&hungry);
        QS_FUN_DICTIONARY(&eating);
    }
    QS_SIG_DICTIONARY(HUNGRY_SIG, this);  // signal for each Philo
    return tran(&thinking);
}
//${Comp::Philo::SM::thinking} ...............................................
Q_STATE_DEF(Philo, thinking) {
    QP::QState status_;
    switch (e->sig) {
        //${Comp::Philo::SM::thinking}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(think_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Comp::Philo::SM::thinking}
        case Q_EXIT_SIG: {
            (void)m_timeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Comp::Philo::SM::thinking::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&hungry);
            break;
        }
        //${Comp::Philo::SM::thinking::TEST}
        case TEST_SIG: {
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//${Comp::Philo::SM::hungry} .................................................
Q_STATE_DEF(Philo, hungry) {
    QP::QState status_;
    switch (e->sig) {
        //${Comp::Philo::SM::hungry}
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philo = this;
            AO_Table->postLIFO(pe);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Comp::Philo::SM::hungry::EAT}
        case EAT_SIG: {
            status_ = tran(&eating);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//${Comp::Philo::SM::eating} .................................................
Q_STATE_DEF(Philo, eating) {
    QP::QState status_;
    switch (e->sig) {
        //${Comp::Philo::SM::eating}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(eat_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Comp::Philo::SM::eating}
        case Q_EXIT_SIG: {
            (void)m_timeEvt.disarm();

            // asynchronously post event to the Container
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philo = this;
            AO_Table->postLIFO(pe);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Comp::Philo::SM::eating::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&thinking);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

} // namespace DPP
//$enddef${Comp::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
