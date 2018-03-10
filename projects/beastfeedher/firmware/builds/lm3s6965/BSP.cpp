// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Board Support Package.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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
#include "hw_types.h"
#include "hw_ints.h"
#include "systick.h"
#include "uartstdio.h"

#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"

// Utilities Library.
#include "Button.h"
#include "GPIOs.h"

// Drivers Library.
#include "DS3234.h"
#include "SSD1329.h"

// Corelink Library.
#include "SPI.h"

// This application.
#include "BFH_Mgr_AO.h"
#include "BFH_Mgr_Evt.h"
#include "BSP.h"
#include "DisplayMgr_AO.h"
#include "RTCC_AO.h"

Q_DEFINE_THIS_FILE

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
// DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
//
enum KernelUnawareISRs { // see NOTE00
    // ...
    MAX_KERNEL_UNAWARE_CMSIS_PRI  // keep always last
};

// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);
#if 1
enum KernelAwareISRs {
    SYSTICK_PRIO = QF_AWARE_ISR_CMSIS_PRI, // see NOTE00
    GPIOPORTA_PRIO,
    GPIOPORTE_PRIO,
    GPIOPORTF_PRIO,
    // ...
    MAX_KERNEL_AWARE_CMSIS_PRI // keep always last
};
// "kernel-aware" interrupts should not overlap the PendSV priority
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));
#endif

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// TODO/FIXME: check if this can be simplified in any way.
class LM3S6965SSIPinCfg : public SSIPinCfg {
 public:
  LM3S6965SSIPinCfg(unsigned int aSPIID) : SSIPinCfg(aSPIID) {}
  ~LM3S6965SSIPinCfg() {}

  void SetPins(void) const;
};


// Helper structure for GPIO port/pin.
struct GPIO {
  unsigned long mPort;
  unsigned int  mPin;
};


struct SSIGPIO {
  unsigned long mPort;
  unsigned int  mClkPin;
  unsigned int  mRxPin;
  unsigned int  mTxPin;
};

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

extern void ISR_Ethernet(void);

static void UserLEDInit(void);
static void EtherLEDInit(void);

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// SPI Dev.
static CoreLink::SPIDev *sSPIDevPtr = nullptr;

// Button objects.
static Button *sManualFeedButtonPtr = nullptr;
static Button *sTimedFeedButtonPtr  = nullptr;
static Button *sSelectButtonPtr     = nullptr;

static GPIOs const * const sSelectGPIOPtr = new GPIOs(GPIO_PORTF_BASE, GPIO_PIN_1);

static GPIOs const * const sManualFeedGPIOPtr = new GPIOs(GPIO_PORTC_BASE, GPIO_PIN_4);
static GPIOs const * const sTimedFeedGPIOPtr  = new GPIOs(GPIO_PORTD_BASE, GPIO_PIN_4);

// SSD1329 GPIOs.
static struct GPIO const sDCnGPIO     = {GPIO_PORTC_BASE, GPIO_PIN_7};
static struct GPIO const sEn15VGPIO   = {GPIO_PORTC_BASE, GPIO_PIN_6};
static struct GPIO const sOLEDCSnGPIO = {GPIO_PORTA_BASE, GPIO_PIN_3};

// LEDs GPIOs.
static struct GPIO const sLinkLEDGPIO     = {GPIO_PORTF_BASE, GPIO_PIN_3};
static struct GPIO const sActivityLEDGPIO = {GPIO_PORTF_BASE, GPIO_PIN_2};
static struct GPIO const sUserLEDGPIO     = {GPIO_PORTF_BASE, GPIO_PIN_0};

// SSI0 GPIOs.
static struct SSIGPIO const sSSIGPIOs = {
  GPIO_PORTA_BASE,
  GPIO_PIN_2, // Clk.
  GPIO_PIN_4, // Rx.
  GPIO_PIN_5  // Tx.
};

// UART0 GPIOs.
static struct GPIO const sU0RxGPIO = {GPIO_PORTA_BASE, GPIO_PIN_0};
static struct GPIO const sU0TxGPIO = {GPIO_PORTA_BASE, GPIO_PIN_1};

// RTCC GPIOs.
GPIOs * const BSP_gRTCCCSnGPIOPtr = new GPIOs(GPIO_PORTA_BASE, GPIO_PIN_7);
GPIOs * const BSP_gRTCCIntGPIOPtr = new GPIOs(GPIO_PORTA_BASE, GPIO_PIN_6);

// Motor controller GPIOs.
GPIOs * const BSP_gIn1GPIOPtr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_6);
GPIOs * const BSP_gIn2GPIOPtr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_5);
GPIOs * const BSP_gPWMGPIOPtr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_0);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void BSP_Init(void) {
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
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinTypeUART(sU0RxGPIO.mPort, sU0RxGPIO.mPin | sU0TxGPIO.mPin);
  UARTStdioInit(0);
  UARTprintf("Hello!\n");
}


