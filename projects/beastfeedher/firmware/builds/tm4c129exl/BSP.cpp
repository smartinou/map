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
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <array>

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

// FatFS.
#include "ff.h"

#include "inc/Button.h"
#include "inc/GPIO.h"
#include "inc/PortPin.h"

#include "FatFSDisk.h"
#include "SDC.h"
#include "SPIMasterDev.h"
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

static constexpr uint32_t sClkRate{120000000U};
static constexpr unsigned long sUartPortNbr{0UL};
static constexpr uint32_t sUartBaudRate{115200U};

enum RTTTerminal {
    _stdin = 0,
    _stdout = 1,
    _stderr = 2,
    _qspy = 3,
};
static constexpr unsigned int sRTTBufferIndex{0U};
static constexpr size_t sRTTUpBufferSize{1024};
static constexpr unsigned int sRTTQSPYTerminal{RTTTerminal::_qspy};

#ifdef Q_SPY

static constexpr size_t sUARTTxFIFODepth{16U};

#endif // Q_SPY

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

namespace BSP {


class Factory final
    : public IBSPFactory {

public:
    explicit Factory(uint32_t const aClkRate = sClkRate)
        : mClkRate{aClkRate}
    {
        // Ctor.
        Init();

        // SPIMasterDev is required with several devices.
        CreateSPIDev(2);
        CreateSPIDev(3);
        // TODO: CONSIDER CREATING RTCC HERE TOO.
    }


    // IBSPFactory Interface.
    auto StartRTCCAO(
        std::shared_ptr<CalendarRec> aCalendarRec,
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) -> std::shared_ptr<RTCC::AO::RTCC_AO> final {
        if (!mRTCCAO) {
            mRTCC = CreateRTCC();
            // RTCC also implements both ITemperature and INVMem interfaces.
            mRTCCAO = std::make_shared<RTCC::AO::RTCC_AO>(
                mRTCC,
                mRTCC,
                mRTCC,
                std::move(aCalendarRec)
            );
            if (mRTCCAO) {
                mRTCCAO->start(aPrio, aQSto, aQLen, nullptr, 0U);
            }
        }
        return mRTCCAO;
    }


    auto CreateDisks() -> unsigned int {
        // This BSP has one or two SDC devices.
        {
            // Drive index 0 is the default drive.
            static constexpr struct PortPin sCSnPin{GPIOD_AHB_BASE, GPIO_PIN_4};
            static constexpr struct PortPin sDetectPin{GPIOF_AHB_BASE, GPIO_PIN_7};
            SDC::Create<SDC>(
                sDefaultDiskIndex,
                mSPI3Dev,
                GPIO{sCSnPin.mPort, sCSnPin.mPin},
                GPIO{sDetectPin.mPort, sDetectPin.mPin}
            );
        }

        {
            // Sharp 128x128 memory LCD & microSD card BoosterPack.
            // Card detect line requires 0Ohm resistor.
            static constexpr struct PortPin sCSnPin{GPIOC_AHB_BASE, GPIO_PIN_7};
            static constexpr struct PortPin sDetectPin{GPIOP_BASE, GPIO_PIN_2};
            SDC::Create<SDC>(
                sBoosterPackDiskIndex,
                mSPI2Dev,
                GPIO{sCSnPin.mPort, sCSnPin.mPin},
                GPIO{sDetectPin.mPort, sDetectPin.mPin}
            );
        }

        return FatFSDisk::GetDiskQty();
    }


    [[maybe_unused]] auto MountFS() -> bool final {
        // Create SDC instance to use in FS stubs.
        // WARNING: if SD card disks exist, they should be mounted before calling
        // any other device triggering SPI bus activity.
        // This can be handled in different order when SDCard socket is
        // powered via PIO, but it's not guaranteed the case on all targets.
        // See http://elm-chan.org/docs/mmc/mmc_e.html
        // Section: Consideration on Multi-slave Configuration
        unsigned int const lDiskQty = CreateDisks();
        if (0 != lDiskQty) {
            // Disks found: mount the default drive.
            if (FRESULT const lResult =
                FatFSDisk::StaticMountDisk(
                    sDefaultDiskIndex,
                    &mFatFS
                ); FR_OK == lResult)
            {
                return true;
            }
        }

        // No disk found, or failed to mount default disk.
        return false;
    }


    [[maybe_unused]] auto StartFileSinkAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) -> bool final {
        // FS mounted on default disk: add log sink.
        if (!mFileLogSinkAO) {
            mFileLogSinkAO = std::make_unique<Logging::AO::FileSink_AO>(10 * 60 * BSP::TICKS_PER_SEC);
            if (mFileLogSinkAO) {
                reinterpret_cast<Logging::AO::FileSink_AO *>(
                    mFileLogSinkAO.get())->SetSyncLogLevel(LogLevel::prio::INFO);

                std::unordered_set<std::string> const lCategories{"PFPP"};
                Logging::Event::Init const lFileLogInitEvent(DUMMY_SIG, lCategories);
                mFileLogSinkAO->start(
                    aPrio,
                    aQSto,
                    aQLen,
                    nullptr,
                    0U,
                    &lFileLogInitEvent
                );
                return true;
            }
        }
        return false;
    }


