//****************************************************************************
// Model: dpp.qm
// File:  ./table.cpp
//
// This code has been generated by QM tool (see state-machine.com/qm).
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
//****************************************************************************
//${.::table.cpp} ............................................................
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
namespace DPP {

//${AOs::Table} ..............................................................
class Table : public QP::QMActive {
private:
    uint8_t m_fork[N_PHILO];
    bool m_isHungry[N_PHILO];

public:
    Table();

protected:
    static QP::QState initial(Table * const me, QP::QEvt const * const e);
    static QP::QState active  (Table * const me, QP::QEvt const * const e);
    static QP::QMState const active_s;
    static QP::QState serving  (Table * const me, QP::QEvt const * const e);
    static QP::QState serving_e(Table * const me);
    static QP::QMState const serving_s;
    static QP::QState paused  (Table * const me, QP::QEvt const * const e);
    static QP::QState paused_e(Table * const me);
    static QP::QState paused_x(Table * const me);
    static QP::QMState const paused_s;
};

} // namespace DPP

namespace DPP {

// helper function to provide the RIGHT neighbour of a Philo[n]
inline uint8_t RIGHT(uint8_t const n) {
    return static_cast<uint8_t>((n + (N_PHILO - 1U)) % N_PHILO);
}

// helper function to provide the LEFT neighbour of a Philo[n]
inline uint8_t LEFT(uint8_t const n) {
    return static_cast<uint8_t>((n + 1U) % N_PHILO);
}

static uint8_t const FREE = static_cast<uint8_t>(0);
static uint8_t const USED = static_cast<uint8_t>(1);

static char_t const * const THINKING = &"thinking"[0];
static char_t const * const HUNGRY   = &"hungry  "[0];
static char_t const * const EATING   = &"eating  "[0];

// Local objects -------------------------------------------------------------
static Table l_table; // the single instance of the Table active object

// Global-scope objects ------------------------------------------------------
QP::QMActive * const AO_Table = &l_table; // "opaque" AO pointer

} // namespace DPP

//............................................................................
namespace DPP {

//${AOs::Table} ..............................................................
//${AOs::Table::Table} .......................................................
Table::Table()
  : QMActive(Q_STATE_CAST(&Table::initial))
{
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = false;
    }
}

