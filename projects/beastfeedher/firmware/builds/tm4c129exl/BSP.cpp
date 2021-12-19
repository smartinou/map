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
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// CMSIS Library.
#include "TM4C129ENCPDT.h"

// TI Library.
//#define TARGET_IS_BLIZZARD_RA1
//#define DTARGET_IS_SNOWFLAKE_RA0
//#define PART_TM4C129ENCPDT
#include <inc/hw_ints.h>
//#include "inc/hw_memmap.h" // duplicated defines in TM4C129ENCPDT.h
#include <inc/hw_types.h>
#include <driverlib/flash.h>
#include <driverlib/fpu.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/uart.h>

#include <net/EthernetAddress.h>

#include "inc/Button.h"
#include "inc/GPIO.h"

#include "FatFSDisk.h"
#include "SDC.h"
#include "SPI.h"
#include "DS3234.h"
#include "TB6612.h"

#include "Display_AOs.h"
#include "Logging_AOs.h"
//#include "BLE_AOs.h"
#include "LwIP_AOs.h"
#include "LwIP_Events.h"
#include "PFPP_AOs.h"
#include "RTCC_AOs.h"

#include "PFPP_Events.h"
#include "Signals.h"
#include "IBSP.h"
#include "BSP.h"

#include "netif/tm4c129/EthDrv.h"

#ifdef USE_UART0
#include "uartstdio.h"
#endif // USE_UART0

#ifdef USE_RTT
// SEGGER RTT.
#include <SEGGER_RTT.h>
#endif // USE_RTT

#ifdef Q_SPY
#if !defined(USE_UART0) && !defined(USE_RTT)
#error Must define USE_UART0 or USE_RTT for using Q_SPY
#endif
#endif // Q_SPY

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
    //GPIOPORTF_PRIO,
    // ...
    // Keep always last.
    MAX_KERNEL_AWARE_CMSIS_PRI
};

// "kernel-aware" interrupts should not overlap the PendSV priority.
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

static constexpr uint32_t sClkRate = 120000000;
static constexpr unsigned long sUartPortNbr = 0;
static constexpr uint32_t sUartBaudRate = 115200U;

enum RTTTerminal {
    _stdin = 0,
    _stdout = 1,
    _stderr = 2,
    _qspy = 3,
};
static constexpr unsigned int sRTTBufferIndex = 0U;
static constexpr size_t sRTTUpBufferSize = 1024;
static constexpr unsigned int sRTTQSPYTerminal = RTTTerminal::_qspy;

#ifdef Q_SPY

#define UART_TXFIFO_DEPTH   16U

#endif // Q_SPY

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

namespace BSP {


class SSI3PinCfg
    : public CoreLink::SSIPinCfg {

public:
    constexpr SSI3PinCfg() noexcept : CoreLink::SSIPinCfg(3) {}

    void SetPins(void) const override {
        GPIO::EnableSysCtlPeripheral(sSSIPins.mPort);
        MAP_GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
        MAP_GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0);
        MAP_GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1);
        MAP_GPIOPinTypeSSI(
            sSSIPins.mPort,
            sSSIPins.mClkPin | sSSIPins.mRxPin | sSSIPins.mTxPin
        );

        // Set a weak pull-up on MISO pin for SD Card's proper operation.
        MAP_GPIOPadConfigSet(
            sSSIPins.mPort,
            sSSIPins.mRxPin,
            GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD_WPU
        );

        MAP_GPIOPadConfigSet(
            sSSIPins.mPort,
            sSSIPins.mClkPin | sSSIPins.mTxPin,
            GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD
        );
    }

private:
    // PQ0: SSI3CLK
    // PQ3: SSI3RX
    // PQ2: SSI3TX
    static struct SSI3GPIO {
        unsigned long mPort;
        unsigned int  mClkPin;
        unsigned int  mRxPin;
        unsigned int  mTxPin;
    } constexpr sSSIPins = {
        GPIOQ_BASE,
        GPIO_PIN_0, // Clk.
        GPIO_PIN_3, // Rx.
        GPIO_PIN_2  // Tx.
    };
};