    [[maybe_unused]] auto StartPFPPAO(
        std::shared_ptr<FeedCfgRec> aFeedCfgRec,
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) -> bool final {
        if (!mPFPPAO) {
            // Create motor controller and pass ownership to Mgr_AO.
            auto lMotorControl = CreateMotorControl();
            auto lLogger = std::make_unique<Logger>();
            mPFPPAO = std::make_unique<PFPP::AO::Mgr_AO>(
                std::move(lMotorControl),
                std::move(aFeedCfgRec),
                std::move(lLogger)
            );
            if (mPFPPAO) {
                mPFPPAO->start(aPrio, aQSto, aQLen, nullptr, 0U);
                return true;
            }
        }
        return false;
    }


    [[maybe_unused]] auto StartDisplayMgrAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) -> bool final {
        if (!mDisplayMgrAO) {
            // TODO: CREATE AN ACTUAL OBJECT WHEN AVAILABLE.
            mDisplayMgrAO = std::unique_ptr<Display::AO::Mgr_AO>(nullptr);
            if (mDisplayMgrAO != nullptr) {
                mDisplayMgrAO->start(aPrio, aQSto, aQLen, nullptr, 0U);
                return true;
            }
        }
        return false;
    }


    [[maybe_unused]] auto StartLwIPMgrAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen,
        QP::QEvt const * const aInitEvt
    ) -> bool final {
        if (!mLwIPMgrAO) {
            // Create all Ethernet drivers required before the AO.
            // LwIP::AO::Mgr doesn't use any local references to LwIPDrv.
            // They are referenced via LwIPDrv static functions.
            CreateEthDrv();
            mLwIPMgrAO = std::make_unique<LwIP::AO::Mgr_AO>();
            if (mLwIPMgrAO) {
                mLwIPMgrAO->start(aPrio, aQSto, aQLen, nullptr, 0U, aInitEvt);
                return true;
            }
        }
        return false;
    }


    auto CreateBLEAO() -> std::shared_ptr<QP::QActive> final {
        return nullptr;//std::make_shared<QP::QActive>(nullptr);//std::shared_ptr<PFPP::AO::BLE_AO>(nullptr);
    }


    // Local interface.
    static constexpr auto GetRTCCInterruptPin() { return mRTCCInterruptPin; }
    //static GPIO const &GetBLEInterruptPin() { return mBLEInterruptPin; }

    static uint8_t const sSysTick_Handler;
    static uint8_t const sGPIOPortP3_IRQHandler;