//${AOs::Table::SM} ..........................................................
QP::QState Table::initial(Table * const me, QP::QEvt const * const e) {
    static struct {
        QP::QMState const *target;
        QP::QActionHandler act[2];
    } const tatbl_ = { // transition-action table
        &serving_s,
        {
            Q_ACTION_CAST(&serving_e), // entry
            Q_ACTION_CAST(0)  // zero terminator
        }
    };
    // ${AOs::Table::SM::initial}
    (void)e; // suppress the compiler warning about unused parameter

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QP::QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    QS_SIG_DICTIONARY(DONE_SIG,      (void *)0); // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       (void *)0);
    QS_SIG_DICTIONARY(PAUSE_SIG,     (void *)0);
    QS_SIG_DICTIONARY(SERVE_SIG,     (void *)0);
    QS_SIG_DICTIONARY(TERMINATE_SIG, (void *)0);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    me); // signal just for Table

    me->subscribe(DONE_SIG);
    me->subscribe(PAUSE_SIG);
    me->subscribe(SERVE_SIG);
    me->subscribe(TERMINATE_SIG);

    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        me->m_fork[n] = FREE;
        me->m_isHungry[n] = false;
        BSP_displayPhilStat(n, THINKING);
    }
    return QM_TRAN_INIT(&tatbl_);
}
//${AOs::Table::SM::active} ..................................................
QP::QMState const Table::active_s = {
    static_cast<QP::QMState const *>(0), // superstate (top)
    Q_STATE_CAST(&active),
    Q_ACTION_CAST(0), // no entry action
    Q_ACTION_CAST(0), // no exit action
    Q_ACTION_CAST(0)  // no intitial tran.
};
// ${AOs::Table::SM::active}
QP::QState Table::active(Table * const me, QP::QEvt const * const e) {
    QP::QState status_;
    switch (e->sig) {
        // ${AOs::Table::SM::active::TERMINATE}
        case TERMINATE_SIG: {
            BSP_terminate(0);
            status_ = QM_HANDLED();
            break;
        }
        // ${AOs::Table::SM::active::EAT}
        case EAT_SIG: {
            Q_ERROR();
            status_ = QM_HANDLED();
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    (void)me; // avoid compiler warning in case 'me' is not used
    return status_;
}
//${AOs::Table::SM::active::serving} .........................................
QP::QMState const Table::serving_s = {
    &Table::active_s, // superstate
    Q_STATE_CAST(&serving),
    Q_ACTION_CAST(&serving_e),
    Q_ACTION_CAST(0), // no exit action
    Q_ACTION_CAST(0)  // no intitial tran.
};
// ${AOs::Table::SM::active::serving}
QP::QState Table::serving_e(Table * const me) {
    for (uint8_t n = 0U; n < N_PHILO; ++n) { // give permissions to eat...
        if (me->m_isHungry[n]
            && (me->m_fork[LEFT(n)] == FREE)
            && (me->m_fork[n] == FREE))
        {
            me->m_fork[LEFT(n)] = USED;
            me->m_fork[n] = USED;
            TableEvt *te = Q_NEW(TableEvt, EAT_SIG);
            te->philoNum = n;
            QP::QF::PUBLISH(te, me);
            me->m_isHungry[n] = false;
            BSP_displayPhilStat(n, EATING);
        }
    }
    return QM_ENTRY(&serving_s);
}
// ${AOs::Table::SM::active::serving}
QP::QState Table::serving(Table * const me, QP::QEvt const * const e) {
    QP::QState status_;
    switch (e->sig) {
        // ${AOs::Table::SM::active::serving::HUNGRY}
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, HUNGRY);
            uint8_t m = LEFT(n);
            // ${AOs::Table::SM::active::serving::HUNGRY::[bothfree]}
            if ((me->m_fork[m] == FREE) && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = USED;
                me->m_fork[n] = USED;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(n, EATING);
                status_ = QM_HANDLED();
            }
            // ${AOs::Table::SM::active::serving::HUNGRY::[else]}
            else {
                me->m_isHungry[n] = true;
                status_ = QM_HANDLED();
            }
            break;
        }
        // ${AOs::Table::SM::active::serving::DONE}
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            // both forks of Phil[n] must be used
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = FREE;
            me->m_fork[n] = FREE;
            m = RIGHT(n); // check the right neighbor

            if (me->m_isHungry[m] && (me->m_fork[m] == FREE)) {
                me->m_fork[n] = USED;
                me->m_fork[m] = USED;
                me->m_isHungry[m] = false;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(m, EATING);
            }
            m = LEFT(n); // check the left neighbor
            n = LEFT(m); // left fork of the left neighbor
            if (me->m_isHungry[m] && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = USED;
                me->m_fork[n] = USED;
                me->m_isHungry[m] = false;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(m, EATING);
            }
            status_ = QM_HANDLED();
            break;
        }
        // ${AOs::Table::SM::active::serving::EAT}
        case EAT_SIG: {
            Q_ERROR();
            status_ = QM_HANDLED();
            break;
        }
        // ${AOs::Table::SM::active::serving::PAUSE}
        case PAUSE_SIG: {
            static struct {
                QP::QMState const *target;
                QP::QActionHandler act[2];
            } const tatbl_ = { // transition-action table
                &paused_s,
                {
                    Q_ACTION_CAST(&paused_e), // entry
                    Q_ACTION_CAST(0)  // zero terminator
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}
//${AOs::Table::SM::active::paused} ..........................................
QP::QMState const Table::paused_s = {
    &Table::active_s, // superstate
    Q_STATE_CAST(&paused),
    Q_ACTION_CAST(&paused_e),
    Q_ACTION_CAST(&paused_x),
    Q_ACTION_CAST(0)  // no intitial tran.
};
// ${AOs::Table::SM::active::paused}
QP::QState Table::paused_e(Table * const me) {
    BSP_displayPaused(1U);
    (void)me; // avoid compiler warning in case 'me' is not used
    return QM_ENTRY(&paused_s);
}
// ${AOs::Table::SM::active::paused}
QP::QState Table::paused_x(Table * const me) {
    BSP_displayPaused(0U);
    (void)me; // avoid compiler warning in case 'me' is not used
    return QM_EXIT(&paused_s);
}
// ${AOs::Table::SM::active::paused}
QP::QState Table::paused(Table * const me, QP::QEvt const * const e) {
    QP::QState status_;
    switch (e->sig) {
        // ${AOs::Table::SM::active::paused::SERVE}
        case SERVE_SIG: {
            static struct {
                QP::QMState const *target;
                QP::QActionHandler act[3];
            } const tatbl_ = { // transition-action table
                &serving_s,
                {
                    Q_ACTION_CAST(&paused_x), // exit
                    Q_ACTION_CAST(&serving_e), // entry
                    Q_ACTION_CAST(0)  // zero terminator
                }
            };
            status_ = QM_TRAN(&tatbl_);
            break;
        }
        // ${AOs::Table::SM::active::paused::HUNGRY}
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // philo ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));
            me->m_isHungry[n] = true;
            BSP_displayPhilStat(n, HUNGRY);
            status_ = QM_HANDLED();
            break;
        }
        // ${AOs::Table::SM::active::paused::DONE}
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            /* both forks of Phil[n] must be used */
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = FREE;
            me->m_fork[n] = FREE;
            status_ = QM_HANDLED();
            break;
        }
        default: {
            status_ = QM_SUPER();
            break;
        }
    }
    return status_;
}

} // namespace DPP