class Factory
    : public IBSPFactory {

public:
    virtual ~Factory() {
        // Empty Dtor.
    }


    static std::shared_ptr<IBSPFactory> Instance(void) {
        if (nullptr == Factory::mInstance) {
            // Assigning the instance this way rather than with std::make_shared<>()
            // allows to make the Ctor private.
            BSP::Factory::mInstance = std::shared_ptr<IBSPFactory>(new Factory);
        }

        return Factory::mInstance;
    }


    // IBSPFactory Interface.
    std::shared_ptr<RTCC::AO::RTCC_AO> CreateRTCCAO(void) override {
        if (mRTCCAO == nullptr) {
            mRTCC = CreateRTCC();
            // RTCC also implements both ITemperature and INVMem interfaces.
            mRTCCAO = std::make_shared<RTCC::AO::RTCC_AO>(*mRTCC, *mRTCC, *mRTCC);
        }
        return mRTCCAO;
    }


    // This could be used to get a temporary opaque pointer to RTCC AO,
    // For direct posting to AO for instance:
    //     QP::QActive * lPtr = GetOpaqueRTCCAO();
    //     lPtr->Post(myEvent);
    std::shared_ptr<QP::QActive> GetOpaqueRTCCAO(void) const override { return mRTCCAO; }


    std::shared_ptr<QP::QActive> GetOpaqueBLEAO(void) const override { return nullptr; }
    //QP::QActive *GetOpaqueBLEAO(void) override { return mBLEAO.get(); }


    unsigned int CreateDisks(void) override {
        // This BSP has one SDC device.
        if (mSDC == nullptr) {
            constexpr GPIO lCSnPin(GPIOD_AHB_BASE, GPIO_PIN_0);
            mSDC = std::make_shared<SDC>(0, *mSPIDev, lCSnPin);
        }

        return FatFSDisk::GetDiskQty();
    }


    std::shared_ptr<QP::QActive> CreateLogFileSinkAO(void) override {
        if (mFileLogSink == nullptr) {
            mFileLogSink = std::make_shared<Logging::AO::FileSink_AO>();
        }
        return mFileLogSink;
    }


    std::shared_ptr<QP::QActive> CreatePFPPAO(FeedCfgRec &aFeedCfgRec) override {
        if (mPFPPAO == nullptr) {
            mMotorControl = CreateMotorControl();
            mPFPPAO = std::make_shared<PFPP::AO::Mgr_AO>(*mMotorControl, &aFeedCfgRec);
        }
        return mPFPPAO;
    }


    std::shared_ptr<QP::QActive> GetOpaquePFPPAO(void) const { return mPFPPAO; }


    std::shared_ptr<QP::QActive> CreateDisplayMgrAO(void) override {
        return std::shared_ptr<Display::AO::Mgr_AO>(nullptr);
    }


    std::shared_ptr<QP::QActive> GetOpaqueDisplayMgrAO(void) const override { return nullptr; }


    std::shared_ptr<QP::QActive> CreateLwIPMgrAO(void) override {
        // Only one instance of this object can exist.
        if (mLwIPMgrAO == nullptr) {
            // Create all Ethernet drivers required before the AO.
            // LwIP::AO::Mgr doesn't use any local references to LwIPDrv.
            // They are referenced via LwIPDrv static functions.
            CreateEthDrv();
            mLwIPMgrAO = std::make_shared<LwIP::AO::Mgr_AO>();
        }
        return mLwIPMgrAO;
    }


    std::shared_ptr<QP::QActive> CreateBLEAO(void) override {
        return nullptr;//std::make_shared<QP::QActive>(nullptr);//std::shared_ptr<PFPP::AO::BLE_AO>(nullptr);
    }


    // Local interface.
    static constexpr GPIO GetRTCCInterruptPin(void) { return mRTCCInterruptPin; }
    //static GPIO const &GetBLEInterruptPin(void) { return mBLEInterruptPin; }

    static uint8_t const sSysTick_Handler;
    static uint8_t const sGPIOPortP3_IRQHandler;

private:
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    explicit Factory(uint32_t aClkRate = sClkRate)
        : mClkRate(aClkRate)
        , mSPIDev(nullptr)
        , mRTCC(nullptr)
        , mRTCCAO(nullptr)
        , mFileLogSink(nullptr)
        , mMotorControl(nullptr)
        , mPFPPAO(nullptr)
        , mDisplayMgrAO(nullptr)
        , mSDC(nullptr) {

        // Ctor.
        Init();

        // SPIDev is required with several devices.
        mSPIDev = std::unique_ptr<CoreLink::SPIDev>(CreateSPIDev(3));

        // TODO: CONSIDER CREATING RTCC HERE TOO.
    }


    void Init(void) {
        // NOTE: SystemInit() already called from the startup code,
        // where clock already set (CLOCK_SETUP in lm3s_config.h)
        // SystemCoreClockUpdate() also called from there.
        // Settings done for 50MHz system clock.

        mClkRate = SysCtlClockFreqSet(
            SYSCTL_XTAL_25MHZ
                | SYSCTL_OSC_MAIN
                | SYSCTL_USE_PLL
                | SYSCTL_CFG_VCO_480,
            mClkRate
        );

        // Floating point unit.
        MAP_FPUEnable();
        MAP_FPULazyStackingEnable();

        // Enable the clock to the peripherals used by the application.

        // Enable all required GPIO.
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        //MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        //MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
        //MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

#ifdef USE_UART0
        // Debug UART port.
        constexpr GPIO lU0RxGPIO(GPIOA_AHB_BASE, GPIO_PIN_0);
        constexpr GPIO lU0TxGPIO(GPIOA_AHB_BASE, GPIO_PIN_1);
        MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
        MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
        MAP_GPIOPinTypeUART(
            lU0RxGPIO.GetPort(),
            lU0RxGPIO.GetPin() | lU0TxGPIO.GetPin()
        );
        // Enable UART0:
        // @115200, 8-N-1.
        // Flush the buffers.
        UARTStdioInit(sUartPortNbr, mClkRate, sUartBaudRate);
#endif // USE_UART0

#ifdef USE_RTT
    // Leave here until RTT has other use than QSPY.
    SEGGER_RTT_Init();
    static constexpr auto sRTTUpBufferName = "RTTUpBuffer";
    static uint8_t sRTTUpBuffer[sRTTUpBufferSize] = {0};
    int lResult = SEGGER_RTT_ConfigUpBuffer(
        sRTTBufferIndex,
        sRTTUpBufferName,
        &sRTTUpBuffer[0],
        sizeof(sRTTUpBufferSize),
        SEGGER_RTT_MODE_NO_BLOCK_SKIP
    );

    if (lResult < 0) {
        // Error while configuring buffer.
        return;
    }

    static constexpr auto sRTTDownBufferName = "RTTDownBuffer";
    static constexpr size_t sRTTDownBufferSize = 64;
    static uint8_t sRTTDownBuffer[sRTTDownBufferSize] = {0};
    lResult = SEGGER_RTT_ConfigDownBuffer(
        sRTTBufferIndex,
        sRTTDownBufferName,
        &sRTTDownBuffer[0],
        sizeof(sRTTDownBufferSize),
        SEGGER_RTT_MODE_NO_BLOCK_SKIP
    );

    if (lResult < 0) {
        // Error while configuring buffer.
        return;
    }

    SEGGER_RTT_printf(sRTTBufferIndex, "Hello, World!\n");
#endif // USE_RTT

#ifdef Q_SPY
#ifdef USE_UART0
        // Enable interrupts.
        // Interrupt on rx FIFO half-full.
        // UART interrupts: rx and rx-to.
        MAP_UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
        MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
        MAP_IntEnable(INT_UART0);
#endif // USE_UART0

        // Initialize the QS software tracing.
        if (0U == QS_INIT(nullptr)) {
            Q_ERROR();
        }
        QS_OBJ_DICTIONARY(&BSP::Factory::sSysTick_Handler);
        QS_OBJ_DICTIONARY(&sGPIOPortP3_IRQHandler);
        //QS_OBJ_DICTIONARY(&sGPIOPortC_IRQHandler);
        //QS_OBJ_DICTIONARY(&sGPIOPortD_IRQHandler);
        //QS_OBJ_DICTIONARY(&sGPIOPortF_IRQHandler);

        QS_OBJ_DICTIONARY(mPFPPAO.get());
        QS_OBJ_DICTIONARY(mLwIPMgrAO.get());
#endif // Q_SPY
    }


    CoreLink::SPIDev * CreateSPIDev(unsigned int aSSIID) {
        switch (aSSIID) {
        case 3: {
            // Create pin configuration.
            // Initialize SPI Master.
            SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
            constexpr SSI3PinCfg lSSIPinCfg;
            return new CoreLink::SPIDev(SSI3_BASE, mClkRate, lSSIPinCfg);
        }
        case 0:
        case 1:
        case 2:
        default:
            return nullptr;
        }
    }


    std::unique_ptr<DS3234> CreateRTCC(void) {
        // Creates a DS3234 RTCC.
        // Reset input pin.
        constexpr GPIO lResetPin(GPIOK_BASE, GPIO_PIN_7);
        // Calls the Ctor that uses default SPI slave configuration,
        // with specified CSn pin.
        static constexpr unsigned long lInterruptNumber = INT_GPIOP3;
        constexpr GPIO lCSnPin(GPIOQ_BASE, GPIO_PIN_1);
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
        constexpr GPIO lIn1(GPIOB_AHB_BASE, GPIO_PIN_6);
        constexpr GPIO lIn2(GPIOB_AHB_BASE, GPIO_PIN_5);
        constexpr GPIO lPWM(GPIOB_AHB_BASE, GPIO_PIN_0);

        return std::make_unique<TB6612>(lIn1, lIn2, lPWM);
    }


    // When LwIP::AO::Mgr can get rid of any local reference to LwIPDrv,
    // it won't be necessary for this function to return a pointer to EthDrv.
    // It will be referenced via LwIPDrv static functions.
    void CreateEthDrv(void) {
        EthernetAddress lMAC = GetMACAddress();
        static constexpr unsigned int lMyNetIFIndex = 0;
        static constexpr unsigned int lPBufQueueSize = 8;
        mEthDrv = std::make_unique<EthDrv>(
            lMyNetIFIndex,
            lMAC,
            lPBufQueueSize,
            mClkRate
        );
    }


#if 0
    std::unique_ptr<BLE::BLE> CreateBLE(void) {
        unsigned long lInterruptNumber = INT_GPIOB;
        constexpr GPIO lCSnPin(GPIOB_AHB_BASE, GPIO_PIN_4);
        auto lBLE = std::make_unique<BLE::BLE>(
            lInterruptNumber,
            mBLEInterruptPin,
            *mSPIDev,
            lCSnPin
        );

        return lBLE;
    }
#endif


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
            static_cast<uint8_t>((lUser1 & 0x000000FFL) >>  0),
            static_cast<uint8_t>((lUser1 & 0x0000FF00L) >>  8),
            static_cast<uint8_t>((lUser1 & 0x00FF0000L) >> 16)
        );

        if (lMAC.IsValid()) {
            return lMAC;
        }

        constexpr EthernetAddress lDefaultMAC(0x00, 0x50, 0x1d, 0xc2, 0x70, 0xff);
        return lDefaultMAC;
    }

    uint32_t mClkRate;
    std::unique_ptr<CoreLink::SPIDev> mSPIDev;
    std::unique_ptr<DS3234> mRTCC;
    std::shared_ptr<RTCC::AO::RTCC_AO> mRTCCAO;
    std::shared_ptr<Logging::AO::FileSink_AO> mFileLogSink;
    std::unique_ptr<TB6612> mMotorControl;
    std::shared_ptr<PFPP::AO::Mgr_AO> mPFPPAO;
    std::shared_ptr<Display::AO::Mgr_AO> mDisplayMgrAO;
    std::shared_ptr<SDC> mSDC;
    std::unique_ptr<EthDrv> mEthDrv;
    std::shared_ptr<LwIP::AO::Mgr_AO> mLwIPMgrAO;
    //std::unique_ptr<BLE::BLE> mBLE;
    //std::shared_ptr<PFPP::AO::BLE_AO> mBLEAO;

    static constexpr GPIO mRTCCInterruptPin{GPIOP_BASE, GPIO_PIN_3};
    //static constexpr GPIO mBLEInterruptPin{GPIOB_AHB_BASE, GPIO_PIN_1};

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

