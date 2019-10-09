// *****************************************************************************
//
// Project: PFPP
//
// Module: BSP.
//
// *****************************************************************************

//! \file
//! \brief BSP class.
//! \ingroup application_bsp

// *****************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// CMSIS Library.
#include "lm3s_cmsis.h"

// TI Library.
#include <hw_ints.h>
//#include "hw_memmap.h" // duplicated defines in lm3s_cmsis.h
#include <hw_types.h>
#include <driverlib/flash.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/uart.h>

#include <net/EthernetAddress.h>

#include "inc/Button.h"
#include "inc/GPIO.h"

#include "netif/EthDrv.h"
#include "SDC.h"
#include "SPI.h"
#include "DS3234.h"
#include "SSD1329.h"
#include "TB6612.h"

#include "DisplayMgr_AOs.h"
#include "Logging_AOs.h"
#include "PFPP_AOs.h"
#include "RTCC_AO.h"
#include "LwIP_AOs.h"

#include "PFPP_Events.h"
#include "Signals.h"
#include "IBSP.h"
#include "BSP.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
// DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
// See NOTE00
enum KernelUnawareISRs {
    UART0_PRIO,
    // ...
    // Keep always last.
    MAX_KERNEL_UNAWARE_CMSIS_PRI
};


// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts.
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);

// See NOTE00.
enum KernelAwareISRs {
    SYSTICK_PRIO = QF_AWARE_ISR_CMSIS_PRI,
    GPIOPORTA_PRIO,
    GPIOPORTE_PRIO,
    GPIOPORTF_PRIO,
    // ...
    // Keep always last.
    MAX_KERNEL_AWARE_CMSIS_PRI
};

// "kernel-aware" interrupts should not overlap the PendSV priority.
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));


#define UART_BAUD_RATE      115200U
#ifdef Q_SPY

#define UART_TXFIFO_DEPTH   16U

#endif // Q_SPY

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