CoreLink::SPIDev *BSP_InitSPIDev(void) {
  if (nullptr == sSPIDevPtr) {
    // Initialize SPI Master.
    SSIPinCfg * const lSPIMasterPinCfgPtr = new LM3S6965SSIPinCfg(0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    sSPIDevPtr = new CoreLink::SPIDev(SSI0_BASE, *lSPIMasterPinCfgPtr);
  }

  return sSPIDevPtr;
}


SSD1329 *BSP_InitOLEDDisplay(void) {

  // Initialize the OLED display.
  // Create an SPI slave configuration for the OLED display.
  CoreLink::SPISlaveCfg * const lOLEDSPISlaveCfgPtr = new CoreLink::SPISlaveCfg();

  lOLEDSPISlaveCfgPtr->SetProtocol(CoreLink::SPISlaveCfg::MOTO_2);
  lOLEDSPISlaveCfgPtr->SetBitRate(4000000);
  lOLEDSPISlaveCfgPtr->SetDataWidth(8);
  lOLEDSPISlaveCfgPtr->SetCSnGPIO(sOLEDCSnGPIO.mPort,
                                  sOLEDCSnGPIO.mPin);

  if (nullptr == sSPIDevPtr) {
    BSP_InitSPIDev();
  }
  SSD1329 *lSSD1329Ptr = new SSD1329(*sSPIDevPtr,
                                     *lOLEDSPISlaveCfgPtr,
                                     sDCnGPIO.mPort,
                                     sDCnGPIO.mPin,
                                     sEn15VGPIO.mPort,
                                     sEn15VGPIO.mPin,
                                     128,
                                     96);
  return lSSD1329Ptr;
}


unsigned int BSP_GPIOPortToInt(unsigned long aGPIOPort) {

  switch (aGPIOPort) {
  default:
  case GPIO_PORTA_BASE: return INT_GPIOA;
  case GPIO_PORTB_BASE: return INT_GPIOB;
  case GPIO_PORTC_BASE: return INT_GPIOC;
  case GPIO_PORTD_BASE: return INT_GPIOD;
  case GPIO_PORTE_BASE: return INT_GPIOE;
  case GPIO_PORTF_BASE: return INT_GPIOF;
  case GPIO_PORTG_BASE: return INT_GPIOG;
  case GPIO_PORTH_BASE: return INT_GPIOH;
  }

  return 0;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// QF callbacks ==============================================================
void QP::QF::onStartup(void) {

  // Set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
  SysTickPeriodSet(SysCtlClockGet() / BSP_TICKS_PER_SEC);
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

  // Init user LED.
  // Init Ethernet LEDs.
  UserLEDInit();
  EtherLEDInit();

  // Manual and Timed Feed cap sensor.
  // Those are debounced signals.
  unsigned long lIntNbr = BSP_GPIOPortToInt(sManualFeedGPIOPtr->GetPort());
  sManualFeedButtonPtr = new Button(*sManualFeedGPIOPtr, lIntNbr, 0U);
  lIntNbr = BSP_GPIOPortToInt(sTimedFeedGPIOPtr->GetPort());
  sTimedFeedButtonPtr = new Button(*sTimedFeedGPIOPtr, lIntNbr, 0U);

  // Select button.
  // Not debounced, but simply used to turn display on.
  lIntNbr = BSP_GPIOPortToInt(sSelectGPIOPtr->GetPort());
  sSelectButtonPtr = new Button(*sSelectGPIOPtr, lIntNbr, 0U);

  // Manual Feed cap sensor input.
  // Timed Feed cap sensor input.
  sManualFeedButtonPtr->EnableInt();
  sTimedFeedButtonPtr->EnableInt();

  sSelectButtonPtr->EnableInt();
}

//............................................................................
// called with interrupts disabled, see NOTE01
void QP::QV::onIdle(void) {

  // Toggle the user LED, ON then OFF.
  QF_INT_DISABLE();
  GPIOPinWrite(sUserLEDGPIO.mPort,
               sUserLEDGPIO.mPin,
               sUserLEDGPIO.mPin);
  GPIOPinWrite(sUserLEDGPIO.mPort,
               sUserLEDGPIO.mPin,
               0);
  QF_INT_ENABLE();
}


static void EtherLEDInit(void) {
  // GPIO for Ethernet LEDs.
  GPIOPinTypeGPIOOutput(sLinkLEDGPIO.mPort, sLinkLEDGPIO.mPin);
  GPIOPadConfigSet(sLinkLEDGPIO.mPort,
                   sLinkLEDGPIO.mPin,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(sLinkLEDGPIO.mPort, sLinkLEDGPIO.mPin);

  GPIOPinTypeGPIOOutput(sActivityLEDGPIO.mPort, sActivityLEDGPIO.mPin);
  GPIOPadConfigSet(sActivityLEDGPIO.mPort,
                   sActivityLEDGPIO.mPin,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(sActivityLEDGPIO.mPort, sActivityLEDGPIO.mPin);

  IntEnable(INT_ETH);
}


static void UserLEDInit(void) {
  // GPIO for user LED toggling during idle.
  GPIOPinTypeGPIOOutput(sUserLEDGPIO.mPort, sUserLEDGPIO.mPin);
  GPIOPadConfigSet(sUserLEDGPIO.mPort,
                   sUserLEDGPIO.mPin,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinWrite(sUserLEDGPIO.mPort,
               sUserLEDGPIO.mPin,
               sUserLEDGPIO.mPin);
}

//............................................................................
void QP::QF::onCleanup(void) {
}


extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    //QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    //NVIC_SystemReset();
}


void LM3S6965SSIPinCfg::SetPins(void) const {

  // Supports a single SSI device on port A.
  switch (GetID()) {
  case 0:
    // PA2: SSI0CLK
    // PA4: SSI0RX
    // PA5: SSI0TX
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeSSI(sSSIGPIOs.mPort,
                   sSSIGPIOs.mClkPin | sSSIGPIOs.mRxPin | sSSIGPIOs.mTxPin);

    GPIOPadConfigSet(sSSIGPIOs.mPort,
                     sSSIGPIOs.mClkPin | sSSIGPIOs.mTxPin,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
    break;
  default:
    // Do nothing.
    break;
  }
}


extern "C" {

void SysTick_Handler(void);
void SysTick_Handler(void) {

  // Call QF Tick function.
  QP::QF::TICK_X(0U, &l_SysTick_Handler);

  // Uncomment those line if need to publish every single tick.
  // Process time events for rate 0.
  // Publish to suscribers.
  //static QP::QEvt const sTickEvt = { SIG_TIME_TICK, 0U, 0U };
  //QP::QF::PUBLISH(&sTickEvt, &l_SysTick_Handler);
}


// GPIO port A interrupt handler.
void GPIOPortA_IRQHandler(void);
void GPIOPortA_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, lIsMasked);
  unsigned int lPin = BSP_gRTCCIntGPIOPtr->GetPin();
  if (lPin & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTA_BASE, lPin);
    RTCC_AO::GetInstancePtr()->ISRCallback();
  }
}


// GPIO port C interrupt handler.
void GPIOPortC_IRQHandler(void);
void GPIOPortC_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTC_BASE, lIsMasked);
  unsigned int lPin = sManualFeedGPIOPtr->GetPin();
  if (lPin & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTC_BASE, lPin);
    static BFHManualFeedCmdEvt sOnEvt  = { SIG_FEED_MGR_MANUAL_FEED_CMD, true };
    static BFHManualFeedCmdEvt sOffEvt = { SIG_FEED_MGR_MANUAL_FEED_CMD, false };
    if (Button::PRESSED == sManualFeedButtonPtr->GetGPIOPinState()) {
      BFH_Mgr_AO::AOInstance().POST(&sOnEvt, 0);
    } else {
      BFH_Mgr_AO::AOInstance().POST(&sOffEvt, 0);
    }
  }
}


