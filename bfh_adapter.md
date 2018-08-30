---
layout: default
---

# Adapter Board

And adapter board is required to host the main MCU development board, all the 3rd-party boards, \
and connectors to power and some cable harness to remote 3rd-party boards.

## Construction

I chose _BPS (Busboard Prototype Systems)_ for holding the various components and connectors.
I just love the FR4 glass-epoxy substrate that they offer. Way better than those brown bakelite boards!

[BPS web site](http://www.busboard.com)

*  Both boards 100 x 160mm (6.3 x 3.9"), single height (3U) Eurocard.
*  The grid is 38 by 62 holes, drilled 0.100" on centers.

The main board is the PowerBoard-U3 (part #POWU3).

I also required a small bit of StripBoard-U3 (part #STU3). I'll explain why with the pictures.

<img src="./images/proto_boards.JPG" alt="bsp proto boards " height="300" width="400">

### Back Side

This is a picture of the backside.
The board had to be cut and glued back together in order for the IDC connectors of the MCU board to fall on the right strips.

I performed this operation on my table saw, with a cross-cut sled. FR4 material is cut very nicely with clean edges.

The outer strips are used for ground and power rails (3.3V, 5V, 12V).

![back side](./images/adapter_board_back.JPG)

The next picture shows a zoom on the glued seam. I used 30-min epoxy, spread very thin. A very small amount of oozing can be seen.
The challenge is to keep the holes spread 0.1" apart. Using male IDC connectors is handy while the glue set.

![seam zoomin](./images/adapter_board_seam_zoomin.JPG)

### Front Side

The font side holds all the connectors, components and wires to connect each components to each other.

The picture shows the strip of stripboard that is glued over the seam. With the coper side on top,
it is possible to connect the back side traces over the seam by soldering a piece of wire between sides.

After the epoxy glue dried, it was necessary to re-drill some of the wholes. This was an easy job using the proper drill bit.

Overall, the cutting, gluing and re-drilling of the adapter board took no more than an evening! No need to say that it took way more time to solder the wiring and connectors...

![front side](./images/adapter_board_front.JPG)

### Populated Board

This picture shows the board when populated with the majority of the components.

![populated board](./images/adapter_board_with_components.JPG)

*  __Bottom left corner__: +5V/+12V power connector; connector for cat feeding pad.
*  __Top left corner__: RTCC.
*  __Center__: LM3S6965-EK dev board.
*  __Top right corner__: TB6612 DC motor controller.
*  __Bottom right corner__: Manual feeding pad button (toggle touch sensor).



