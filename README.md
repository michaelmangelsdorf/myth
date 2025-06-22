### Intro

This is my successor project to [Sonne8](https://github.com/michaelmangelsdorf/Sonne8), a discrete micro-controller built using about one-hundred 74HC series logic chips, some CMOS memory and passive-components only. It runs about 1 million instructions per second on a four-layer PCB.

A demonstration video in the other repo shows the controller board with a companion I/O-board I designed stacked on top of it.

In the video, it loads a program for multiplying two numbers from a serial EEPROM running a native SPI implementation. It then computes the result and displays it on the I/O boards 7-segment display.

The new project over here will help me explore a modified version of the CPU a little more and write software for it, before developing more hardware.

The schematics have been stripped of everything I thought wasn't essential, so it's a bit easier to read. In other words, although it's possible to load the schematics file into KiCad, its intended use is only for reference.

The C file is also for reference. It shows the intended workings of the CPU. For a more detailed description of this microcontroller read "spec.md".

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/sonne8pcb.jpg)

![PCB with working Sonne8 micro-controller](https://github.com/michaelmangelsdorf/myth/blob/main/mythkicad.png)

The component count is now higher than for the prototype, because in this project I've implemented the ALU (top and top-right in schematics) in discrete logic, where it used to be just look-up tables in a big PROM.


### Status

There is now a command-line tool (`my`) for exploring Myth. It includes an assembler. The tool is documented in the [specification](https://github.com/michaelmangelsdorf/myth/blob/main/spec.md) file.

### Code Example

Here is an example of a native multiplication routine which multiplies the two 8-bit numbers in the accumulator, leaving the accumulator with the 16-bit result (high-order in A). The listing is the output of the assembler using `my -la`.

    ADDR:  OBJCODE:                  LIN:  SOURCE:
                                     0001  
                                     0002  (Multiply 4x7)
    0000:  84 04 84 07 80 03         0003  fa 4, fa 7 - fc >MUL8. ; Sets A to 7, X to 4 and calls mul8
    0006:  8C 06                     0004  @idle fj <idle         ; my-tool stops at 64k cycles
                                     0005                         ; Check the result using: "my -p"
                                     0006  
                                     0007  ; Multiply A times X, result in A:X
                                     0008  ; A:X accumulator acts as a two-element push-down stack when writing to A
                                     0009  ; Both regs are the implied ALU operands, primary result in A, secondary in X
                                     0010  
                                     0011  ; Using page 3: "my"-tool uses p1 for persisting regs, p2 as IO-buffer
                                     0012  
                                     0013  3@MUL8
    0300:  68                        0014      a1          (Save multiplicand into L1 - turns into low order result)
    0301:  17 6B                     0015      xa a4       (Save multiplier into L4)
    0303:  84 00 69                  0016      fa 0, a2    (Set high-order result to 0, keep in L2)
    0306:  85 07                     0017      fd 7        (Initialise loop counter, 8 bits to process)
                                     0018      @loop
                                     0019          fa b0000_0001, 1a AND  (Check LSB of multiplicand)
    030C:  8E 12                     0020          fz >a                  (Skip if zero)
    030E:  63 61 1D 69               0021              4a 2a ADDC a2      (Add multiplier to high order result)
                                     0022          @a
                                     0023          1a SHR, a1             (Shift low-order result right)
    0315:  61 1B 69                  0024          2a SHR, a2             (Shift high-order result right, LSB saved to X)
    0318:  17 60 15 68               0025          xa 1a IOR, a1          (Carry high-order LSB into low-order MSB)
    031C:  8B 08                     0026      fw <loop
    031E:  60                        0027      1a          (Push low-order result)
    031F:  61                        0028      2a          (Push high-order result)
    0320:  05                        0029      RTS


Source Code ![Source](https://github.com/michaelmangelsdorf/myth/blob/main/syntax.png) of the multiplication routine in Sublime text with syntax highlighting.
