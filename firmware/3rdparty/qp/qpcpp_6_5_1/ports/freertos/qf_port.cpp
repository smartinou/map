/// @file
/// @brief QF/C++ port to FreeRTOS (v10.x) kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.5.0
/// Last updated on  2019-03-22
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#if ( configSUPPORT_STATIC_ALLOCATION == 0 )
    #error "This QP/C++ port to FreeRTOS requires configSUPPORT_STATIC_ALLOCATION"
#endif

#if ( configMAX_PRIORITIES < QF_MAX_ACTIVE )
    #error "FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE"
#endif

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void task_function(void *pvParameters); // FreeRTOS task signature

//............................................................................
void QF::init(void) {
    // empty for FreeRTOS
}
//............................................................................
int_t QF::run(void) {
    onStartup();  // the startup callback (configure/enable interrupts)
    vTaskStartScheduler(); // start the FreeRTOS scheduler
    Q_ERROR_ID(110);       // the FreeRTOS scheduler should never return
    return static_cast<int_t>(0); // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

//............................................................................
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t stkSize,
                    QEvt const *ie)
{
    // task name provided by the user in QF_setTaskName() or default name
    char_t const *taskName = (m_thread.pxDummy1 != static_cast<void *>(0))
                             ? static_cast<char_t const *>(m_thread.pxDummy1)
                             : static_cast<char_t const *>("AO");

    Q_REQUIRE_ID(200, (static_cast<uint_fast8_t>(0) < prio)
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) /* in range */
        && (qSto != static_cast<QEvt const **>(0)) /* queue storage */
        && (qLen > static_cast<uint_fast16_t>(0))  /* queue size */
        && (stkSto != static_cast<void *>(0))      /* stack storage */
        && (stkSize > static_cast<uint_fast16_t>(0))); // stack size

    // create the event queue for the AO
    m_eQueue.init(qSto, qLen);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // statically create the FreeRTOS task for the AO
    TaskHandle_t thr = xTaskCreateStatic(
              &task_function, // the task function
              taskName ,      // the name of the task
              stkSize/sizeof(portSTACK_TYPE), // stack length
              this,           // the 'pvParameters' parameter
              prio + tskIDLE_PRIORITY,  // FreeRTOS priority
              static_cast<StackType_t *>(stkSto), // stack storage
              &m_thread);     // task buffer
    Q_ENSURE_ID(210, thr != static_cast<TaskHandle_t>(0)); // must be created
}
//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe from all events
    m_osObject = false; // stop the thread loop (see QF::thread_)
}
#endif
//............................................................................
void QActive::setAttr(uint32_t attr1, void const *attr2) {
    // this function must be called before QACTIVE_START(),
    // which implies that me->thread.pxDummy1 must not be used yet;
    Q_REQUIRE_ID(300, m_thread.pxDummy1 == static_cast<void *>(0));
    switch (attr1) {
        case TASK_NAME_ATTR:
            // temporarily store the name
            m_thread.pxDummy1 = const_cast<void *>(attr2);
            break;
        //...
    }
}
//............................................................................
static void task_function(void *pvParameters) { // FreeRTOS task signature
    QF::thread_(reinterpret_cast<QActive *>(pvParameters));
}
// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
#ifdef QF_ACTIVE_STOP
    act->m_osObject = true;
    while (act->m_osObject)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QF_ACTIVE_STOP
    remove_(act); // remove this object from QF
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
#endif
}

/*==========================================================================*/
// The "FromISR" QP APIs for the FreeRTOS port...
#ifdef Q_SPY
bool QActive::postFromISR_(QEvt const * const e,
                           uint_fast16_t const margin, void *par,
                           void const * const sender)
#else
bool QActive::postFromISR_(QEvt const * const e,
                       uint_fast16_t const margin, void *par)