namespace BSP {


class SSI0PinCfg
    : public CoreLink::SSIPinCfg {

public:
    SSI0PinCfg() : CoreLink::SSIPinCfg(0) {}
    ~SSI0PinCfg() {}

    void SetPins(void) const override {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        GPIOPinTypeSSI(
            sSSIPins.mPort,
            sSSIPins.mClkPin | sSSIPins.mRxPin | sSSIPins.mTxPin
        );

        GPIOPadConfigSet(
            sSSIPins.mPort,
            sSSIPins.mClkPin | sSSIPins.mTxPin,
            GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD
        );
    }

private:
    // PA2: SSI0CLK
    // PA4: SSI0RX
    // PA5: SSI0TX
    static struct SSIGPIO {
        unsigned long mPort;
        unsigned int  mClkPin;
        unsigned int  mRxPin;
        unsigned int  mTxPin;
    } constexpr sSSIPins = {
        GPIO_PORTA_BASE,
        GPIO_PIN_2, // Clk.
        GPIO_PIN_4, // Rx.
        GPIO_PIN_5  // Tx.
    };
};


class Factory
    : public IBSPFactory {

public:
    virtual ~Factory() {
        // Empty Dtor.
    }


    static std::shared_ptr<IBSPFactory> Instance(void) {
        if (nullptr == Factory::mInstance.get()) {
            // Assigning the instance this way rather than with std::make_shared<>()
            // allows to make the Ctor private.
            BSP::Factory::mInstance = std::shared_ptr<IBSPFactory> (new Factory);
        }

        return Factory::mInstance;
    }


    // IBSPFactory Interface.
    std::shared_ptr<RTCC::AO::RTCC_AO> CreateRTCCAO(void) override {
        if (mRTCCAO.get() == nullptr) {
            mRTCC = CreateRTCC();
            mRTCCAO = std::make_shared<RTCC::AO::RTCC_AO>(*mRTCC);
        }
        return mRTCCAO;
    }


    // This could be used to get a temporary opaque pointer to RTCC AO,
    // For direct posting to AO for instance:
    //     QP::QActive * lPtr = GetOpaqueRTCCAO();
    //     lPtr->Post(myEvent);
    QP::QActive *GetOpaqueRTCCAO(void) override { return mRTCCAO.get(); }


    std::shared_ptr<SDC> CreateSDC(void) override {
        if (mSDC.get() == nullptr) {
            GPIO lCSnPin(GPIO_PORTD_BASE, GPIO_PIN_0);
            //auto lSPIDev = GetSPIDev();
            mSDC = std::make_shared<SDC>(0, *mSPIDev, lCSnPin);
        }
        return mSDC;
    }


    std::shared_ptr<QP::QActive> CreateLogFileSinkAO(void) override {
        if (mFileLogSink.get() == nullptr) {
            mFileLogSink = std::make_shared<Logging::AO::FileSink_AO>();
        }
        return mFileLogSink;
    }


    std::shared_ptr<QP::QActive> CreatePFPPAO(FeedCfgRec &aFeedCfgRec) override {
        if (mPFPPAO.get() == nullptr) {
            mMotorControl = CreateMotorControl();
            mPFPPAO = std::make_shared<PFPP::AO::Mgr_AO>(*mMotorControl, &aFeedCfgRec);
        }
        return mPFPPAO;
    }


    QP::QActive *GetOpaquePFPPAO(void) override { return mPFPPAO.get(); }


    std::shared_ptr<QP::QActive> CreateDisplayMgrAO(void) override {
        if (mDisplayMgrAO.get() == nullptr) {
            mDisplay = CreateDisplay();
            mDisplayMgrAO = std::make_shared<Display::AO::Mgr_AO>(*mDisplay);
        }
        return mDisplayMgrAO;
    }


    QP::QActive *GetOpaqueDisplayMgrAO(void) override { return mDisplayMgrAO.get(); }


    std::shared_ptr<QP::QActive> CreateLwIPMgrAO(void) override {
        if (mEthDrv.get() == nullptr) {
            mEthDrv = CreateEthDrv();
            mLwIPMgrAO = std::make_shared<LwIP::AO::Mgr_AO>(*mEthDrv);
        }
        return mLwIPMgrAO;
    }


    // Local interface.
    static GPIO const &GetRTCCInterruptPin(void) { return mRTCCInterruptPin; }

private:
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    explicit Factory()
        : mSSIPinCfg(nullptr)
        , mSPIDev(nullptr)
        , mRTCC(nullptr)
        , mRTCCAO(nullptr)
        , mFileLogSink(nullptr)
        , mMotorControl(nullptr)
        , mPFPPAO(nullptr)
        , mDisplay(nullptr)
        , mDisplayMgrAO(nullptr)
        , mSDC(nullptr) {

        // Ctor.
        // SPIDev is required with several devices.
        mSPIDev = std::unique_ptr<CoreLink::SPIDev>(CreateSPIDev(0));
    }


    CoreLink::SPIDev * CreateSPIDev(unsigned int aSSIID) {
        switch (aSSIID) {
        case 0:
            // Create pin configuration.
            // Initialize SPI Master.
            SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
            mSSIPinCfg = std::make_unique<SSI0PinCfg>();
            return new CoreLink::SPIDev(SSI0_BASE, *mSSIPinCfg);

        case 1:
        default:
            return nullptr;
        }
    }


    std::unique_ptr<DS3234> CreateRTCC(void) {
        // Creates a DS3234 RTCC.
        // Calls the Ctor that uses default SPI slave configuration,
        // with specified CSn pin.
        unsigned long lInterruptNumber = INT_GPIOA;
        GPIO lCSnPin(GPIO_PORTA_BASE, GPIO_PIN_7);
        auto lRTCC = std::make_unique<DS3234>(
            2000,
            lInterruptNumber,
            mRTCCInterruptPin,
            *mSPIDev,
            lCSnPin
        );

        return lRTCC;
    }

    std::unique_ptr<TB6612> CreateMotorControl(void) {
        GPIO lIn1(GPIO_PORTB_BASE, GPIO_PIN_6);
        GPIO lIn2(GPIO_PORTB_BASE, GPIO_PIN_5);
        GPIO lPWM(GPIO_PORTB_BASE, GPIO_PIN_0);

        return std::make_unique<TB6612>(lIn1, lIn2, lPWM);
    }


    std::unique_ptr<SSD1329> CreateDisplay() {
        GPIO lOLEDCSnPin(GPIO_PORTA_BASE, GPIO_PIN_3);
        GPIO lDCnPin(GPIO_PORTC_BASE, GPIO_PIN_7);
        GPIO lEn15VPin(GPIO_PORTC_BASE, GPIO_PIN_6);
        static unsigned int constexpr sDisplayWidth = 128;
        static unsigned int constexpr sDisplayHeight = 96;

        auto lDisplay = std::make_unique<SSD1329>(
            *mSPIDev,
            lOLEDCSnPin,
            lDCnPin,
            lEn15VPin,
            sDisplayWidth,
            sDisplayHeight
        );
        return lDisplay;
    }


    std::unique_ptr<EthDrv> CreateEthDrv(void) {
        EthernetAddress lMAC = GetMACAddress();
        unsigned int lMyNetIFIndex = 0;
        unsigned int lPBufQueueSize = 8;
        auto lEthDrv = std::make_unique<EthDrv>(
            lMyNetIFIndex,
            lMAC,
            lPBufQueueSize
        );

        return lEthDrv;
    }


    EthernetAddress GetMACAddress(void) {
        // For the Stellaris Eval Kits, the MAC address will be stored in the
        // non-volatile USER0 and USER1 registers. These registers can be read
        // using the FlashUserGet function, as illustrated below.
        unsigned long lUser0 = 0;
        unsigned long lUser1 = 0;
        FlashUserGet(&lUser0, &lUser1);

        // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC address.
        EthernetAddress lMAC(
            static_cast<uint8_t>((lUser0 & 0x000000FFL) >>  0),
            static_cast<uint8_t>((lUser0 & 0x0000FF00L) >>  8),
            static_cast<uint8_t>((lUser0 & 0x00FF0000L) >> 16),
            static_cast<uint8_t>((lUser0 & 0xFF000000L) >> 24),
            static_cast<uint8_t>((lUser1 & 0x000000FFL) >>  0),
            static_cast<uint8_t>((lUser1 & 0x0000FF00L) >>  8)
        );

        if (lMAC.IsValid()) {
            return lMAC;
        }

        return EthernetAddress(0x00, 0x50, 0x1d, 0xc2, 0x70, 0xff);
    }

    std::unique_ptr<CoreLink::SSIPinCfg> mSSIPinCfg;
    std::unique_ptr<CoreLink::SPIDev> mSPIDev;
    std::unique_ptr<DS3234> mRTCC;
    std::shared_ptr<RTCC::AO::RTCC_AO> mRTCCAO;
    std::shared_ptr<Logging::AO::FileSink_AO> mFileLogSink;
    std::unique_ptr<TB6612> mMotorControl;
    std::shared_ptr<PFPP::AO::Mgr_AO> mPFPPAO;
    std::unique_ptr<SSD1329> mDisplay;
    std::shared_ptr<Display::AO::Mgr_AO> mDisplayMgrAO;
    std::shared_ptr<SDC> mSDC;
    std::unique_ptr<EthDrv> mEthDrv;
    std::shared_ptr<LwIP::AO::Mgr_AO> mLwIPMgrAO;

    static GPIO const mRTCCInterruptPin;

    static std::shared_ptr<IBSPFactory> mInstance;
};

} // namespace BSP

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

