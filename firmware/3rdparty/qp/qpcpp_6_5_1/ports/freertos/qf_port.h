/// @file
/// @brief QF/C++ port to FreeRTOS 10.x, ARM Cortex-M, IAR-ARM toolset
/// @ingroup ports
/// @cond
///***************************************************************************
/// Last Updated for Version: 6.5.0
/// Date of the Last Update:  2019-03-22
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

#ifndef qf_port_h
#define qf_port_h

// Activate the QF ISR API required for FreeRTOS
#define QF_ISR_API            1

// Activate the QF QActive::stop() API
#define QF_ACTIVE_STOP        1

// FreeRTOS event queue and thread types
#define QF_EQUEUE_TYPE        QEQueue
#define QF_THREAD_TYPE        StaticTask_t

#ifdef QF_ACTIVE_STOP
    #define QF_OS_OBJECT_TYPE bool
#endif

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE         32

// QF interrupt disabling/enabling (task level)
#define QF_INT_DISABLE()      taskDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()       taskENABLE_INTERRUPTS()

// QF critical section for FreeRTOS (task level), see NOTE2
// #define QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(stat_)  taskENTER_CRITICAL()
#define QF_CRIT_EXIT(stat_)   taskEXIT_CRITICAL()

#include "FreeRTOS.h"  // FreeRTOS master include file, see NOTE4/
#include "task.h"      // FreeRTOS task  management

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // this QP port uses the native QF event queue
#include "qmpool.h"    // this QP port uses the native QF memory pool
#include "qf.h"        // QF platform-independent public interface

// the "FromISR" versions of the QF APIs, see NOTE3
#ifdef Q_SPY
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_), (sender_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), QP::QF_NO_MARGIN, \
                     (pxHigherPrioTaskWoken_), (sender_))

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_), (sender_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, sender_) \
        tickXfromISR_((tickRate_), (pxHigherPrioTaskWoken_), (sender_))
#else
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR_((e_), QP::QF_NO_MARGIN, (pxHigherPrioTaskWoken_))

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, dummy) \
        tickXfromISR_((tickRate_), (pxHigherPrioTaskWoken_))
#endif

#define TICK_FROM_ISR(pxHigherPrioTaskWoken_, sender_) \
    TICK_X_FROM_ISR(static_cast<uint_fast8_t>(0), \
                   (pxHigherPrioTaskWoken_), (sender_))

#ifdef Q_EVT_CTOR
    #define Q_NEW_FROM_ISR(evtT_, sig_, ...) \
        (new(QP::QF::newXfromISR_(static_cast<uint_fast16_t>(sizeof(evtT_)),\
                     QP::QF_NO_MARGIN, static_cast<enum_t>(0))) \
            evtT_((sig_),  ##__VA_ARGS__))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_, ...) do { \
        (e_) = static_cast<evtT_ *>( \
                  QP::QF::newXfromISR_(static_cast<uint_fast16_t>( \
                  sizeof(evtT_)), (margin_), static_cast<enum_t>(0))); \
        if ((e_) != static_cast<evtT_ *>(0)) { \
            new((e_)) evtT_((sig_),  ##__VA_ARGS__); \
        } \
     } while (0)

#else // QEvt is a POD (Plain Old Datatype)
    #define Q_NEW_FROM_ISR(evtT_, sig_) \
        (static_cast<evtT_ *>(QP::QF::newXfromISR_( \
                static_cast<uint_fast16_t>(sizeof(evtT_)), \
                QP::QF_NO_MARGIN, (sig_))))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_) \
        ((e_) = static_cast<evtT_ *>(\
        QP::QF::newXfromISR_(static_cast<uint_fast16_t>(sizeof(evtT_)),\
                         (margin_), (sig_))))
#endif

namespace QP {

enum FreeRTOS_TaskAttrs {
    TASK_NAME_ATTR
};

} // namespace QP

// FreeRTOS hooks prototypes (not provided by FreeRTOS)
extern "C" {
#if (configUSE_IDLE_HOOK > 0)
    void vApplicationIdleHook(void);
#endif
#if (configUSE_TICK_HOOK > 0)
    void vApplicationTickHook(void);
#endif
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
#endif
#if (configSUPPORT_STATIC_ALLOCATION > 0)
    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                       StackType_t **ppxIdleTaskStackBuffer,
                                       uint32_t *pulIdleTaskStackSize);
#endif
} // extern "C"

//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL
    // FreeRTOS blocking for event queue implementation (task level)
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt *>(0)) { \
            QF_CRIT_EXIT_(); \
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY); \
            QF_CRIT_ENTRY_(); \
        }

    // FreeRTOS signaling (unblocking) for event queue (task level)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QF_CRIT_EXIT_(); \
        xTaskNotifyGive((TaskHandle_t)&(me_)->m_thread); \
        QF_CRIT_ENTRY_(); \
    } while (0)

    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) vTaskSuspendAll()
    #define QF_SCHED_UNLOCK_()    xTaskResumeAll()

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif /* ifdef QP_IMPL */

//****************************************************************************
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased to 64,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of FreeRTOS task priorities,
// because each QP active object requires a unique priority level.
//
// NOTE2:
// The critical section definition applies only to the FreeRTOS "task level"
// APIs. The "FromISR" APIs are defined separately.
//
// NOTE3:
// The design of FreeRTOS requires using different APIs inside the ISRs
// (the "FromISR" variant) than at the task level. Accordingly, this port
// provides the "FromISR" variants for QP functions and "FROM_ISR" variants
// for QP macros to be used inside ISRs. ONLY THESE "FROM_ISR" VARIANTS
// ARE ALLOWED INSIDE ISRs AND CALLING THE TASK-LEVEL APIs IS AN ERROR.
//

#endif // qf_port_h
