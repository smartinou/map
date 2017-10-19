/// @file
/// @brief QF/C++ port to FreeRTOS.org (v8.2.x), ARM Cortex-M, GNU-ARM toolset
/// @cond
///***************************************************************************
/// Last updated for version 5.4.3
/// Last updated on  2015-06-18
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

#ifndef qf_port_h
#define qf_port_h

// FreeRTOS event queue and thread types
#define QF_EQUEUE_TYPE        QEQueue
#define QF_THREAD_TYPE        TaskHandle_t
#define QF_OS_OBJECT_TYPE     TaskHandle_t

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE         32

// QF critical section for FreeRTOS/ARM-Cortex-M3/M4, see NOTE2
#define QF_CRIT_STAT_TYPE     uint32_t
#define QF_CRIT_ENTRY(stat_)  ((stat_) = ulPortRaiseBASEPRI())
#define QF_CRIT_EXIT(stat_)   vPortSetBASEPRI(stat_)

#define QF_INT_DISABLE()      portDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()       portENABLE_INTERRUPTS()

#include "qf_int_config.h"  // interrupt configuration, see NOTE3
#include "FreeRTOS.h"  // FreeRTOS master include file, see NOTE4
#include "task.h"      // FreeRTOS task  management

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // this QP port uses the native QF event queue
#include "qmpool.h"    // this QP port uses the native QF memory pool
#include "qpset.h"     // this QP port uses the native QF priority set
#include "qf.h"        // QF platform-independent public interface

// Facilities specific to the FreeRTOS port...
#define QF_ISR_ENTRY(stat_) do { \
   (stat_) = portSET_INTERRUPT_MASK_FROM_ISR(); \
   if ((QP::FreeRTOS_extras.isrNest++) == static_cast<BaseType_t>(0)) { \
       QP::FreeRTOS_extras.ctxtReq = pdFALSE; \
   } \
   portCLEAR_INTERRUPT_MASK_FROM_ISR(stat_); \
} while (false)

#define QF_ISR_EXIT(stat_, ctxtReq_) do { \
   (stat_) = portSET_INTERRUPT_MASK_FROM_ISR(); \
   if (QP::FreeRTOS_extras.ctxtReq != pdFALSE) { \
       (ctxtReq_) = pdTRUE; \
   } \
   --QP::FreeRTOS_extras.isrNest; \
   portCLEAR_INTERRUPT_MASK_FROM_ISR(stat_); \
} while (false)

namespace QP {

// FreeRTOS "extras" for handling ISRs for FreeRTOS/ARM-Cortex-M
struct FreeRTOSExtras {
    BaseType_t volatile isrNest;
    BaseType_t volatile ctxtReq;
} ;

extern FreeRTOSExtras FreeRTOS_extras;

// FreeRTOS hooks prototypes (not provided by FreeRTOS)
#if (configUSE_IDLE_HOOK > 0)
    void vApplicationIdleHook(void);
#endif
#if (configUSE_TICK_HOOK > 0)
    void vApplicationTickHook(void);
#endif
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
#endif

// set FreeRTOS taks name; must be be called before QMActive::start()
void QF_setTaskName(QMActive *act, char_t const *taskName);

} // namespace QP

//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL
    // FreeRTOS blocking for event queue implementation
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt *>(0)) { \
            QF_CRIT_EXIT_(); \
            vTaskSuspend(static_cast<TaskHandle_t>(0)); \
            QF_CRIT_ENTRY_(); \
        }

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        if (FreeRTOS_extras.isrNest == static_cast<BaseType_t>(0)) { \
            QF_CRIT_EXIT_(); \
            vTaskResume((me_)->m_osObject); \
            QF_CRIT_ENTRY_(); \
        } \
        else { \
            BaseType_t ctxtReq; \
            QF_CRIT_EXIT_(); \
            ctxtReq = xTaskResumeFromISR((me_)->m_osObject); \
            QF_CRIT_ENTRY_(); \
            if (ctxtReq != pdFALSE) { \
                FreeRTOS_extras.ctxtReq = pdTRUE; \
            } \
        } \
    } while (false)

    #define QACTIVE_EQUEUE_ONEMPTY_(me_) ((void)0)

    // native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif // ifdef QP_IMPL

//****************************************************************************
// NOTE1:
// The maximum number of active objects in QP can be increased to 63,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of FreeRTOS task priorities,
// because each QP active object requires a unique priority level.
//
// NOTE2:
// The critical section definition applies only to ARM Cortex-M3/M4,
// because it assumes the BASEPRI register (which is not available in M0/M0+).
//
// NOTE3:
// The header file "qf_int_config_h" provides the definition of
// configMAX_SYSCALL_INTERRUPT_PRIORITY, which must be consistent between
// this QF port and applications. To guarantee this consistency, the
// the applicatioin-level FreeRTOS header file "FreeRTOSConfig.h" needs to
// also include "qf_int_config_h" instead of re-defining
// configMAX_SYSCALL_INTERRUPT_PRIORITY.
//
// NOTE4:
// The header file "FreeRTOS.h" includes the "FreeRTOSConfig.h" header file
// which creates an artificial dependency on the FreeRTOS configuration. This
// port uses a dummy "FreeRTOSConfig.h" from the "config" sub-directory,
// so that applications can still use their own (and potentially different)
// FreeRTOS configuration at compile time.
//

#endif // qf_port_h
