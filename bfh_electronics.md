---
layout: default
---

# Electronics

## Introduction

Based on EKI-LM3S6965 dev board (ARM Cortex-M3), now owned by TI: [link](http://www.ti.com/product/LM3S6965)

[datasheet](http://www.ti.com/lit/ug/spmu029a/spmu029a.pdf)

![lm3s6965](./images/LM3S6965.jpg)
![lm3s6965_kit](./images/lm3s6965_kit.jpg)

 __Not recommended for new designs!!!__
 
    > This MCU is obsoleted and not recommended for new designs.
    >
    > This board was chosen because I had one sleeping on my desk.
    > HW and FW are done so that they could easily be ported to another
    > TI-based MCU/dev board, or any ARM Cortex-MCU.

__Key Features__
*  50MHz clock
*  256KB embedded flash
*  64KB embedded RAM
*  10/100 Ethernet port (MAC + PHY)

![dev_board](./images/ek-lm3s8962_fig.1.jpg)

## Block Diagrams

![block diagram](./images/lm3s6965ek_block_diagram.png)

![lm3s6965 internals](./images/custom_diagram_1_LM3S6965.gif)

## Pinout

The dev board has two breakout connectors. Some of the pins have a dedicated usage on the dev board. Some of the dev board features (and thus pins) are used, like the OLED display, buzzer, navigation buttons, etc.

If an alternate use of the pin is required, a soldering bridge can be cut to free the pin from its dedicated usage.

The following tables show the allocated usage for each breakout connectors.

| Comment                          | EK Usage | Description | Pad No | Pad No | Description  | EK Usage | Comment                 |
|----------------------------------|----------|-------------|--------|--------|--------------|----------|-------------------------|
| Cat feed pad out.                |          | PD4/CCP0    | 34     | 33     | PB4/C0-      |          | BLE CSn                 |
|                                  |          | PD6/FAULT   | 32     | 31     | PD5/CCP2     |          |                         |
|                                  |          | GND         | 30     | 29     | PD7/CCP1     |          |                         |
|                                  |          | ADC1        | 28     | 27     | ADC0         |          |                         |
|                                  |          | ADC3        | 26     | 25     | ADC2         |          |                         |
| SmartCard SPI CSn signal.        | CARDCSn  | PD0/IDX0*   | 24     | 23     | GND          |          |                         |
|                                  |          | PD2/U1RX    | 22     | 21     | PD1/PWM1*    | SOUND    | Speaker.                |
|                                  |          | PG1/U2TX    | 20     | 19     | PD3/U1TX     |          |                         |
| Data/Control#                    | OLEDDC   | PC7/PHBO*   | 18     | 17     | PG0/U2RX     |          |                         |
|                                  |          | PC5/C1+     | 16     | 15     | PC6/CCP3*    | EN+15V   | OLED +15V enable.       |
|                                  |          | GND         | 14     | 13     | PC4/PHA0     |          | Manual feed pad out.    |
|                                  |          | +3.3V       | 12     | 11     | PA0/U0RX*    | VCP_RX   | USB Stuff. Don't touch. |
| USB Stuff. Don't touch.          | VCP_TX   | PA1/U0TX*   | 10     | 9      | PA2/SSI0CLK* | SSICLK   | SPI Clock signal.       |
| RTI OLED display SPI CSn signal. | OLEDCSn  | PA3/SS0FS*  | 8      | 7      | PA4/SSI0RX*  | SSIRX    | SPI Rx signal.          |
| SPI Tx signal.                   | SSITX    | PA5/SSI0TX* | 6      | 5      | PA6/SCL1     |          | RTCC SQW.               |
| RTCC CSn                         |          | PA7/SDA1    | 4      | 3      | GND          |          |                         |
|                                  |          | GND         | 2      | 1      | +5V          |          |                         |
* * *
    
| Comment                 | EK Usage   | Description | Pad No | Pad No | Description | EK Usage  | Comment                      |
|-------------------------|------------|-------------|--------|--------|-------------|-----------|------------------------------|
|                         |            | +12V        | 35     | 36     | GND         |           |                              |
| Motor controller BIN2.  |            | PB5/C1-     | 37     | 38     | PB6/C0+     |           | Motor controller BIN1.       |
|                         | JTAG       | PB7/TRSTn   | 39     | 40     | PC2/TDI*    | JTAG      |                              |
|                         | JTAG       | PC3/TDO*    | 41     | 42     | PE3/PHA1*   | RIGHT_SWn | Right Button.                |
| Left button.            | LEFT_SWn   | PE2/PHB1*   | 43     | 44     | GND         |           |                              |
| Up button.              | UP_SWn     | PE0/PWM4*   | 45     | 46     | PE1/PWM5*   | DOWN_SWn  | Down Button.                 |
| Reserved for I2C.       |            | PB2/SCL0    | 47     | 48     | PB3/SDA0    |           | Reserved for I2C.            |
| BLE IRQn                |            | PB1/PWM3    | 49     | 50     | GND         |           |                              |
| Select button.          | SELECT_SWn | PF1/IDX1*   | 51     | 52     | PB0/PWM2    |           | Motor controller PWMA input. |
| RJ45 Link/Activity LED. | LED0       | PF3/LED0*   | 53     | 54     | PF2/LED1*   | LED1      | RJ45 Link/Activity LED.      |
|                         |            | GND         | 55     | 56     | OSC32       |           |                              |
|                         |            | GND         | 57     | 58     | OSC32       |           |                              |
| Status LED.             | LED2       | PF0/PWM0*   | 59     | 60     | +3.3V       |           |                              |


    > In tables above, an asterisk (*) by a signal name (also on the EVB PCB) indicates the signal
    > is normally used for on-board functions.
    >
    > Normally, you should cut the associated jumper (JP1-15) before using an assigned signal
    > for external interfacing.
    
![component locations](./images/connection_details_w_color.png)

## Adapter Board

An adapter board has to be build for interfacing the MCU dev board with all the various 3rd-party extension/breakout boards.

![BFH board](./images/BFH.png)

[Adapter board construction](./bfh_adapter)

## 3rd-Party Extension/Breakout Boards

### Touch sensors
Manual Feed Pad is done with a toggle  capacitive touch sensor from AdaFruit. It is based on AT42QT1012.

<img src="./images/adafruit_at42qt1012.jpg" alt="adafruit at42qt1012" height="100" width="150">
<https://www.adafruit.com/product/1375>

Cat Feed Pad is done with a momentary capacitive touch sensor from AdaFruit. It is based on AT42QT1010. It is installed away from the main board, linked to it via a connector.

<img src="./images/adafruit_at42qt1010.jpg" alt="adafruit at42qt1010" height="100" width="150">
<https://www.adafruit.com/?q=at42qt1010>

### PWM motor controller
The DC/Stepper motor controller breakout board is the one from AdaFruit. It can control two motors. It is build around TB6612 motor driver. Only  motor controller half 'A' is used.

<img src="./images/adafruit_tb6612.jpg" alt="adafruit tb6612" height="100" width="150">
<https://www.adafruit.com/product/2448>

### RTCC
The RTCC breakout board is from SparkFun. It is build on the DS3234.

It was selected because of its accuracy (temperature compensated), alarm capabilities and its battery-backup'ed RAM. It is used by FW to store settings

<img src="./images/sparkfun_ds3234.jpg" alt="sparkfun ds3234" height="100" width="150">
<https://www.sparkfun.com/products/10160>

### RFID Detector
The RFID controller from AdaFruit is installed remotely of the board using a connector.

<img src="./images/adafruit_rfid.jpg" alt="adafruit rfid" height="100" width="150">
<https://www.adafruit.com/product/789>

### Bluetooth Low Energy (BLE)
The BLE board is from Adafruit. This version uses an SPI interface to communicate with the MCU.

<img src="./images/adafruit_ble_spi.jpg" alt="adafruit ble" height="100" width="150">
<https://www.adafruit.com/product/2633>