#ifdef Q_SPY
static void TxData(void);
#endif // Q_SPY

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

static time_t sSystemTime = 0;

#ifdef Q_SPY
// For local extern "C" functions, not part of any namespace.
static QP::QSTimeCtr QS_tickTime_ = 0;
static QP::QSTimeCtr const QS_tickPeriod_ = SystemCoreClock / BSP::TICKS_PER_SEC;

namespace BSP {
    // event-source identifiers used for tracing
    uint8_t const Factory::sSysTick_Handler = 0U;
    uint8_t const Factory::sGPIOPortP3_IRQHandler = 0U;
}

#endif // Q_SPY


// Button class should become GPIO class.
std::shared_ptr<IBSPFactory> BSP::Factory::mInstance = nullptr;
//GPIO constexpr BSP::Factory::mRTCCInterruptPin(GPIOP_BASE, GPIO_PIN_3);
//GPIO const BSP::Factory::mBLEInterruptPin(GPIOB_AHB_BASE, GPIO_PIN_1);


namespace BSP {

// Those variables are used locally in various stubs and IRQ handlers.
static Button const mManualFeedButton(GPIOJ_AHB_BASE, GPIO_PIN_0, INT_GPIOJ, 0);
static Button const mTimedFeedButton(GPIOJ_AHB_BASE, GPIO_PIN_1, INT_GPIOJ, 0);

static constexpr GPIO sUserLEDPin(GPION_BASE, GPIO_PIN_1);
} // namespace BSP

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace BSP {

std::shared_ptr<IBSPFactory> Init(void) {
    return Factory::Instance();
}


}  // namespace BSP

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

