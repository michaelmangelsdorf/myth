This is my successor project to
[Sonne8](https://github.com/michaelmangelsdorf/Sonne8), a cpu/micro-controller built with 74HC series logic chips, some CMOS memory and passive-components only.

I've built a little working computer with it as a proof-of-concept. A demonstration video in the other repo shows the controller load a program for multiplying two numbers from a serial EEPROM using a native SPI implementation. It then computes the result and displays it on a 7-segment display with its inbuilt parallel I/O functions.

The new project over here will help me explore a modified version of the CPU a little more and write software for it, before developing more hardware for it.

Here is an example of a native multiplication routine which multiplies the two 8-bit numbers in the accumulator, leaving the accumulator with the 16-bit result (high-order in A).

    ;Multiply A times X, result in A:X
    ;A:X accumulator acts as a two-element push-down stack when writing to A
    ;Both regs are the implied ALU operands, primary result in A, secondary in X
    @mul8
        a1          (Save multiplicand into L1 - turns into low order result)
        xa a4       (Save multiplier into L4)
        fa 0, a2    (Set high-order result to 0, keep in L2)
        fd 7        (Initialise loop counter, 8 bits to process)
        @loop
            fa 0000_0001b, 1a AND  (Check LSB of multiplicand)
            fz >a                  (Skip if zero)
                4a 2a ADDC a2      (Add multiplier to high order result)
            @a
            1a SHR, a1             (Shift low-order result right)
            2a SHR, a2             (Shift high-order result right, LSB saved to X)
            xa 1a IOR, a1          (Carry high-order LSB into low-order MSB)
        fw <loop
        1a          (Push low-order result)
        2a          (Push high-order result)
        RET

The schematics have been stripped of everything I thought wasn't essential, so it's a bit easier to read. In other words, although it's possible to load the schematics file into KiCad, its intended use is only for reference.

The C file is also for reference. It shows the intended workings of the CPU. For a more detailed description of this microcontroller read "spec.md".

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/sonne8pcb.jpg)

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/mythkicad.png)

The component count is now higher than for the prototype, because in this project I've implemented the ALU (top and top-right in schematics) in discrete logic, where it used to be just look-up tables in a big PROM.
