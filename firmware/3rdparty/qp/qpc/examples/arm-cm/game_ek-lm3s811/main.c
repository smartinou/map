/*****************************************************************************
* Product: "Fly 'n' Shoot" game example
* Last updated for version 5.4.0
* Last updated on  2015-03-25
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, www.state-machine.com.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Web:   www.state-machine.com
* Email: info@state-machine.com
*****************************************************************************/
#include "qpc.h"
#include "bsp.h"
#include "game.h"

/*..........................................................................*/
int main() {
    static QEvt const * missileQueueSto[2];
    static QEvt const * shipQueueSto[3];
    static QEvt const * tunnelQueueSto[GAME_MINES_MAX + 5];

    static QF_MPOOL_EL(QEvt)           smlPoolSto[10];
    static QF_MPOOL_EL(ObjectImageEvt) medPoolSto[2*GAME_MINES_MAX + 10];

    static QSubscrList    subscrSto[MAX_PUB_SIG];

    /* explicitly invoke the active objects' ctors... */
    Missile_ctor();
    Ship_ctor();
    Tunnel_ctor();

    QF_init();    /* initialize the framework and the underlying RT kernel */
    BSP_init();   /* initialize the Board Support Package */

    /* init publish-subscribe... */
    QF_psInit(subscrSto, Q_DIM(subscrSto));

    /* initialize the event pools... */
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QF_poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));

    /* send object dictionaries for event queues... */
    QS_OBJ_DICTIONARY(missileQueueSto);
    QS_OBJ_DICTIONARY(shipQueueSto);
    QS_OBJ_DICTIONARY(tunnelQueueSto);

    /* send object dictionaries for event pools... */
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(medPoolSto);

    /* send signal dictionaries for globally published events... */
    QS_SIG_DICTIONARY(TIME_TICK_SIG,      (void *)0);
    QS_SIG_DICTIONARY(PLAYER_TRIGGER_SIG, (void *)0);
    QS_SIG_DICTIONARY(PLAYER_QUIT_SIG,    (void *)0);
    QS_SIG_DICTIONARY(GAME_OVER_SIG,      (void *)0);

    /* start the active objects... */
    QACTIVE_START(AO_Missile,
                  1U,                /* QP priority */
                  missileQueueSto, Q_DIM(missileQueueSto), /* evt queue */
                  (void *)0, 0U,     /* no per-thread stack */
                  (QEvt *)0);        /* no initialization event */
    QACTIVE_START(AO_Ship,
                  2U,                /* QP priority */
                  shipQueueSto,    Q_DIM(shipQueueSto), /* evt queue */
                  (void *)0, 0U,     /* no per-thread stack */
                  (QEvt *)0);        /* no initialization event */
    QACTIVE_START(AO_Tunnel,
                  3U,                /* QP priority */
                  tunnelQueueSto,  Q_DIM(tunnelQueueSto), /* evt queue */
                  (void *)0, 0U,     /* no per-thread stack */
                  (QEvt *)0);        /* no initialization event */

    return QF_run(); /* run the QF application */
}