namespace BSP {


static void InitEtherLED(void) {

    // GPIO for Ethernet LEDs.
    constexpr GPIO lLinkLEDPin(GPIOF_AHB_BASE, GPIO_PIN_4);
    MAP_GPIOPinTypeGPIOOutput(lLinkLEDPin.GetPort(), lLinkLEDPin.GetPin());
    MAP_GPIOPadConfigSet(
        lLinkLEDPin.GetPort(),
        lLinkLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    MAP_GPIOPinConfigure(GPIO_PF4_EN0LED1);
    MAP_GPIOPinTypeEthernetLED(lLinkLEDPin.GetPort(), lLinkLEDPin.GetPin());

    constexpr GPIO lActivityLEDPin(GPIOF_AHB_BASE, GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(lActivityLEDPin.GetPort(), lActivityLEDPin.GetPin());
    MAP_GPIOPadConfigSet(
        lActivityLEDPin.GetPort(),
        lActivityLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    MAP_GPIOPinConfigure(GPIO_PF0_EN0LED0);
    MAP_GPIOPinTypeEthernetLED(lActivityLEDPin.GetPort(), lActivityLEDPin.GetPin());

    MAP_IntEnable(INT_EMAC0);
}


static void InitUserLED(void) {
    // GPIO for user LED toggling during idle.
    MAP_GPIOPinTypeGPIOOutput(sUserLEDPin.GetPort(), sUserLEDPin.GetPin());
    MAP_GPIOPadConfigSet(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
    MAP_GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        sUserLEDPin.GetPin()
    );
}


static void SetUserLED(void) {

    MAP_GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        sUserLEDPin.GetPin()
    );
}


static void ClrUserLED(void) {

    MAP_GPIOPinWrite(
        sUserLEDPin.GetPort(),
        sUserLEDPin.GetPin(),
        0
    );
}

} // namespace BSP


// QF callbacks ==============================================================
void QP::QF::onStartup(void) {

    // Set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    // !!! SysCtlClockGet can not be used for tm4c129 !!!
    // Using constant here assumes SysCtlClockFreqSet() was successful.
    // This is better handled in the lm3s6965 port.
    MAP_SysTickPeriodSet(sClkRate / BSP::TICKS_PER_SEC);
    MAP_IntPrioritySet(FAULT_SYSTICK, 0x80); //SYSTICK_PRIO
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();

    // Assign all priority bits for preemption-prio. and none to sub-prio.
    //NVIC_SetPriorityGrouping(0U); from CMSIS
    MAP_IntPriorityGroupingSet(0U);
    MAP_IntPrioritySet(INT_GPIOA, 0x20); //GPIOPORTA_PRIO);
    MAP_IntPrioritySet(INT_GPIOB, 0x20); //GPIOPORTA_PRIO);
    MAP_IntPrioritySet(INT_GPIOC, 0x60); //GPIOPORTA_PRIO);
    MAP_IntPrioritySet(INT_GPIOD, 0x60); //GPIOPORTA_PRIO);
    MAP_IntPrioritySet(INT_GPIOE, 0x40); //GPIOPORTE_PRIO);
    //MAP_IntPrioritySet(INT_GPIOF, 0x60); //GPIOPORTF_PRIO);
    MAP_IntPrioritySet(INT_EMAC0, 0x60);

    // Init user LED.
    // Init Ethernet LEDs.
    BSP::InitUserLED();
    BSP::InitEtherLED();

    // Manual Feed cap sensor input.
    // Timed Feed cap sensor input.
    BSP::mManualFeedButton.EnableInt();
    BSP::mTimedFeedButton.EnableInt();

#if defined(Q_SPY) && defined(USE_UART0)
    // UART0 interrupt used for QS-RX.
    NVIC_SetPriority(UART0_IRQn, UART0_PRIO);
    NVIC_EnableIRQ(UART0_IRQn);
#endif // Q_SPY && USE_UART0
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
#ifdef USE_RTT
    // No interrupts: poll for received bytes and move to QS rx buffer.
    if (SEGGER_RTT_HasData(sRTTBufferIndex) != 0) {
        uint8_t lByte = 0;
        // Could also check against available space in QS rx buffer.
        while(SEGGER_RTT_Read(sRTTBufferIndex, &lByte, sizeof(lByte)))
        {
            QP::QS::rxPut(lByte);
        }
    }
#endif // USE_RTT
    QS::rxParse();

    // TX done?
    TxData();

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


//............................................................................
extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    NVIC_SystemReset();

    // noreturn: loop forever.
    while(1);
}


#ifdef QF_ACTIVE_STOP
void QP::QActive::stop(void) {
    // Unsubscribe from all events.
    // Remove this object from QF.
    QP::QActive::unsubscribeAll();
    QP::QF::remove_(this);
}
#endif // QF_ACTIVE_STOP

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
    //QS_tickPeriod_ = SysTickPeriodGet();
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
    // Used for QUTest only.
}


//............................................................................
// NOTE: invoked with interrupts DISABLED.
QP::QSTimeCtr QP::QS::onGetTime(void) {

    // Not set?
    // TODO: Check if can be done via API call.

    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
        return QS_tickTime_ - static_cast<QSTimeCtr>(SysTickValueGet());
    } else {
        // The rollover occured, but the SysTick_ISR did not run yet.
        return QS_tickTime_ + QS_tickPeriod_
            - static_cast<QSTimeCtr>(SysTickValueGet()); //SysTick->VAL);
    }
}


