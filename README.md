This is my successor project to
[Sonne8](https://github.com/michaelmangelsdorf/Sonne8), a cpu/micro-controller built with
74HC series logic chips, some CMOS memory and passive-components only.

I've built a little working computer with it as a proof-of-concept. A demonstration video in the other repo shows the controller load a program for multiplying two numbers from a serial EEPROM using a native SPI implementation. It then computes the result and displays it on a 7-segment display with its inbuilt parallel I/O functions.

The new project over here will help me explore a modified version of the CPU a little more and write software for it,
before developing more hardware for it.

The schematics have been stripped of everything I thought wasn't essential, so it's a bit
easier to read. In other words, although it's possible to load the schematics file into KiCad,
its intended use is only for reference.

The C file is also for reference. It shows the intended workings of the CPU. For a more detailed description of this microcontroller read "spec.md".

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/sonne8pcb.jpg)

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/mythkicad.png)

The component count is now higher than for the prototype, because in this project I've implemented the ALU (top and top-right in schematics) in discrete logic, where it used to be just look-up tables in a big PROM.