namespace BSP {
static void InitEtherLED(void);
static void InitUserLED(void);
static void SetUserLED(void);
static void ClrUserLED(void);
} // namespace BSP

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

#ifdef Q_SPY
// For local extern "C" functions, not part of any namespace.
static QP::QSTimeCtr QS_tickTime_ = 0;
static QP::QSTimeCtr QS_tickPeriod_ = 0; //SystemCoreClock / BSP_TICKS_PER_SEC;

// event-source identifiers used for tracing
static uint8_t const sSysTick_Handler      = 0U;
static uint8_t const sGPIOPortA_IRQHandler = 0U;

#endif // Q_SPY


// Button class should become GPIO class.
std::shared_ptr<IBSPFactory> BSP::Factory::mInstance = nullptr;
GPIO const BSP::Factory::mRTCCInterruptPin(GPIO_PORTA_BASE, GPIO_PIN_6);


namespace BSP {

// Those variables are used locally in various stubs and IRQ handlers.
static Button const mManualFeedButton(GPIO_PORTC_BASE, GPIO_PIN_4, INT_GPIOC, 0);
static Button const mTimedFeedButton(GPIO_PORTD_BASE, GPIO_PIN_4, INT_GPIOD, 0);
static Button const mSelectButton(GPIO_PORTF_BASE, GPIO_PIN_1, INT_GPIOF, 0);

static GPIO const sUserLEDPin(GPIO_PORTF_BASE, GPIO_PIN_0);
} // namespace BSP

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace BSP {

std::shared_ptr<IBSPFactory> Init(void) {
    // NOTE: SystemInit() already called from the startup code,
    // where clock already set (CLOCK_SETUP in lm3s_config.h)
    // SystemCoreClockUpdate() also called from there.
    // Settings done for 50MHz system clock.
#if 0
    SysCtlClockSet(
        SYSCTL_SYSDIV_1
        | SYSCTL_USE_OSC
        | SYSCTL_OSC_MAIN
        | SYSCTL_XTAL_8MHZ
    );
#endif

    // Enable the clock to the peripherals used by the application.

    // Enable all required GPIO.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);

    // Debug UART port.
    GPIO lU0RxGPIO(GPIO_PORTA_BASE, GPIO_PIN_0);
    GPIO lU0TxGPIO(GPIO_PORTA_BASE, GPIO_PIN_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinTypeUART(lU0RxGPIO.GetPort(), lU0RxGPIO.GetPin() | lU0TxGPIO.GetPin());
    //UARTStdioInit(0);
    // Enable UART0:
    // @115200, 8-N-1.
    // Interrupt on rx FIFO half-full.
    // UART interrupts: rx and rx-to.
    // Flush the buffers.
    UARTConfigSetExpClk(
        UART0_BASE,
        SysCtlClockGet(),
        UART_BAUD_RATE,
        (UART_CONFIG_PAR_NONE
        | UART_CONFIG_STOP_ONE
        | UART_CONFIG_WLEN_8)
    );
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);
    UARTEnable(UART0_BASE);

    // Enable interrupts.
    UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART0);

    // Initialize the QS software tracing.
    if (0U == QS_INIT(nullptr)) {
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&sSysTick_Handler);
    QS_OBJ_DICTIONARY(&sGPIOPortA_IRQHandler);
    //QS_OBJ_DICTIONARY(&sGPIOPortC_IRQHandler);
    //QS_OBJ_DICTIONARY(&sGPIOPortD_IRQHandler);
    //QS_OBJ_DICTIONARY(&sGPIOPortF_IRQHandler);

    return Factory::Instance();
}


}  // namespace BSP

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