//............................................................................
void QP::QS::onFlush(void) {
#ifdef USE_UART0

    // Tx FIFO depth.
    uint16_t lFIFOLen = UART_TXFIFO_DEPTH;
    uint8_t const *lBlockPtr = nullptr;

    QF_INT_DISABLE();
    while ((lBlockPtr = QS::getBlock(&lFIFOLen)) != nullptr) {
        QF_INT_ENABLE();
        // Busy-wait until TX FIFO empty.
        while (!MAP_UARTSpaceAvail(UART0_BASE)) {
        }

        // Any bytes in the block?
        while (lFIFOLen-- != 0U) {
            // Put into the TX FIFO.
            MAP_UARTCharPut(UART0_BASE, *lBlockPtr++);
        }

        // Re-load the Tx FIFO depth.
        lFIFOLen = UART_TXFIFO_DEPTH;
        QF_INT_DISABLE();
    }
#elif defined(USE_RTT)
    // Tx FIFO depth.
    uint16_t lFIFOLen = SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex);
    uint8_t const *lBlockPtr = nullptr;

    QF_INT_DISABLE();
    while ((lBlockPtr = QS::getBlock(&lFIFOLen)) != nullptr) {
        QF_INT_ENABLE();
        // Busy-wait until TX FIFO empty.
        //while (SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex) != (sRTTUpBufferSize - 1)) {
        //}

        //SEGGER_RTT_SetTerminal(sRTTQSPYTerminal);
        unsigned int lLen = SEGGER_RTT_Write(sRTTBufferIndex, lBlockPtr, lFIFOLen);
        //SEGGER_RTT_SetTerminal(0);
        // TODO: do something with len.
        static_cast<void>(lLen);
        // Re-load the Tx FIFO depth.
        lFIFOLen = SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex);
        QF_INT_DISABLE();
    }