// GPIO port D interrupt handler.
void GPIOPortD_IRQHandler(void);
void GPIOPortD_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTD_BASE, lIsMasked);
  unsigned int lPin = sTimedFeedGPIOPtr->GetPin();
  if (lPin & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTD_BASE, lPin);
    // Only interested in the pin coming high.
    if (Button::PRESSED == sTimedFeedButtonPtr->GetGPIOPinState()) {
      static BFHTimedFeedCmdEvt sEvt = { SIG_FEED_MGR_TIMED_FEED_CMD, 0 };
      BFH_Mgr_AO::AOInstance().POST(&sEvt, 0);
    }
  }
}


// GPIO port F interrupt handler.
void GPIOPortF_IRQHandler(void);
void GPIOPortF_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTF_BASE, lIsMasked);
  unsigned int lPin = sSelectGPIOPtr->GetPin();
  if (lPin & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTF_BASE, lPin);
    // Only interested in the pin coming high.
    if (Button::PRESSED == sTimedFeedButtonPtr->GetGPIOPinState()) {
      static QP::QEvt sEvt = { SIG_DISPLAY_REFRESH, 0 };
      DisplayMgr_AO::AOInstance().POST(&sEvt, 0);
    }
  }
}


void Ethernet_IRQHandler(void);
void Ethernet_IRQHandler(void) {
  ISR_Ethernet();
}

} // extern C

// *****************************************************************************
// NOTE00:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QK_ISR_ENTRY/QK_ISR_ENTRY
// macros or any other QF/QK  services. These ISRs are "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call any QF/QK services. In particular they
// can NOT call the macros QK_ISR_ENTRY/QK_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE01:
// The QV::onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV::onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE02:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
