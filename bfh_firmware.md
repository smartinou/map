---
layout: default
---

# Firmware

These pages describe the firmware architecture.

*  [class diagrams](./bfh_class_diagrams)
*  [state machines](./bfh_state_machines)

## QP

One of the main goal for this project was to learn the QP framework. So guess what? It's built around QP.

![qp logo](./images/logo_ql.png)

QP is a real-time framework for building systems of event-driven, asynchronous active objects (actors) and hierarchical state machines.

QP is a formidable framework that is very different than traditional RTOSes.
In my job, I often had to design embedded system firmware that were strongly event-driven,
but never was quite satisfied with the implementation with traditional RTOS.

Years ago, I came across the 1st edition of "Practical Statecharts in C/C++: Quantum Programming for Embedded Systems" book,
written by Dr. Miro Samek.
I was quite impressed with its analysis of current RTOS programming problems, and its proposed solution.
This was my first acquaintance with QP.

A few years later, I was listening to Dr. Miro Samek himself on lectures about QP at Embedded System Conference in Boston.
I was more and more convinced I had to try this on a specific project.

Later on I bought the 2nd edition of the book. QP framework never quite left my mind since the day I first learned about it,
waiting for the proper opportunity to use it in my work.

Unfortunately, this never happened. Somehow, there's never time to experiment with new development methods,
and the only way to sell such paradigm change to management is to develop something on personal time and try to sell it
once done.

This is what this project was for: learn QP. Now that I did, I can only say that I never want to get back to traditional RTOS programming.

    > QP claims their "software and tools make software development fun again".
    >
    > I can only highlight how true this is!!!

You can find more about QP on its web site: [www.state-machine.com](https://www.state-machine.com)
There you will find tons of resources, links to download QP and related tools, e-books, app notes, and more.

## LwIP

This project uses a TCP/IP stack for setting the board configuration through a web page.

LwIP is a natural choice for an open-source TCP/IP stack.
In fact, the non-blocking nature of LwIP when used in raw/native API goes hand in hand with QP,
so much that there's an [application note](https://www.state-machine.com/doc/AN_QP_and_lwIP.pdf) written about it.

LwIP is actively maintained. It's very well suited to embedded development with its small footprint.
It is hosted at: [http://savannah.nongnu.org/projects/lwip/](http://savannah.nongnu.org/projects/lwip/)

## FatFS

FatFS is an open-source FAT-based file system.

It is used in this project to write logging files onto a micro SD Card.