private:
    void Init() {
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
        static constexpr struct PortPin lU0RxGPIO{GPIOA_AHB_BASE, GPIO_PIN_0};
        static constexpr struct PortPin lU0TxGPIO{GPIOA_AHB_BASE, GPIO_PIN_1};
        MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
        MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
        MAP_GPIOPinTypeUART(
            lU0RxGPIO.mPort,
            lU0RxGPIO.mPin | lU0TxGPIO.mPin
        );
        // Enable UART0:
        // @115200, 8-N-1.
        // Flush the buffers.
        UARTStdioInit(sUartPortNbr, mClkRate, sUartBaudRate);
#endif // USE_UART0

#ifdef USE_RTT
    // Leave here until RTT has other use than QSPY.
    SEGGER_RTT_Init();
    static constexpr auto sRTTUpBufferName{"RTTUpBuffer"};
    static std::array<uint8_t, sRTTUpBufferSize> sRTTUpBuffer{0};
    int lResult = SEGGER_RTT_ConfigUpBuffer(
        sRTTBufferIndex,
        sRTTUpBufferName,
        sRTTUpBuffer.data(),
        sRTTUpBuffer.size(),
        SEGGER_RTT_MODE_NO_BLOCK_SKIP
    );

    if (lResult < 0) {
        // Error while configuring buffer.
        return;
    }

    static constexpr auto sRTTDownBufferName{"RTTDownBuffer"};
    static constexpr size_t sRTTDownBufferSize{64};
    static std::array<uint8_t, sRTTDownBufferSize> sRTTDownBuffer{0};
    lResult = SEGGER_RTT_ConfigDownBuffer(
        sRTTBufferIndex,
        sRTTDownBufferName,
        sRTTDownBuffer.data(),
        sRTTDownBuffer.size(),
        SEGGER_RTT_MODE_NO_BLOCK_SKIP
    );

    if (lResult < 0) {
        // Error while configuring buffer.
        return;
    }

    //ITM_SendChar("Hello, World!\n");
    // TODO: HAS TO BE ENABLED FIRST.
    //ITM_SendChar('M');
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


    void CreateSPIDev(unsigned int aSSIID) {
        switch (aSSIID) {
        case 2: {
            // Create pin configuration.
            // Initialize SPI Master.
            SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
            static constexpr CoreLink::SPIMasterDev::SSIGPIO lSSIGPIO {
                GPIO_PD3_SSI2CLK,
                GPIO_PD1_SSI2XDAT0,
                GPIO_PD0_SSI2XDAT1,
                GPIOD_AHB_BASE,
                GPIO_PIN_3, // Clk.
                GPIO_PIN_0, // Rx.
                GPIO_PIN_1  // Tx.
            };
            mSPI2Dev = std::make_shared<CoreLink::SPIMasterDev>(
                SSI2_BASE,
                mClkRate,
                lSSIGPIO
            );
        } break;
        case 3: {
            // Create pin configuration.
            // Initialize SPI Master.
            SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
            static constexpr CoreLink::SPIMasterDev::SSIGPIO lSSIGPIO {
                GPIO_PQ0_SSI3CLK,
                GPIO_PQ2_SSI3XDAT0,
                GPIO_PQ3_SSI3XDAT1,
                GPIOQ_BASE,
                GPIO_PIN_0, // Clk.
                GPIO_PIN_3, // Rx.
                GPIO_PIN_2  // Tx.
            };
            mSPI3Dev = std::make_shared<CoreLink::SPIMasterDev>(
                SSI3_BASE,
                mClkRate,
                lSSIGPIO
            );
        } break;
        case 0: [[fallthrough]];
        case 1: [[fallthrough]];
        default:
            return;
        }
    }


    auto CreateRTCC() -> std::shared_ptr<DS3234> {
        // Creates a DS3234 RTCC.
        // Reset input pin.
        [[maybe_unused]] static constexpr struct PortPin sResetPin{
            GPIOK_BASE,
            GPIO_PIN_7
        };
        // Calls the Ctor that uses default SPI slave configuration,
        // with specified CSn pin.
        static constexpr unsigned long sInterruptNumber{INT_GPIOP3};
        static constexpr struct PortPin sCSnPin{GPIOQ_BASE, GPIO_PIN_1};
        static constexpr auto sBaseYear = 2000;
        auto lRTCC = std::make_shared<DS3234>(
            sBaseYear,
            sInterruptNumber,
            GPIO{mRTCCInterruptPin.mPort, mRTCCInterruptPin.mPin},
            mSPI3Dev,
            GPIO{sCSnPin.mPort, sCSnPin.mPin}
        );

        return lRTCC;
    }


    auto CreateMotorControl() -> std::unique_ptr<IMotorControl> {
        static constexpr struct PortPin lIn1{GPIOB_AHB_BASE, GPIO_PIN_6};
        static constexpr struct PortPin lIn2{GPIOB_AHB_BASE, GPIO_PIN_5};
        static constexpr struct PortPin lPWM{GPIOB_AHB_BASE, GPIO_PIN_0};

        // In fact, creates *half* a TB6612.
        return std::make_unique<TB6612>(
            GPIO{lIn1.mPort, lIn1.mPin},
            GPIO{lIn2.mPort, lIn2.mPin},
            GPIO{lPWM.mPort, lPWM.mPin}
        );
    }


    void CreateEthDrv() {
        static constexpr unsigned int sMyNetIFIndex{0};
        auto const lMAC = GetMACAddress();
        static constexpr unsigned int sPBufQueueSize{8};
        mEthDrv = std::make_unique<EthDrv>(
            sMyNetIFIndex,
            lMAC,
            sPBufQueueSize,
            mClkRate
        );
    }


#if 0
    std::unique_ptr<BLE::BLE> CreateBLE() {
        unsigned long lInterruptNumber = INT_GPIOB;
        static constexpr GPIO lCSnPin(GPIOB_AHB_BASE, GPIO_PIN_4);
        auto lBLE = std::make_unique<BLE::BLE>(
            lInterruptNumber,
            mBLEInterruptPin,
            *mSPI3Dev,
            lCSnPin
        );

        return lBLE;
    }
#endif


    auto GetMACAddress() -> EthernetAddress {
        // For the Stellaris Eval Kits, the MAC address will be stored in the
        // non-volatile USER0 and USER1 registers. These registers can be read
        // using the FlashUserGet function, as illustrated below.
        unsigned long lUser0{0};
        unsigned long lUser1{0};
        FlashUserGet(&lUser0, &lUser1);

        // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC address.
        EthernetAddress const lMAC{
            static_cast<uint8_t>((lUser0 & 0x000000FFL) >>  0),
            static_cast<uint8_t>((lUser0 & 0x0000FF00L) >>  8),
            static_cast<uint8_t>((lUser0 & 0x00FF0000L) >> 16),
            static_cast<uint8_t>((lUser1 & 0x000000FFL) >>  0),
            static_cast<uint8_t>((lUser1 & 0x0000FF00L) >>  8),
            static_cast<uint8_t>((lUser1 & 0x00FF0000L) >> 16)
        };

        if (lMAC.IsValid()) {
            return lMAC;
        }

        static constexpr EthernetAddress sDefaultMAC{
            0x00, 0x50, 0x1d, 0xc2, 0x70, 0xff
        };
        return sDefaultMAC;
    }

    uint32_t mClkRate{};
    std::shared_ptr<CoreLink::SPIMasterDev> mSPI2Dev{};
    std::shared_ptr<CoreLink::SPIMasterDev> mSPI3Dev{};
    std::shared_ptr<DS3234> mRTCC{};
    std::shared_ptr<RTCC::AO::RTCC_AO> mRTCCAO{};
    std::unique_ptr<Logging::AO::FileSink_AO> mFileLogSinkAO{};
    std::unique_ptr<PFPP::AO::Mgr_AO> mPFPPAO{};
    std::unique_ptr<Display::AO::Mgr_AO> mDisplayMgrAO{};
    std::unique_ptr<EthDrv> mEthDrv{};
    std::unique_ptr<LwIP::AO::Mgr_AO> mLwIPMgrAO{};
    //std::unique_ptr<BLE::BLE> mBLE{};
    //std::shared_ptr<PFPP::AO::BLE_AO> mBLEAO{};

    static constexpr auto sDefaultDiskIndex{0};
    static constexpr auto sBoosterPackDiskIndex{1};

    static constexpr struct PortPin mRTCCInterruptPin{GPIOP_BASE, GPIO_PIN_3};
    //static constexpr GPIO mBLEInterruptPin{GPIOB_AHB_BASE, GPIO_PIN_1};

    FATFS mFatFS{};
};


} // namespace BSP

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