#endif
{
    /// @pre event pointer must be valid
    Q_REQUIRE_ID(400, e != static_cast<QEvt const *>(0));

    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();
    QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into the temporary

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > static_cast<QEQueueCtr>(0)) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(410); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();       // timestamp
            QS_OBJ_(sender);  // the sender object
            QS_SIG_(e->sig);  // the signal of the event
            QS_OBJ_(this);    // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);   // number of free entries
            QS_EQC_(m_eQueue.m_nMin); // min number of free entries
        QS_END_NOCRIT_()

        // is it a dynamic event?
        if (e->poolId_ != static_cast<uint8_t>(0)) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        --nFree;  // one free entry just used up
        m_eQueue.m_nFree = nFree; // update the volatile
        if (m_eQueue.m_nMin > nFree) {
            m_eQueue.m_nMin = nFree; // update minimum so far
        }

        // is the queue empty?
        if (m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) {
            m_eQueue.m_frontEvt = e; // deliver event directly
            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

            // signal the event queue
            vTaskNotifyGiveFromISR(reinterpret_cast<TaskHandle_t>(&m_thread),
                                   static_cast<BaseType_t *>(par));
        }
        // queue is not empty, insert event into the ring-buffer
        else {
            // insert event into the ring buffer (FIFO)
            QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;
            if (m_eQueue.m_head == static_cast<QEQueueCtr>(0)) {
                m_eQueue.m_head = m_eQueue.m_end; // wrap around
            }
            --m_eQueue.m_head; // advance the head (counter clockwise)
            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
        }
    }
    else {

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();       // timestamp
            QS_OBJ_(sender);  // the sender object
            QS_SIG_(e->sig);  // the signal of the event
            QS_OBJ_(this);    // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);   // number of free entries
            QS_EQC_(static_cast<QEQueueCtr>(margin)); // margin requested
        QS_END_NOCRIT_()

        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

        QF::gcFromISR(e); // recycle the evnet to avoid a leak
    }

    return status;
}
/*..........................................................................*/
#ifdef Q_SPY
void QF::publishFromISR_(QEvt const *e, void *par,
                         void const * const sender)