#endif

    QF_INT_ENABLE();
}


//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QP::QS::onReset(void) {
    //NVIC_SystemReset();
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


static void TxData(void) {
#ifdef USE_UART0
    // TX done?
    if (MAP_UARTSpaceAvail(UART0_BASE)) {
        // Max bytes we can accept.
        uint16_t lFIFOLen = UART_TXFIFO_DEPTH;

        QF_INT_DISABLE();
        // Get next block to transmit.
        uint8_t const *lBlockPtr = QS::getBlock(&lFIFOLen);
        QF_INT_ENABLE();

        // Any bytes in the block?
        while (lFIFOLen-- != 0U) {
            // Put into the FIFO.
            MAP_UARTCharPut(UART0_BASE, *lBlockPtr++);
        }
    }

#elif defined(USE_RTT)
    // TX done?
    unsigned int lAvailWriteSpace = SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex);
    if (lAvailWriteSpace > 0) {
        // Max bytes we can accept.
        uint16_t lBlockLen = lAvailWriteSpace;

        QF_INT_DISABLE();
        // Get pointer to next block to transmit.
        uint8_t const *lBlockPtr = QP::QS::getBlock(&lBlockLen);
        QF_INT_ENABLE();

        while (lBlockLen != 0U) {
            //SEGGER_RTT_SetTerminal(sRTTQSPYTerminal);
            unsigned int lWrittenLen = SEGGER_RTT_Write(sRTTBufferIndex, lBlockPtr, lBlockLen);
            //SEGGER_RTT_SetTerminal(0);
            if (lBlockLen <= lWrittenLen) {
                lBlockLen -= lWrittenLen;
                lBlockPtr += lWrittenLen;
            } else {
                lBlockLen = 0;
            }
        }
    }