namespace BSP {


static void InitEtherLED(void) {

    // GPIO for Ethernet LEDs.
    GPIO const lLinkLEDPin(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(lLinkLEDPin.GetPort(), lLinkLEDPin.GetPin());
    GPIOPadConfigSet(
        lLinkLEDPin.GetPort(),
        lLinkLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    GPIOPinTypeEthernetLED(lLinkLEDPin.GetPort(), lLinkLEDPin.GetPin());

    GPIO const lActivityLEDPin(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(lActivityLEDPin.GetPort(), lActivityLEDPin.GetPin());
    GPIOPadConfigSet(
        lActivityLEDPin.GetPort(),
        lActivityLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    GPIOPinTypeEthernetLED(lActivityLEDPin.GetPort(), lActivityLEDPin.GetPin());

    IntEnable(INT_ETH);
}


static void InitUserLED(void) {
    // GPIO for user LED toggling during idle.
    GPIOPinTypeGPIOOutput(sUserLEDPin.GetPort(), sUserLEDPin.GetPin());
    GPIOPadConfigSet(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        sUserLEDPin.GetPin()
    );
}


static void SetUserLED(void) {

    GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        sUserLEDPin.GetPin()
    );
}


static void ClrUserLED(void) {

    GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        0
    );
}

} // namespace BSP


// QF callbacks ==============================================================
void QP::QF::onStartup(void) {

    // Set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    SysTickPeriodSet(SysCtlClockGet() / BSP::TICKS_PER_SEC);
    IntPrioritySet(FAULT_SYSTICK, 0x80); //SYSTICK_PRIO
    SysTickIntEnable();
    SysTickEnable();

    // Assign all priority bits for preemption-prio. and none to sub-prio.
    //NVIC_SetPriorityGrouping(0U); from CMSIS
    IntPriorityGroupingSet(0U);
    IntPrioritySet(INT_GPIOA, 0x20); //GPIOPORTA_PRIO);
    IntPrioritySet(INT_GPIOC, 0x60); //GPIOPORTA_PRIO);
    IntPrioritySet(INT_GPIOD, 0x60); //GPIOPORTA_PRIO);
    IntPrioritySet(INT_GPIOE, 0x40); //GPIOPORTE_PRIO);
    IntPrioritySet(INT_GPIOF, 0x60); //GPIOPORTF_PRIO);
    IntPrioritySet(INT_ETH, 0x60);

    NVIC_SetPriority(UART0_IRQn, UART0_PRIO);

    // Init user LED.
    // Init Ethernet LEDs.
    BSP::InitUserLED();
    BSP::InitEtherLED();

    // Manual Feed cap sensor input.
    // Timed Feed cap sensor input.
    BSP::mManualFeedButton.EnableInt();
    BSP::mTimedFeedButton.EnableInt();
    BSP::mSelectButton.EnableInt();

#ifdef Q_SPY
    // UART0 interrupt used for QS-RX.
    NVIC_EnableIRQ(UART0_IRQn);
#endif // Q_SPY
}


//............................................................................
// called with interrupts disabled, see NOTE01
void QP::QV::onIdle(void) {

    // Toggle LED for visual effect.
    BSP::SetUserLED();
    BSP::ClrUserLED();

#ifdef Q_SPY
    QF_INT_ENABLE();

    // Parse all the received bytes.
    QS::rxParse();

    // TX done?
    if (UARTSpaceAvail(UART0_BASE)) {
        // Max bytes we can accept.
        uint16_t lFIFOLen = UART_TXFIFO_DEPTH;

        QF_INT_DISABLE();
        // Try to get next block to transmit.
        uint8_t const *lBlockPtr = QS::getBlock(&lFIFOLen);
        QF_INT_ENABLE();

        // Any bytes in the block?
        while (lFIFOLen-- != 0U) {
            // Put into the FIFO.
            UARTCharPut(UART0_BASE, *lBlockPtr++);
        }
    }

#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    // Atomically go to sleep and enable interrupts.
    QV_CPU_SLEEP();
#else
    // Just enable interrupts.
    QF_INT_ENABLE();
#endif // Q_SPY
}


// QS callbacks ==============================================================
#ifdef Q_SPY

//............................................................................
bool QP::QS::onStartup(void const *aArgPtr) {

    // Buffer for Quantum Spy.
    // Buffer for QS receive channel.
    static uint8_t sQSTxBuf[2 * 1024];
    static uint8_t sQSRxBuf[100];

    initBuf(sQSTxBuf, sizeof(sQSTxBuf));
    rxInitBuf(sQSRxBuf, sizeof(sQSRxBuf));

    // To start timestamp at zero.
    uint32_t volatile lTmp = SysTick->CTRL;
    static_cast<void>(lTmp);
    QS_tickPeriod_ = SysTickPeriodGet();
    QS_tickTime_ = QS_tickPeriod_;

    // Setup the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    // Return success.
    return true;
}


//............................................................................
void QP::QS::onCleanup(void) {
}


//............................................................................
// NOTE: invoked with interrupts DISABLED.
QP::QSTimeCtr QP::QS::onGetTime(void) {
    // Not set?
    // TODO: Check if can be done via API call.

    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
        return QS_tickTime_ - static_cast<QSTimeCtr>(SysTickValueGet()); //SysTick->VAL);
    } else {
        // The rollover occured, but the SysTick_ISR did not run yet.
        return QS_tickTime_ + QS_tickPeriod_
            - static_cast<QSTimeCtr>(SysTickValueGet()); //SysTick->VAL);
    }
}


//............................................................................
void QP::QS::onFlush(void) {
    // Tx FIFO depth.
    uint16_t lFIFOPtr = UART_TXFIFO_DEPTH;
    uint8_t const *lBlockPtr = nullptr;

    QF_INT_DISABLE();
    while ((lBlockPtr = QS::getBlock(&lFIFOPtr)) != nullptr) {
        QF_INT_ENABLE();
        // Busy-wait until TX FIFO empty.
        while (!UARTSpaceAvail(UART0_BASE)) {
        }

        // Any bytes in the block?
        while (lFIFOPtr-- != 0U) {
            // Put into the TX FIFO.
            UARTCharPut(UART0_BASE, *lBlockPtr++);
        }

        // Re-load the Tx FIFO depth.
        lFIFOPtr = UART_TXFIFO_DEPTH;
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}


//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QP::QS::onReset(void) {
    NVIC_SystemReset();
}


//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
void QP::QS::onCommand(uint8_t aCmdId, uint32_t aParam1, uint32_t aParam2, uint32_t aParam3) {
    static_cast<void>(aCmdId);
    static_cast<void>(aParam1);
    static_cast<void>(aParam2);
    static_cast<void>(aParam3);

    //TBD
}

#endif // Q_SPY


extern "C" {

// LwIP
u32_t sys_now() {
    return SysTickValueGet();
}


void SysTick_Handler(void);
void SysTick_Handler(void) {

#ifdef Q_SPY
    {
        // Clear SysTick_CTRL_COUNTFLAG.
        // Account for the clock rollover.
        uint32_t volatile lTmp = SysTick->CTRL;
        static_cast<void>(lTmp);
        QS_tickTime_ += QS_tickPeriod_;
    }
#endif // Q_SPY

    // Call QF Tick function.
    QP::QF::TICK_X(0U, &sSysTick_Handler);

    // Uncomment those line if need to publish every single tick.
    // Process time events for rate 0.
    // Publish to suscribers.
    //static QP::QEvt const sTickEvt(TIME_TICK_SIG);
    //QP::QF::PUBLISH(&sTickEvt, &sSysTick_Handler);
}


// GPIO port A interrupt handler.
void GPIOPortA_IRQHandler(void);
void GPIOPortA_IRQHandler(void) {

    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, lIsMasked);
    unsigned int lPin = BSP::Factory::GetRTCCInterruptPin().GetPin();
    if (lPin & lIntStatus) {
        GPIOPinIntClear(GPIO_PORTA_BASE, lPin);

        // Signal to AO that RTCC generated an interrupt.
        // This can be done with direct POST to known RTCC AO, but since there's a single instance,
        // broadcasting the event/signal is just fine too.
        // Otherwise, would require something like: BSP::Factory::Instance()->GetOpaqueRTCCAO()->Post();
        static QP::QEvt const sRTCCAlarmIntEvent(RTCC_INTERRUPT_SIG);
        //QP::QF::PUBLISH(&sRTCCAlarmIntEvent, 0);
        BSP::Factory::Instance()->GetOpaqueRTCCAO()->POST(&sRTCCAlarmIntEvent, 0);
    }
}


// GPIO port C interrupt handler.
void GPIOPortC_IRQHandler(void);
void GPIOPortC_IRQHandler(void) {

    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTC_BASE, lIsMasked);
    unsigned int lPin = BSP::mManualFeedButton.GetPin();
    if (lPin & lIntStatus) {
        GPIOPinIntClear(GPIO_PORTC_BASE, lPin);

        static PFPP::Event::ManualFeedCmd const sOnEvt(FEED_MGR_MANUAL_FEED_CMD_SIG, true);
        static PFPP::Event::ManualFeedCmd const sOffEvt(FEED_MGR_MANUAL_FEED_CMD_SIG, false);
        // Decouple using framework PUBLISH() method instead of direct posting to AO.
        QP::QActive * const lPFPPAO = BSP::Factory::Instance()->GetOpaquePFPPAO();
        if (nullptr != lPFPPAO) {
            if (Button::IS_HIGH == BSP::mManualFeedButton.GetGPIOPinState()) {
                //QP::QF::PUBLISH(&sOnEvt, 0);
                lPFPPAO->POST(&sOnEvt, 0);
            } else {
                //QP::QF::PUBLISH(&sOffEvt, 0);
                lPFPPAO->POST(&sOffEvt, 0);
            }
        }
    }
}


