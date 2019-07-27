// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Application class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

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
#include "hw_ints.h"
//#include "hw_memmap.h" // duplicated defines in lm3s_cmsis.h
#include "hw_types.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"
#include "systick.h"
#include "uart.h"


#include "SDC.h"
#include "SPI.h"
#include "DS3234.h"
#include "GPIOs.h"
#include "Button.h"
#include "SSD1329.h"

#include "BFHMgr_Evt.h"
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


#ifdef Q_SPY

#define UART_BAUD_RATE      115200U
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

    void SetPins(void) const {
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
    Factory()
        : mRTCCCsPin(GPIO_PORTA_BASE, GPIO_PIN_7)
        , mSDCCsPin(GPIO_PORTD_BASE, GPIO_PIN_0)
        , mIn1Pin(GPIO_PORTB_BASE, GPIO_PIN_6)
        , mIn2Pin(GPIO_PORTB_BASE, GPIO_PIN_5)
        , mPWMPin(GPIO_PORTB_BASE, GPIO_PIN_0)
        , mDCnPin(GPIO_PORTC_BASE, GPIO_PIN_7)
        , mEn15VPin(GPIO_PORTC_BASE, GPIO_PIN_6)
        , mOLEDCSnPin(GPIO_PORTA_BASE, GPIO_PIN_3) {

        // Empty Ctor.
    }


    virtual ~Factory() {
        delete [] mSSIPinCfg;
    }

    // IBSPFactory Interface.
    CoreLink::SPIDev * CreateSPIDev() override {
        return CreateSPIDev(0);
    }


    IRTCC * CreateRTCC(CoreLink::SPIDev &aSPIDev) override {
        // This BSP creates a DS3234 RTCC.
        // Calls the Ctor that uses default SPI slave configuration,
        // with specified Cs pin.
        // TODO: CHECK CLEAN CODE BOOK TO FIND ALTERNATIVE TO INDENTATION OF
        // PARAMETERS.
        unsigned long lInterruptNumber = INT_GPIOA;
        IRTCC * const lRTCC = new DS3234(
            2000,
            lInterruptNumber,
            mRTCCInterruptPin,
            aSPIDev,
            mRTCCCsPin
        );

        return lRTCC;
    }


    GPIOs * CreateSDCCsPin(void) override {
        return new GPIOs(GPIO_PORTD_BASE, GPIO_PIN_0);
    }


    SDC * CreateSDC(
        CoreLink::SPIDev &aSPIDev,
        CoreLink::SPISlaveCfg &aSlaveCfg,
        GPIOs const &aCsPin) override {

        aSlaveCfg.SetBitRate(400000);
        aSlaveCfg.SetDataWidth(8);
        aSlaveCfg.SetCSnGPIO(aCsPin.GetPort(), aCsPin.GetPin());

        SDC * const lSDC = new SDC(0, aSPIDev, aSlaveCfg);
        return lSDC;
    }


    ILCD * CreateDisplay(CoreLink::SPIDev &aSPIDev) override {
        static unsigned int const sDisplayWidth = 128;
        static unsigned int const sDisplayHeight = 96;
        ILCD *lLCD = new SSD1329(
            aSPIDev,
            mOLEDCSnPin,
            mDCnPin,
            mEn15VPin,
            sDisplayWidth,
            sDisplayHeight
        );
        return lLCD;
    }

  IFS * CreateFS(CoreLink::SPIDev * const aSPIDev) override {
    return nullptr;
  }

  // Local interface.
  static GPIOs const &GetRTCCInterruptPin(void) { return mRTCCInterruptPin; }

private:
  CoreLink::SPIDev * CreateSPIDev(unsigned int aSSIID) override {

    switch (aSSIID) {
    case 0:
      // Create pin configuration.
      // Initialize SPI Master.
      SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
      mSSIPinCfg = new SSI0PinCfg;
      return new CoreLink::SPIDev(SSI0_BASE, *mSSIPinCfg);

    case 1:
    default:
      return nullptr;
    }
  }

  static GPIOs const mRTCCInterruptPin;

  // All that is RTCC-related.
  GPIOs const mRTCCCsPin;

  CoreLink::SSIPinCfg *mSSIPinCfg = {nullptr};

  // All that is SDC-related.
  GPIOs const mSDCCsPin;

  // All that is BFH Manager-related.
  GPIOs const mIn1Pin;
  GPIOs const mIn2Pin;
  GPIOs const mPWMPin;

  // All that is OLED display-related.
  GPIOs const mDCnPin;
  GPIOs const mEn15VPin;
  GPIOs const mOLEDCSnPin;
};

} // namespace BSP

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