#else
void QF::publishFromISR_(QEvt const *e, void *par)
#endif
{
    /// @pre the published signal must be within the configured range
    Q_REQUIRE_ID(500, static_cast<enum_t>(e->sig) < QF_maxPubSignal_);

    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();

    QS_BEGIN_NOCRIT_(QS_QF_PUBLISH,
        static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();           // the timestamp
        QS_OBJ_(sender);      // the sender object
        QS_SIG_(e->sig);      // the signal of the event
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        // NOTE: The reference counter of a dynamic event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gcFromISR()) decrements the reference counter
        // and recycles the event if the counter drops to zero. This covers
        // the case when the event was published without any subscribers.
        //
        QF_EVT_REF_CTR_INC_(e);
    }

    // make a local, modifiable copy of the subscriber list
    QPSet subscrList = QF_PTR_AT_(QF_subscrList_, e->sig);
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

    if (subscrList.notEmpty()) {
        uint_fast8_t p = subscrList.findMax(); // the highest-prio subscriber

        // no need to lock the scheduler in the ISR context
        do { // loop over all subscribers */
            // the prio of the AO must be registered with the framework
            Q_ASSERT_ID(510, active_[p] != static_cast<QActive *>(0));

            // POST_FROM_ISR() asserts internally if the queue overflows
            (void)active_[p]->POST_FROM_ISR(e, par, sender);

            subscrList.remove(p); // remove the handled subscriber
            if (subscrList.notEmpty()) {  // still more subscribers?
                p = subscrList.findMax(); // the highest-prio subscriber
            }
            else {
                p = static_cast<uint_fast8_t>(0); // no more subscribers
            }
        } while (p != static_cast<uint_fast8_t>(0));
        // no need to unlock the scheduler in the IRS context
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    gcFromISR(e);
}
//............................................................................
#ifdef Q_SPY
void QF::tickXfromISR_(uint_fast8_t const tickRate, void *par,
                       void const * const sender)
#else
void QF::tickXfromISR_(uint_fast8_t const tickRate, void *par)
#endif
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];
    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();

    QS_BEGIN_NOCRIT_(QS_QF_TICK, static_cast<void*>(0), static_cast<void*>(0))
        QS_TEC_(static_cast<QTimeEvtCtr>(++prev->m_ctr)); // tick ctr
        QS_U8_(static_cast<uint8_t>(tickRate));           // tick rate
    QS_END_NOCRIT_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == static_cast<QTimeEvt *>(0)) {

            // any new time events armed since the last run of QF::tickX_()?
            if (timeEvtHead_[tickRate].m_act != static_cast<void *>(0)) {

                // sanity check
                Q_ASSERT_ID(610, prev != static_cast<QTimeEvt *>(0));
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = static_cast<void *>(0);
                t = prev->m_next; // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time event scheduled for removal?
        if (t->m_ctr == static_cast<QTimeEvtCtr>(0)) {
            prev->m_next = t->m_next;
            t->refCtr_ &= static_cast<uint8_t>(0x7F); // mark as unlinked
            // do NOT advance the prev pointer
            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
        }
        else {
            --t->m_ctr;

            // is time evt about to expire?
            if (t->m_ctr == static_cast<QTimeEvtCtr>(0)) {
                QActive *act = t->toActive(); // temporary for volatile

                // periodic time evt?
                if (t->m_interval != static_cast<QTimeEvtCtr>(0)) {
                    t->m_ctr = t->m_interval; // rearm the time event
                    prev = t; // advance to this time event
                }
                // one-shot time event: automatically disarm
                else {
                    prev->m_next = t->m_next;

                    // mark as unlinked
                    t->refCtr_ &= static_cast<uint8_t>(0x7F);
                    // do NOT advance the prev pointer

                    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_AUTO_DISARM,
                                     QS::priv_.locFilter[QS::TE_OBJ], t)
                        QS_OBJ_(t);   // this time event object
                        QS_OBJ_(act); // the target AO
                        QS_U8_(static_cast<uint8_t>(tickRate)); // tick rate
                    QS_END_NOCRIT_()
                }

                QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_POST,
                                 QS::priv_.locFilter[QS::TE_OBJ], t)
                    QS_TIME_();       // timestamp
                    QS_OBJ_(t);       // the time event object
                    QS_SIG_(t->sig);  // signal of this time event
                    QS_OBJ_(act);     // the target AO
                    QS_U8_(static_cast<uint8_t>(tickRate)); // tick rate
                QS_END_NOCRIT_()

                taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

                // asserts if queue overflows
                (void)act->POST_FROM_ISR(t, par, sender);
            }
            else {
                prev = t; // advance to this time event
                taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
            }
        }
        // re-enter crit. section to continue
        uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();
    }
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
}
//............................................................................
QEvt *QF::newXfromISR_(uint_fast16_t const evtSize,
                   uint_fast16_t const margin, enum_t const sig)
{
    uint_fast8_t idx;

    // find the pool id that fits the requested event size ...
    for (idx = static_cast<uint_fast8_t>(0); idx < QF_maxPool_; ++idx) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF_pool_[idx])) {
            break;
        }
    }
    // cannot run out of registered pools
    Q_ASSERT_ID(710, idx < QF_maxPool_);