// GPIO port D interrupt handler.
void GPIOPortD_IRQHandler(void);
void GPIOPortD_IRQHandler(void) {

    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTD_BASE, lIsMasked);
    unsigned int lPin = BSP::mTimedFeedButton.GetPin();
    if (lPin & lIntStatus) {
        GPIOPinIntClear(GPIO_PORTD_BASE, lPin);
        // Only interested in the pin coming high.
        QP::QActive * const lPFPPAO = BSP::Factory::Instance()->GetOpaquePFPPAO();
        if (nullptr != lPFPPAO) {
            if (Button::IS_HIGH == BSP::mTimedFeedButton.GetGPIOPinState()) {
                static PFPP::Event::TimedFeedCmd const sEvt(FEED_MGR_TIMED_FEED_CMD_SIG, 0);
                //QP::QF::PUBLISH(&sEvt, 0);
                lPFPPAO->POST(&sEvt, 0);
            }
        }
    }
}


// GPIO port F interrupt handler.
void GPIOPortF_IRQHandler(void);
void GPIOPortF_IRQHandler(void) {

    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTF_BASE, lIsMasked);
    unsigned int lPin = BSP::mSelectButton.GetPin();
    if (lPin & lIntStatus) {
        GPIOPinIntClear(GPIO_PORTF_BASE, lPin);
        // Only interested in the pin coming high.
        QP::QActive * const lDisplayMgrAO = BSP::Factory::Instance()->GetOpaqueDisplayMgrAO();
        if (nullptr != lDisplayMgrAO) {
            if (Button::IS_HIGH == BSP::mSelectButton.GetGPIOPinState()) {
                static QP::QEvt const sEvt(DISPLAY_REFRESH_SIG);
                QP::QF::PUBLISH(&sEvt, 0);
                lDisplayMgrAO->POST(&sEvt, 0);
            }
        }
    }
}


void UART0_IRQHandler(void);
#ifdef Q_SPY
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled.
// Such ISRs don't need to call QK_ISR_ENTRY/QK_ISR_EXIT
// and they cannot post or publish events.
//
void UART0_IRQHandler(void) {

    // Get the raw interrupt status.
    // Clear the asserted interrupts.
    unsigned long lStatus = UARTIntStatus(UART0_BASE, true);
    UARTIntStatus(UART0_BASE, lStatus);

    // While RX FIFO NOT empty.
    while (UARTCharsAvail(UART0_BASE)) {
        unsigned long lLongByte = UARTCharGet(UART0_BASE);
        uint8_t lByte = static_cast<uint8_t>(lLongByte);
        QP::QS::rxPut(lByte);
    }
}

#else // Q_SPY
void UART0_IRQHandler(void) {
    // Intentional empty function body.
}
#endif // Q_SPY


// TODO: make this conditional to Ethernet support.

void Ethernet_IRQHandler(void);
void Ethernet_IRQHandler(void) {
    //ISR_Ethernet();
    //if (mEthernetCallback) {
    //mEtherCallback();
    //}
}


} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