#endif
}

#endif // Q_SPY


extern "C" {

// LwIP
u32_t sys_now() {
    // Returns the current time in milliseconds.
    return SysTickValueGet() * BSP::MS_PER_TICK;
}


void sntp_set_system_time(time_t aTime) {
    sSystemTime = aTime;

    // Send event with new system time from SNTP.
    LwIP::Event::SystemTimeUpdate * const lEvent = Q_NEW(
        LwIP::Event::SystemTimeUpdate,
        LWIP_SYSTEM_TIME_UPDATE_SIG,
        aTime
    );
#ifdef Q_SPY
    static QP::QSpyId const sSNTPSetSystemTime = {0U};
#endif // Q_SPY
    QP::QF::PUBLISH(lEvent, &sSNTPSetSystemTime);
}


void SysTick_Handler(void);
void SysTick_Handler(void) {

#ifdef Q_SPY
    {
        // Clear SysTick_CTRL_COUNTFLAG.
        // Account for the clock rollover.
        //uint32_t volatile lTmp = SysTick->CTRL;
        //static_cast<void>(lTmp);
        QS_tickTime_ += QS_tickPeriod_;
    }
#endif // Q_SPY

    // Call QF Tick function.
    QP::QF::TICK_X(0U, &BSP::Factory::sSysTick_Handler);

    // Uncomment those lines if need to publish every single tick.
    // Process time events for rate 0.
    // Publish to subscribers.
    //static QP::QEvt const sTickEvent(TIME_TICK_SIG);
    //QP::QF::PUBLISH(&sTickEvent, &sSysTick_Handler);
}


// GPIO port P interrupt handler.
void GPIOPortP3_IRQHandler(void);
void GPIOPortP3_IRQHandler(void) {

    // Get the state of the GPIO and issue the corresponding event.
    static constexpr bool sIsMasked = true;
    constexpr GPIO lRTCCInterruptPin = BSP::Factory::GetRTCCInterruptPin();
    unsigned long lIntStatus = MAP_GPIOIntStatus(lRTCCInterruptPin.GetPort(), sIsMasked);
    constexpr unsigned long lPort = lRTCCInterruptPin.GetPort();
    constexpr unsigned int lPin = lRTCCInterruptPin.GetPin();
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(lPort, lPin);

        // Signal to AO that RTCC generated an interrupt.
        // This can be done with direct POST to known RTCC AO, but since there's a single instance,
        // broadcasting the event/signal is just fine too.
        static QP::QEvt const sRTCCAlarmIntEvent(RTCC_INTERRUPT_SIG);
        //QP::QF::PUBLISH(&sRTCCAlarmIntEvent, 0);
        BSP::Factory::Instance()->GetOpaqueRTCCAO()->POST(&sRTCCAlarmIntEvent, 0);
    }
}


