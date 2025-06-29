### Traps

Trap instructions are single-cycle subroutine calls. There are 32 of them, and they all do the same thing, with just a different address.

They act like instruction set extensions, because it's completely up to you to decide what happens, when their opcodes are executed.

You may or may not know this, but below what is called machine-code, many computers have another layer of even more primitive instructions, called microcode. The regular machine-code instructions are built from these microcode instructions, that tell the hardware exactly how the (macro-)instruction must perform.

So when you normally program in assembler, with each instruction, you are actually running a little microcode program.

The Myth computer is really primitive, so that it took fewer components to build it. It's instructions are actually microcode.

And the idea with implementing trap instructions was that you then have 32 free opcodes to implement more complex custom instructions in the form of single-byte subroutine calls.

A trap instruction opcode encodes a page number between 0 and 31. That's the base address for your call. The trap call always goes to offset zero of "its" page.

So if your goal is to write an instruction handler for "Trap5", you need to put a subroutine into page 5, starting at the first byte.

Page 0 is special: the instruction "trap0" (call page 0) as a side-effect sets the "busy" flag of the computer for reasons explained in the section on interrupts. As a general rule, while you're in page 0, or if the busy flag is on, you can't be interrupted, even if you didn't get there running "trap0": it's your safe zone.

In regular call-return type instructions (Call, COR, RTS), the base pointer (B:O) is used for saving and restoring the return address. But during trap calls and interrupts, the amenity pointer IA (Interrupt address) is used (we explained this in the section on BOPS), and you must use **RTI** (Return from Interrupt) to return from a trap or interrupt service routine. The advantage of this is that your trap or interrupt is completely transparent to the caller. Using trap instruction you can really build new, fully independent instructions.
