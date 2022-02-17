---
layout: default
---

# Dev Board

While developping BFH project, the code size started approaching the limit of the lm3s6965 MCU.
With code size ever increasing with the addition of BLE, RFID and more, moving to a new development board became the only available option.

I dipped into my bin of dev board, and I found an unused Texas Instrument ek-tm4c129exl. With 1MByte of code space (4x time the size of the lm3s6965), this should be good for a long while.

The ek-tm4c129exl has the following features:
*  1MB code
*  256KB RAM
*  10M/100M Ethernet with integrated PHY
*  BoosterPack connectors for adding extensions

Moving from an ARM Cortex-M3 to -M4 is relatively easy. The major efforts should be porting to a whole new set of pins, and making a new Ethernet driver for a DMA-based GMAC.

# Adapter Board

The ek-tm4c129exl requires a lot of peripheral originally used in the initial version (BFH). Instead of building an homemade adapter board, I decided to make a full PCB.

For this, the BoosterPack connector are fully appropriate. There's two of those on the board, and I can dedicate one to add the following peripherals:

*  DS3234 RTCC with battery backup
*  MicroSD Card cage connector
*  TB6612 motor conttroller

## Construction

The schematic was captured using KiCad. Symbols were gathered from misc. projects found on the web. There's nothing outstanding about it.

The PCB was also captured using KiCad. It's a 2-layer board of minimal size of 1.5" x 2".

### Back Side

The back side being mostly a ground plane with a few straps. It was not possible to avoid these few ones.

### Front Side

The front side host all components.

### Fabrication

The gerber files were uploaded to [JLC PCB][https://jlcpcb.com/] with minor mods to fit their expected file formats and csv layout/fields.

I also got all capacitors, resistors/networks, even the MicroSD card cage installed at the factory for a fairly cheap price.

The DS3234 and connectors were soldered manually. The TB6612 was still BO at the time I received the boards.

### Future Improvements

For the next revision, the following improvements are planned:

*  SDCard power controlled via transistor
*  Addition of a microphone

This was super fun to do and I learned a lot while doing so.
