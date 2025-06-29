### SYS

Finally!, the last little block of instructions, the SYS group.
There are only 8 of them: NOP, SSI - SSO, SCL - SCH, RTS - RTI, and COR.
Let's go through them systematically.

#### NOP (No operation)

Here, the computer just sits pretty and passes its turn for one cycle. It's opcode is zero, so this is something like the "default" instruction.

#### Serial Port Control

The next four are for serial communication. In another document, there is a whole section on serial communication. But the basics are very simple.

When you transmit or receive over a serial data line, which is literally a wire, it goes just one bit at a time. So you put your data bit of electricity - low or high level voltage, 0 or 1 - on the line, so that the receiver can sample your data: is it a zero or a one bit?

###### Clock

Now that the received knows which kind of bit you sent, a good way for her to let you know know that she has read your bit and is ready for the next, is to use a second wire, the "clock" line.

Just like you have put a data bit on your line, your partner now puts a "tick" on the clock line. For example: low, then high, then low again. This is an encoded message to you! It says: I'm done with your data bit.
 
Then you - the sender - detect that tick on the clock line and send your next data bit. Rinse and repeat, that's all there is to it.

Apart from the clock line, this computer uses two data lines, one for input, one for output.
 
#### SCL - SCH (Set Serial Clock Low / High)

These two instructions are for controlling the state of the clock line. When you say SCL, the clock signal is set to "low" (0) and when you say SCH, the clock line is set to "high" (1). 

Now we need to figure out a way to convert a byte into a series of bits and vice-versa. There are two registers in this computer which do that: SIR and SOR.
 
#### SSI (Shift Serial Bit In)
 
SIR is the Serial Input Register. One bit of it is connected to the serial input wire, and it has what is called a shift register.

Every time you say "SSI", the bit that is on the serial input line is shifted into SIR at the lowest bit position, pushing out its highest order bit into nothing.

So with every "SSI" instruction you execute, the SIR is slowly filling up with bits, one by one, until 8 of them have been read in. Since 8 bits are all that the SIR can hold, if you execute more SSIs than that, the first bits that you shifted in will be "pushed out" of the shift register and you would be losing them.

So the sensible thing after 8 SSI instructions is to read out the SIR and store your data byte somewhere. You can do this with a pair instruction like SD (store SIR into D).
 
Also, you in between issuing the SSI instructions, you should also tick the clock-line using SCL and SCH as we said, to synchronise your partner.
  
#### SSO (Shift Serial Bit Out)
 
SSO is the Serial Output Register. Just like the SIR is converting bits into bytes, the SOR is turning bytes into bits.
 
For this to work, you store a byte into the SOR with a pair instruction like DS (store D into SOR). Then you do 8 SSO instructions, and with each of them, the current high-order bit of SOR gets shifted out onto the serial output line, the low order places slowly filling with zeros.

Also, you in between issuing the SSI instructions, you should also tick the clock-line using SCL and SCH as we said, to synchronise your partner.

#### RTS (Return from Subroutine)

This instruction reverses a call instruction. RTS increments L, so that the caller will have his local page back. Then it puts whatever is in B into C, and whatever is in O into PC. So the base pointer (B:O) had better point to the exact address you need to return to, because the next instruction is going to be fetched from there.

#### RTI (Return from TRAPs and Interrupts)

RTI (Return from Interrupt) is similar in operation but "clears the BUSY flag".

Let's explain this in a little more detail: As long as this flag is set, the computer will not accept an interrupt. An interrupt, when it is accepted by the computer, is literally a "Trap0" instruction that you can't predict will happen!

The computer - without you knowing - makes you run a call to an "interrupt service routine", and in the process, it sets the BUSY flag.

In contrast to RTS, trap calls, interrupts (same as Trap0!) and RTI do **not** use B:O to store the return address. And since they don't alter your base pointer, if the trap handler does a good job, you will be returned to where you left off to exactly the same state you were in and not notice a thing.

Traps and RTS use the amenity pointer "IA" (Interrupt Address), to store and retrieve the return address, instead of the base-pointer.

#### COR

COR stands for "Coroutine". This word should always be in the plural, since when you are using COR, you become the coroutine of another subroutine, and you are its coroutine.

COR puts whatever is in the base pointer (B:O) into C:PC, but keeps a copy (the return address) before overwriting these registers. Then, just before the jump, it overwrites the base pointer with the return address, so your coroutine knows where you left off.

The idea is that you both do your thing, taking turns: you jump to the coroutine, it does a bit of work, jumps backs to you, you do a bit of work and so on.

This instruction acts very much like a call/return, but without changing local pages. Of course you can also just use it as a jump to a 16-bit address.