// GPIO port B interrupt handler.
void GPIOPortB_IRQHandler(void);
void GPIOPortB_IRQHandler(void) {
#if 0
    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = MAP_GPIOIntStatus(GPIOB_AHB_BASE, lIsMasked);
    unsigned int lPin = BSP::Factory::GetBLEInterruptPin().GetPin();
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(GPIOB_AHB_BASE, lPin);

        // Signal to AO that BLE generated an interrupt.
        static QP::QEvt const sBLEIntEvent(BLE_INTERRUPT_SIG);
        //QP::QF::PUBLISH(&sBLEIntEvent, 0);
        BSP::Factory::Instance()->GetOpaqueBLEAO()->POST(&sBLEIntEvent, 0);
    }
#endif
}


// GPIO port J interrupt handler.
void GPIOPortJ_IRQHandler(void);
void GPIOPortJ_IRQHandler(void) {
    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked = true;
    unsigned long lIntStatus = MAP_GPIOIntStatus(GPIOJ_AHB_BASE, lIsMasked);

    unsigned int lPin = BSP::mManualFeedButton.GetPin();
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(GPIOJ_AHB_BASE, lPin);
        std::shared_ptr<QP::QActive> lPFPPAO = BSP::Factory::Instance()->GetOpaquePFPPAO();
        if (nullptr != lPFPPAO) {
            if (Button::IS_HIGH == BSP::mManualFeedButton.GetGPIOPinState()) {
                static PFPP::Event::Mgr::ManualFeedCmd const sOnEvent(FEED_MGR_MANUAL_FEED_CMD_SIG, true);
                //QP::QF::PUBLISH(&sOnEvent, 0);
                lPFPPAO->POST(&sOnEvent, 0);
            } else {
                static PFPP::Event::Mgr::ManualFeedCmd const sOffEvent(FEED_MGR_MANUAL_FEED_CMD_SIG, false);
                //QP::QF::PUBLISH(&sOffEvent, 0);
                lPFPPAO->POST(&sOffEvent, 0);
            }
        }
    }

    lPin = BSP::mTimedFeedButton.GetPin();
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(GPIOJ_AHB_BASE, lPin);
        // Only interested in the pin coming high.
        std::shared_ptr<QP::QActive> lPFPPAO = BSP::Factory::Instance()->GetOpaquePFPPAO();
        if (nullptr != lPFPPAO) {
            if (Button::IS_HIGH == BSP::mTimedFeedButton.GetGPIOPinState()) {
                static PFPP::Event::Mgr::TimedFeedCmd const sEvent(FEED_MGR_TIMED_FEED_CMD_SIG, 0);
                //QP::QF::PUBLISH(&sEvent, 0);
                lPFPPAO->POST(&sEvent, 0);
            }
        }
    }
}


// GPIO port D interrupt handler.
void GPIOPortD_IRQHandler(void);
void GPIOPortD_IRQHandler(void) {

}


#if defined(Q_SPY) && defined (USE_UART0)
void UART0_IRQHandler(void);
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled.
// Such ISRs don't need to call QK_ISR_ENTRY/QK_ISR_EXIT
// and they cannot post or publish events.
//
void UART0_IRQHandler(void) {
    // Get the raw interrupt status.
    // Clear the asserted interrupts.
    unsigned long lStatus = MAP_UARTIntStatus(UART0_BASE, true);
    MAP_UARTIntStatus(UART0_BASE, lStatus);

    // While RX FIFO NOT empty.
    while (MAP_UARTCharsAvail(UART0_BASE)) {
        unsigned long lLongByte = MAP_UARTCharGet(UART0_BASE);
        uint8_t lByte = static_cast<uint8_t>(lLongByte);
        QP::QS::rxPut(lByte);
    }
}
#else // Q_SPY
// Intentional undefined function body.
#endif // Q_SPY


// TODO: make this conditional to Ethernet support.
void EMAC0_IRQHandler(void);
void EMAC0_IRQHandler(void) {
    LwIPDrv::StaticISR(0);
}


} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