namespace BSP {
static void InitEtherLED();
static void InitUserLED(struct PortPin const &aUserLEDPortPin);
static void SetUserLED(struct PortPin const &aUserLEDPortPin, bool aState);
} // namespace BSP

#ifdef Q_SPY
static void TxData();
#endif // Q_SPY

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

static time_t sSystemTime{0};

#ifdef Q_SPY
// For local extern "C" functions, not part of any namespace.
static QP::QSTimeCtr QS_tickTime_{0};
static QP::QSTimeCtr const QS_tickPeriod_{SystemCoreClock / BSP::TICKS_PER_SEC};

namespace BSP {
// event-source identifiers used for tracing
uint8_t constexpr Factory::sSysTick_Handler{0U};
uint8_t constexpr Factory::sGPIOPortP3_IRQHandler{0U};
}

#endif // Q_SPY


// Button class should become GPIO class.
//GPIO const BSP::Factory::mBLEInterruptPin(GPIOB_AHB_BASE, GPIO_PIN_1);


namespace BSP {

// Those variables are used locally in various stubs and IRQ handlers.
static constexpr PortPin sUserLED1PortPin{GPION_BASE, GPIO_PIN_1};
static constexpr PortPin sUserLED2PortPin{GPION_BASE, GPIO_PIN_0};
} // namespace BSP

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace BSP {

std::unique_ptr<IBSPFactory> Create() {
    return std::make_unique<Factory>();
}


}  // namespace BSP

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

