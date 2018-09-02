

# State Machines & Signals

This page details the various state machines used in the firmware.

As explained in another page, QM modeling tool __was not__ used to create the various code representing the 
state machines of the system. Instead, they were all coded by hand, deriving from the proper QP base class.

    > I later used QM modeling tool to try it out, and re-modeled some of the state machines
    > of this project (LwIPMgr_AO and DisplayMgr_AO).
    > I found really useful that the tool both generates the code, and the state machine diagrams.
    > 
    > This removes the step of having to use other tools to create UML state machine diagrams,
    > thus following the "don't repeat yourself" (DRY) rule.
 
## RTCC_AO
 
The RTCC manages the periodic and alarm interrupts.
 
When one of the two happen, it simply publish an event to the QP framework that such thing occurred.

    > The periodic interrupt can be set to the desired response granularity of the system.
    > For instance, it is possible to set the periodic alarm to every minute,
    > leaving the system in low power mode in between interrupts.
    >
    > Currently the periodic interrupt is set to the second for better observability.

### Diagram

![rtcc_ao_sm](./images/rtcc_ao_state_machine.png)

### Signal List

## LwIPMgr_AO
 
This is the state machine responsible for managing the low-level Ethernet driver with respect to signals.
 
It is copied over 95% of the original application note from QP on how to use LwIP in conjunction with QP,
found [here](https://www.state-machine.com/doc/AN_QP_and_lwIP.pdf)
 
### Diagram
 
See the application note above.

### Signal List

See the application note above.

## BFHMgr_AO

The state machine responsible for handling the various feeding and alarm events.

### Diagram


### Signal List