#ifdef Q_SPY
    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();
    QS_BEGIN_NOCRIT_(QS_QF_NEW, static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();                              // timestamp
        QS_EVS_(static_cast<QEvtSize>(evtSize)); // the size of the event
        QS_SIG_(static_cast<QSignal>(sig));      // the signal of the event
    QS_END_NOCRIT_()
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
#endif // Q_SPY

    // get e -- platform-dependent
    QEvt *e = static_cast<QEvt *>(
              QF_pool_[idx].getFromISR(((margin != QF_NO_MARGIN)
                  ? margin
                  : static_cast<uint_fast16_t>(0))));

    // was e allocated correctly?
    if (e != static_cast<QEvt const *>(0)) {
        e->sig     = static_cast<QSignal>(sig); // set the signal
        // store pool ID
        e->poolId_ = static_cast<uint8_t>(
                       idx + static_cast<uint_fast8_t>(1));
        // initialize the reference counter to 0
        e->refCtr_ = static_cast<uint8_t>(0);
    }
    else {
        // event was not allocated, assert that the caller provided non-zero
        // margin, which means that they can tollerate bad allocation
        Q_ASSERT_ID(720, margin != static_cast<uint_fast16_t>(0));
    }
    return e;
}
//............................................................................
void QF::gcFromISR(QEvt const * const e) {
    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();

        // isn't this the last reference?
        if (e->refCtr_ > static_cast<uint8_t>(1)) {
            QF_EVT_REF_CTR_DEC_(e); // decrement the ref counter

            QS_BEGIN_NOCRIT_(QS_QF_GC_ATTEMPT,
                static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_();        // timestamp
                QS_SIG_(e->sig);   // the signal of the event
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_END_NOCRIT_()

            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
        }
        // this is the last reference to this event, recycle it
        else {
            uint_fast8_t idx = static_cast<uint_fast8_t>(e->poolId_)
                               - static_cast<uint_fast8_t>(1);

            QS_BEGIN_NOCRIT_(QS_QF_GC,
                static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_();        // timestamp
                QS_SIG_(e->sig);   // the signal of the event
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_END_NOCRIT_()

            taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

            // pool ID must be in range
            Q_ASSERT_ID(810, idx < QF_maxPool_);

#ifdef Q_EVT_VIRTUAL
            // explicitly exectute the destructor'
            // NOTE: casting 'const' away is legitimate,
            // because it's a pool event
            QF_EVT_CONST_CAST_(e)->~QEvt(); // xtor,
#endif
            // cast 'const' away, which is OK, because it's a pool event
            QF_pool_[idx].putFromISR(QF_EVT_CONST_CAST_(e));
        }
    }
}
//............................................................................
void QMPool::putFromISR(void *b) {
    /// @pre # free blocks cannot exceed the total # blocks and
    /// the block pointer must be in range to come from this pool.
    ///
    Q_REQUIRE_ID(900, (m_nFree < m_nTot)
                      && QF_PTR_RANGE_(b, m_start, m_end));

    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();
    static_cast<QFreeBlock*>(b)->m_next =
        static_cast<QFreeBlock *>(m_free_head); // link into the free list
    m_free_head = b; // set as new head of the free list
    ++m_nFree;       // one more free block in this pool

    QS_BEGIN_NOCRIT_(QS_QF_MPOOL_PUT,
                     QS::priv_.locFilter[QS::MP_OBJ], m_start)
        QS_TIME_();       // timestamp
        QS_OBJ_(m_start); // the memory managed by this pool
        QS_MPC_(m_nFree); // the number of free blocks in the pool
    QS_END_NOCRIT_()

    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);
}
//............................................................................
void *QMPool::getFromISR(uint_fast16_t const margin) {
    QFreeBlock *fb;
    UBaseType_t uxSavedInterruptState = taskENTER_CRITICAL_FROM_ISR();

    // have the than margin?
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        fb = static_cast<QFreeBlock *>(m_free_head);  // get a free block

        // the pool has some free blocks, so a free block must be available
        Q_ASSERT_ID(910, fb != static_cast<QFreeBlock *>(0));

        void *fb_next = fb->m_next; // put volatile to a temporary to avoid UB

        // is the pool becoming empty?
        --m_nFree;  // one free block less
        if (m_nFree == static_cast<QMPoolCtr>(0)) {
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_ID(920, fb_next == static_cast<QFreeBlock *>(0));

            m_nMin = static_cast<QMPoolCtr>(0);// remember that pool got empty
        }
        else {
            // pool is not empty, so the next free block must be in range
            //
            // NOTE: the next free block pointer can fall out of range
            // when the client code writes past the memory block, thus
            // corrupting the next block.
            Q_ASSERT_ID(930, QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the number of free blocks the new minimum so far?
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; // remember the minimum so far
            }
        }

        m_free_head = fb_next; // adjust list head to the next free block

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET,
                         QS::priv_.locFilter[QS::MP_OBJ], m_start)
            QS_TIME_();        // timestamp
            QS_OBJ_(m_start);  // the memory managed by this pool
            QS_MPC_(m_nFree);  // the number of free blocks in the pool
            QS_MPC_(m_nMin);   // the mninimum # free blocks in the pool
        QS_END_NOCRIT_()
    }
    else {
        fb = static_cast<QFreeBlock *>(0);

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET_ATTEMPT,
                         QS::priv_.locFilter[QS::MP_OBJ], m_start)
            QS_TIME_();        // timestamp
            QS_OBJ_(m_start);  // the memory managed by this pool
            QS_MPC_(m_nFree);  // the # free blocks in the pool
            QS_MPC_(margin);   // the requested margin
        QS_END_NOCRIT_()
    }
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

    return fb; // return the block or NULL pointer to the caller
}

} // namespace QP