namespace BSP {


static void InitEtherLED() {

    // GPIO for Ethernet LEDs.
    static constexpr PortPin lLinkLEDPortPin{GPIOF_AHB_BASE, GPIO_PIN_4};
    InitUserLED(lLinkLEDPortPin);
    MAP_GPIOPinConfigure(GPIO_PF4_EN0LED1);
    MAP_GPIOPinTypeEthernetLED(lLinkLEDPortPin.mPort, lLinkLEDPortPin.mPin);

    static constexpr PortPin lActivityLEDPortPin{GPIOF_AHB_BASE, GPIO_PIN_0};
    InitUserLED(lActivityLEDPortPin);
    MAP_GPIOPinConfigure(GPIO_PF0_EN0LED0);
    MAP_GPIOPinTypeEthernetLED(lActivityLEDPortPin.mPort, lActivityLEDPortPin.mPin);
}


static void InitUserLED(struct PortPin const &aUserLEDPortPin) {
    // GPIO for user LED toggling during idle.
    MAP_GPIOPinTypeGPIOOutput(aUserLEDPortPin.mPort, aUserLEDPortPin.mPin);
    MAP_GPIOPadConfigSet(
        aUserLEDPortPin.mPort,
        aUserLEDPortPin.mPin,
        GPIO_STRENGTH_2MA,
        GPIO_PIN_TYPE_STD
    );
}


static void SetUserLED(struct PortPin const &aUserLEDPortPin, bool const aState) {

    MAP_GPIOPinWrite(
        aUserLEDPortPin.mPort,
        aUserLEDPortPin.mPin,
        aState ? aUserLEDPortPin.mPin : 0
    );
}


} // namespace BSP


