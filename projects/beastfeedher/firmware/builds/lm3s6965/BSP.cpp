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

// Corelink Library.
#include "SPI.h"

// This application.
#include "BFH_Mgr_AO.h"
#include "BFH_Mgr_Evt.h"
#include "BSP.h"
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

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

extern void ISR_Ethernet(void);

static void UserLEDInit(void);
static void EtherLEDInit(void);

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// Button objects.
static Button *sManualFeedButtonPtr = nullptr;
static Button *sTimedFeedButtonPtr  = nullptr;
static Button *sSelectButtonPtr     = nullptr;

static GPIOs  *sSelectGPIOPtr = new GPIOs(GPIO_PORTF_BASE, GPIO_PIN_1);

static GPIOs  *sFeederManualFeedPtr = new GPIOs(GPIO_PORTC_BASE, GPIO_PIN_4);
static GPIOs  *sFeederTimedFeedPtr  = new GPIOs(GPIO_PORTD_BASE, GPIO_PIN_4);

// RTCC GPIOs.
GPIOs *gRTCCCSnPtr = new GPIOs(GPIO_PORTA_BASE, GPIO_PIN_7);
GPIOs *gRTCCIntPtr = new GPIOs(GPIO_PORTA_BASE, GPIO_PIN_6);

// Motor controller GPIOs.
GPIOs *gIn1Ptr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_6);
GPIOs *gIn2Ptr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_5);
GPIOs *gPWMPtr     = new GPIOs(GPIO_PORTB_BASE, GPIO_PIN_0);

// SSD1329 GPIOs.
GPIOs *gDCnPtr     = new GPIOs(GPIO_PORTC_BASE, GPIO_PIN_7);
GPIOs *gEn15VPtr   = new GPIOs(GPIO_PORTC_BASE, GPIO_PIN_6);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

CoreLink::SPIDev * BSPInit(void) {
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
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  // Initialize SPI Master.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
  SSIPinCfg * const lSPIMasterPinCfgPtr = new LM3S6965SSIPinCfg(0);
  CoreLink::SPIDev *lSPIDevPtr = new CoreLink::SPIDev(SSI0_BASE,
                                                      *lSPIMasterPinCfgPtr);

#if 0
  // Initialize the OLED display.
  // Create an SPI slave configuration for the OLED display.
  CoreLink::SPISlaveCfg *lOLEDSPISlaveCfgPtr = new CoreLink::SPISlaveCfg();

  lOLEDSPISlaveCfgPtr->SetProtocol(CoreLink::SPISlaveCfg::MOTO_2);
  lOLEDSPISlaveCfgPtr->SetBitRate(4000000);
  lOLEDSPISlaveCfgPtr->SetDataWidth(8);
  lOLEDSPISlaveCfgPtr->SetCSnGPIO(GPIO_PORTA_BASE, GPIO_PIN_3);

  // The GPIO peripheral must be enabled before use.
  // Otherwise this will generate a HW fault exception.
  SSD1329 *lSSD1329Ptr = new SSD1329(*lSPIDevPtr,
                                     *lOLEDSPISlaveCfgPtr,
                                     GPIO_PORTC_BASE,
                                     GPIO_PIN_7,
                                     GPIO_PORTC_BASE,
                                     GPIO_PIN_6,
                                     128,
                                     96);
  lSSD1329Ptr->Init();
  lSSD1329Ptr->DrawImg(&gcat_drawing_128x91_neg_ImgBuf[0][0],
                       0,
                       0,
                       gcat_drawing_128x91_neg_Width,
                       gcat_drawing_128x91_neg_Height);

  lSSD1329Ptr->DrawStr("Bonjour",
                       0,
                       0,
                       0x8);
#endif

  // Debug UART port.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioInit(0);
  UARTprintf("Hello!\n");

  return lSPIDevPtr;
}


unsigned int BSPGPIOPortToInt(unsigned long aGPIOPort) {

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

  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

  // Init user LED.
  // Init Ethernet LEDs.
  UserLEDInit();
  EtherLEDInit();

  // Manual and Timed Feed cap sensor.
  // Those are debounced signals.
  unsigned long lIntNbr = BSPGPIOPortToInt(GPIO_PORTC_BASE);
  sManualFeedButtonPtr = new Button(*sFeederManualFeedPtr,
                                    lIntNbr,
                                    0U);
  lIntNbr = BSPGPIOPortToInt(GPIO_PORTD_BASE);
  sTimedFeedButtonPtr = new Button(*sFeederTimedFeedPtr,
                                   lIntNbr,
                                   0U);

  // Select button.
  // Not debounced, but simply used to turn display on.
  lIntNbr = BSPGPIOPortToInt(GPIO_PORTF_BASE);
  sSelectButtonPtr = new Button(*sSelectGPIOPtr, lIntNbr, 0U);

  // Manual Feed cap sensor input.
  // Timed Feed cap sensor input.
  sManualFeedButtonPtr->EnableInt();
  sTimedFeedButtonPtr->EnableInt();

  sSelectButtonPtr->EnableInt();
}

//............................................................................
void QP::QV::onIdle(void) {  // called with interrupts disabled, see NOTE01

  // Toggle the user LED, ON then OFF.
  QF_INT_DISABLE();
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               GPIO_PIN_0);
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               0);
  QF_INT_ENABLE();
}


static void EtherLEDInit(void) {
  // GPIO for Ethernet LEDs.
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_2,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(GPIO_PORTF_BASE,
                         GPIO_PIN_2);

  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_3,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinTypeEthernetLED(GPIO_PORTF_BASE,
                         GPIO_PIN_3);
  IntEnable(INT_ETH);
}


static void UserLEDInit(void) {
  // GPIO for user LED toggling during idle.
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_0,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);
  GPIOPinWrite(GPIO_PORTF_BASE,
               GPIO_PIN_0,
               GPIO_PIN_0);
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
    // PA3: SSI0FSS
    // PA4: SSI0RX
    // PA5: SSI0TX
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeSSI(GPIO_PORTA_BASE,
                   GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTA_BASE,
                     GPIO_PIN_2 | GPIO_PIN_5,
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

  // Process time events for rate 0.
  // Publish to suscribers.
  static QP::QEvt const sTickEvt = { SIG_TIME_TICK, 0U, 0U };
  QP::QF::TICK_X(0U, &l_SysTick_Handler);
  QP::QF::PUBLISH(&sTickEvt, &l_SysTick_Handler);
}


// GPIO port A interrupt handler.
void GPIOPortA_IRQHandler(void);
void GPIOPortA_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, lIsMasked);
  if (GPIO_PIN_6 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
    RTCC_AO::GetInstancePtr()->ISRCallback();
  }
}


// GPIO port C interrupt handler.
void GPIOPortC_IRQHandler(void);
void GPIOPortC_IRQHandler(void) {

  // Get the state of the GPIO and issue the corresponding event.
  static const bool lIsMasked = true;
  unsigned long lIntStatus = GPIOPinIntStatus(GPIO_PORTC_BASE, lIsMasked);
  if (GPIO_PIN_4 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
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
  if (GPIO_PIN_4 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTD_BASE, GPIO_PIN_4);
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
  if (GPIO_PIN_1 & lIntStatus) {
    GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_1);
    // Only interested in the pin coming high.
    if (Button::PRESSED == sTimedFeedButtonPtr->GetGPIOPinState()) {

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