namespace BSP {
static void InitEtherLED(void);
static void InitUserLED(void);
static void SetUserLED(bool aIsSet);
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


// TODO: GPIOs class should really be a Pin class.
// Button class should become GPIO class.
GPIOs const BSP::Factory::mRTCCInterruptPin(GPIO_PORTA_BASE, GPIO_PIN_6);


namespace BSP {

// Those variables are used locally in various stubs and IRQ handlers.
static Button const mManualFeedButton(GPIO_PORTC_BASE, GPIO_PIN_4, INT_GPIOC, 0);
static Button const mTimedFeedButton(GPIO_PORTD_BASE, GPIO_PIN_4, INT_GPIOC, 0);
static Button const mSelectButton(GPIO_PORTF_BASE, GPIO_PIN_1, INT_GPIOF, 0);

static GPIOs const sLinkLEDPin(GPIO_PORTF_BASE, GPIO_PIN_3);
static GPIOs const sActivityLEDPin(GPIO_PORTF_BASE, GPIO_PIN_2);
static GPIOs const sUserLEDPin(GPIO_PORTF_BASE, GPIO_PIN_0);
} // namespace BSP

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

namespace BSP {

IBSPFactory *Init(void) {
  // NOTE: SystemInit() already called from the startup code,
  // where clock already set (CLOCK_SETUP in lm3s_config.h)
  // SystemCoreClockUpdate() also called from there.
  // Settings done for 50MHz system clock.
#if 0
  SysCtlClockSet(SYSCTL_SYSDIV_1
                 | SYSCTL_USE_OSC
                 | SYSCTL_OSC_MAIN
                 | SYSCTL_XTAL_8MHZ);
#endif

  // Enable the clock to the peripherals used by the application.

  // Enable all required GPIOs.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);

  // Debug UART port.
  GPIOs lU0RxGPIO(GPIO_PORTA_BASE, GPIO_PIN_0);
  GPIOs lU0TxGPIO(GPIO_PORTA_BASE, GPIO_PIN_1);
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
     | UART_CONFIG_WLEN_8));
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

  IBSPFactory *lFactory = new Factory;
  return lFactory;
}


}  // namespace BSP

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

namespace BSP {


static void InitEtherLED(void) {

  // GPIO for Ethernet LEDs.
  GPIOPinTypeGPIOOutput(sLinkLEDPin.GetPort(), sLinkLEDPin.GetPin());
  GPIOPadConfigSet(
    sLinkLEDPin.GetPort(),
    sLinkLEDPin.GetPin(),
    GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(sLinkLEDPin.GetPort(), sLinkLEDPin.GetPin());

  GPIOPinTypeGPIOOutput(sActivityLEDPin.GetPort(), sActivityLEDPin.GetPin());
  GPIOPadConfigSet(
    sActivityLEDPin.GetPort(),
    sActivityLEDPin.GetPin(),
    GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(sActivityLEDPin.GetPort(), sActivityLEDPin.GetPin());

  IntEnable(INT_ETH);
}


static void InitUserLED(void) {
  // GPIO for user LED toggling during idle.
  GPIOPinTypeGPIOOutput(sUserLEDPin.GetPort(), sUserLEDPin.GetPin());
  GPIOPadConfigSet(
    sUserLEDPin.GetPort(),
    sUserLEDPin.GetPin(),
    GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD);
  GPIOPinWrite(
    sUserLEDPin.GetPort(),
    sUserLEDPin.GetPin(),
    sUserLEDPin.GetPin());
}


static void SetUserLED(bool aIsSet) {

  if (aIsSet) {
    GPIOPinWrite(
      sUserLEDPin.GetPort(),
      sUserLEDPin.GetPin(),
      sUserLEDPin.GetPin());
  } else {
    GPIOPinWrite(
      sUserLEDPin.GetPort(),
      sUserLEDPin.GetPin(),
      0);
  }
}

} // namespace BSP


// QF callbacks ==============================================================
void QP::QF::onStartup(void) {

  // Set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
  SysTickPeriodSet(SysCtlClockGet() / BSP::TICKS_PER_SEC);
  IntPrioritySet(FAULT_SYSTICK, 0x80); //SYSTICK_PRIO
  SysTickIntEnable();
  SysTickEnable();

  // assing all priority bits for preemption-prio. and none to sub-prio.
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
// called with interrupts disabled, see NOTE01
void QP::QV::onIdle(void) {

  // Toggle LED for visual effect.
  BSP::SetUserLED(true);
  BSP::SetUserLED(false);

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
    // This can be done with direct POST to known RTCC AO,
    // but global publish() offers better decoupling.
    static QP::QEvt const sRTCCAlarmIntEvent(RTCC_INTERRUPT_SIG);
    QP::QF::PUBLISH(&sRTCCAlarmIntEvent, 0);
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

    static BFHManualFeedCmdEvt const sOnEvt(FEED_MGR_MANUAL_FEED_CMD_SIG, true);
    static BFHManualFeedCmdEvt const sOffEvt(FEED_MGR_MANUAL_FEED_CMD_SIG, false);
    // Decouple using framework PUBLISH() method instead of direct posting to AO.
    if (Button::PRESSED == BSP::mManualFeedButton.GetGPIOPinState()) {
      QP::QF::PUBLISH(&sOnEvt, 0);
    } else {
      QP::QF::PUBLISH(&sOffEvt, 0);
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
    if (Button::PRESSED == BSP::mTimedFeedButton.GetGPIOPinState()) {
      static BFHTimedFeedCmdEvt const sEvt(FEED_MGR_TIMED_FEED_CMD_SIG, 0);
      QP::QF::PUBLISH(&sEvt, 0);
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
    if (Button::PRESSED == BSP::mSelectButton.GetGPIOPinState()) {
      static QP::QEvt const sEvt(DISPLAY_REFRESH_SIG);
      //DisplayMgr_AO::AOInstance().POST(&sEvt, 0);
      QP::QF::PUBLISH(&sEvt, 0);
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