// QF callbacks ==============================================================
void QP::QF::onStartup() {

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
    BSP::InitUserLED(BSP::sUserLED1PortPin);
    BSP::InitUserLED(BSP::sUserLED2PortPin);
    BSP::InitEtherLED();

    MAP_IntEnable(INT_EMAC0);

#if defined(Q_SPY) && defined(USE_UART0)
    // UART0 interrupt used for QS-RX.
    NVIC_SetPriority(UART0_IRQn, UART0_PRIO);
    NVIC_EnableIRQ(UART0_IRQn);
#endif // Q_SPY && USE_UART0
}


//............................................................................
// called with interrupts disabled, see NOTE01
void QP::QV::onIdle() {

    // Toggle LED for visual effect.
    BSP::SetUserLED(BSP::sUserLED1PortPin, true);
    BSP::SetUserLED(BSP::sUserLED1PortPin, false);

#ifdef Q_SPY
    QF_INT_ENABLE();

    // Parse all the received bytes.
#ifdef USE_RTT
    // No interrupts: poll for received bytes and move to QS rx buffer.
    if (SEGGER_RTT_HasData(sRTTBufferIndex) != 0) {
        uint8_t lByte{0};
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
extern "C" void Q_onAssert(char const * const module, int const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    static_cast<void>(module);
    static_cast<void>(loc);
    static constexpr auto sDelay{10000U};
    QS_ASSERTION(module, loc, sDelay);
    NVIC_SystemReset();

    // noreturn: loop forever.
    while (true) {}
}


#ifdef QF_ACTIVE_STOP
void QP::QActive::stop() {
    // Unsubscribe from all events.
    // Remove this object from QF.
    QP::QActive::unsubscribeAll();
    QP::QF::remove_(this);
}
#endif // QF_ACTIVE_STOP

// QS callbacks ==============================================================
#ifdef Q_SPY

//............................................................................
bool QP::QS::onStartup(void const * const aArgs) {

    static_cast<void>(aArgs);

    // Buffer for Quantum Spy.
    static std::array<uint8_t, 2 * 1024> sQSTxBuf{};
    initBuf(sQSTxBuf.data(), sQSTxBuf.size());

    // Buffer for QS receive channel.
    static std::array<uint8_t, 100> sQSRxBuf{};
    rxInitBuf(sQSRxBuf.data(), sQSRxBuf.size());

    // To start timestamp at zero.
    uint32_t volatile lTmp{SysTick->CTRL};
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
void QP::QS::onCleanup() {
    // Used for QUTest only.
}


//............................................................................
// NOTE: invoked with interrupts DISABLED.
QP::QSTimeCtr QP::QS::onGetTime() {

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
void QP::QS::onFlush() {
#ifdef USE_UART0

    // Tx FIFO depth.
    uint16_t lFIFOLen{sUARTTxFIFODepth};
    uint8_t const *lBlockPtr{nullptr};

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
        lFIFOLen = sUARTTxFIFODepth;
        QF_INT_DISABLE();
    }
#elif defined(USE_RTT)
    // Tx FIFO depth.
    uint16_t lFIFOLen = SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex);
    uint8_t const *lBlockPtr{nullptr};

    QF_INT_DISABLE();
    while ((lBlockPtr = QS::getBlock(&lFIFOLen)) != nullptr) {
        QF_INT_ENABLE();
        // Busy-wait until TX FIFO empty.
        //while (SEGGER_RTT_GetAvailWriteSpace(sRTTBufferIndex) != (sRTTUpBufferSize - 1)) {
        //}

        //SEGGER_RTT_SetTerminal(sRTTQSPYTerminal);
        unsigned int const lLen = SEGGER_RTT_Write(sRTTBufferIndex, lBlockPtr, lFIFOLen);
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
void QP::QS::onReset() {
    //NVIC_SystemReset();
}


//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
void QP::QS::onCommand(
    uint8_t const aCmdId,
    uint32_t const aParam1,
    uint32_t const aParam2,
    uint32_t const aParam3
) {
    static_cast<void>(aCmdId);
    static_cast<void>(aParam1);
    static_cast<void>(aParam2);
    static_cast<void>(aParam3);

    //TBD
}


static void TxData() {
#ifdef USE_UART0
    // TX done?
    if (MAP_UARTSpaceAvail(UART0_BASE)) {
        // Max bytes we can accept.
        uint16_t lFIFOLen{sUARTTxFIFODepth};

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


void sntp_set_system_time(time_t const aTime) {
    sSystemTime = aTime;

    // Send event with new system time from SNTP.
    LwIP::Event::SystemTimeUpdate * const lEvent = Q_NEW(
        LwIP::Event::SystemTimeUpdate,
        LWIP_SYSTEM_TIME_UPDATE_SIG,
        aTime
    );
#ifdef Q_SPY
    static QP::QSpyId const sSNTPSetSystemTime{0U};
#endif // Q_SPY
    QP::QF::PUBLISH(lEvent, &sSNTPSetSystemTime);
}


static void DebounceSwitches() {

    // [MG] LOOKS LIKE THIS IS USED ONLY TO SETUP THE GPIO PIN STATE.
    static constexpr struct Button_s sManualFeedButton{
        GPIOJ_AHB_BASE,
        GPIO_PIN_0,
        INT_GPIOJ,
        0
    };
    static constexpr struct Button_s sTimedFeedButton{
        GPIOJ_AHB_BASE,
        GPIO_PIN_1,
        INT_GPIOJ,
        0
    };

    static constexpr std::size_t sStateDepth{5};
    static std::array<int32_t, sStateDepth> sPinsState{0};
    static int32_t sPreviousDebounce{0};
    static std::size_t lStateIx{0};
    sPinsState.at(lStateIx) = ~MAP_GPIOPinRead(
        GPIOJ_AHB_BASE,
        sManualFeedButton.mPin | sTimedFeedButton.mPin);
    lStateIx++;
    if (lStateIx >= sStateDepth) {
        lStateIx = 0;
    }

    // Bitwise-AND all last current pin states.
    int32_t lCurrentDebounce{sManualFeedButton.mPin | sTimedFeedButton.mPin};
    for (auto const lPinState : sPinsState) {
        lCurrentDebounce &= lPinState;
    }

#ifdef Q_SPY
    static QP::QSpyId const sSysTick_Handler{0U};
#endif // Q_SPY
    // What changed now? Look for pressed states.
    if ((!sPreviousDebounce) & lCurrentDebounce) {
        if (lCurrentDebounce & sManualFeedButton.mPin) {
            BSP::SetUserLED(BSP::sUserLED2PortPin, true);
            static PFPP::Event::Mgr::ManualFeedCmd constexpr sOnEvent(
                FEED_MGR_MANUAL_FEED_CMD_SIG,
                true,
                QP::QEvt::StaticEvt::STATIC_EVT
            );
            QP::QF::PUBLISH(&sOnEvent, &sSysTick_Handler);
        }
        if (lCurrentDebounce & sTimedFeedButton.mPin) {
            static PFPP::Event::Mgr::TimedFeedCmd constexpr sOnEvent(
                FEED_MGR_TIMED_FEED_CMD_SIG,
                true,
                QP::QEvt::StaticEvt::STATIC_EVT
            );
            QP::QF::PUBLISH(&sOnEvent, &sSysTick_Handler);
        }
    }

    // Look for released states.
    if (sPreviousDebounce & !lCurrentDebounce) {
        if ((!lCurrentDebounce) & sManualFeedButton.mPin) {
            BSP::SetUserLED(BSP::sUserLED2PortPin, false);
            static PFPP::Event::Mgr::ManualFeedCmd constexpr sOffEvent(
                FEED_MGR_MANUAL_FEED_CMD_SIG,
                false,
                QP::QEvt::StaticEvt::STATIC_EVT
            );
            QP::QF::PUBLISH(&sOffEvent, &sSysTick_Handler);
        }
    }

    sPreviousDebounce = lCurrentDebounce;
}


void SysTick_Handler();
void SysTick_Handler() {

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
#if 0
    // Process time events for rate 0.
    // Publish to subscribers.
    static constexpr QP::QEvt sTickEvent{
        TIME_TICK_SIG,
        QP::QEvt::StaticEvt::STATIC_EVT
    };
    QP::QF::PUBLISH(&sTickEvent, &sSysTick_Handler);
#endif
    DebounceSwitches();
}


// GPIO port P interrupt handler.
void GPIOPortP3_IRQHandler();
void GPIOPortP3_IRQHandler() {

    // Get the state of the GPIO and issue the corresponding event.
    static constexpr bool sIsMasked{true};
    static constexpr struct PortPin lRTCCInterruptPin = BSP::Factory::GetRTCCInterruptPin();
    auto const lIntStatus = MAP_GPIOIntStatus(lRTCCInterruptPin.mPort, sIsMasked);
    constexpr auto lPin = lRTCCInterruptPin.mPin;
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(lRTCCInterruptPin.mPort, lPin);

        // Signal to AO that RTCC generated an interrupt.
        // This can be done with direct POST to known RTCC AO, but since there's a single instance,
        // broadcasting the event/signal is just fine too.
        static QP::QEvt constexpr sRTCCAlarmIntEvent{
            RTCC_INTERRUPT_SIG,
            QP::QEvt::StaticEvt::STATIC_EVT
        };
#ifdef Q_SPY
        static QP::QSpyId const sGPIOPortP3_IRQHandler{0U};
#endif // Q_SPY
        QP::QF::PUBLISH(&sRTCCAlarmIntEvent, &sGPIOPortP3_IRQHandler);
    }
}


// GPIO port B interrupt handler.
#if 0
void GPIOPortB_IRQHandler();
void GPIOPortB_IRQHandler() {
    // Get the state of the GPIO and issue the corresponding event.
    static const bool lIsMasked{true};
    unsigned long lIntStatus = MAP_GPIOIntStatus(GPIOB_AHB_BASE, lIsMasked);
    unsigned int lPin = BSP::Factory::GetBLEInterruptPin().GetPin();
    if (lPin & lIntStatus) {
        MAP_GPIOIntClear(GPIOB_AHB_BASE, lPin);

        // Signal to AO that BLE generated an interrupt.
        static QP::QEvt const sBLEIntEvent(BLE_INTERRUPT_SIG);
        //QP::QF::PUBLISH(&sBLEIntEvent, 0);
        BSP::Factory::Instance()->GetOpaqueBLEAO()->POST(&sBLEIntEvent, 0);
    }
}
#endif

#if 0
// GPIO port J interrupt handler.
void GPIOPortJ_IRQHandler();
void GPIOPortJ_IRQHandler() {
}


// GPIO port D interrupt handler.
void GPIOPortD_IRQHandler();
void GPIOPortD_IRQHandler() {

}
#endif


#if defined(Q_SPY) && defined (USE_UART0)
void UART0_IRQHandler();
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled.
// Such ISRs don't need to call QK_ISR_ENTRY/QK_ISR_EXIT
// and they cannot post or publish events.
//
void UART0_IRQHandler() {
    // Get the raw interrupt status.
    // Clear the asserted interrupts.
    unsigned long const lStatus = MAP_UARTIntStatus(UART0_BASE, true);
    MAP_UARTIntStatus(UART0_BASE, lStatus);

    // While RX FIFO NOT empty.
    while (MAP_UARTCharsAvail(UART0_BASE)) {
        unsigned long const lLongByte = MAP_UARTCharGet(UART0_BASE);
        uint8_t lByte{static_cast<uint8_t>(lLongByte)};
        QP::QS::rxPut(lByte);
    }
}
#else // Q_SPY
// Intentional undefined function body.
#endif // Q_SPY


// TODO: make this conditional to Ethernet support.
void EMAC0_IRQHandler();
void EMAC0_IRQHandler() {
    LwIPDrv::StaticISR(0);
}


} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
