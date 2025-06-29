### SYS

Finally!, the last little block of instructions, the SYS group.
There are only 8 of them: NOP, SSI - SSO, SCL - SCH, RTS - RTI, and COR.
Let's go through them systematically.

#### NOP (No operation)

Here, the computer just sits pretty and passes its turn for one cycle. It's opcode is zero, so this is something like a "default" instruction.

#### Serial Port Control

The next four are for serial communication. In another document, there is a whole section on serial communication. But the basics are very simple.

When you transmit or receive over a serial data line, which is literally a wire, it goes just one bit at a time. So you put your data bit of electricity - low or high level voltage, 0 or 1 - on one end of the line.

###### Clock

The only way the receiver on the other end of the wire can let you know that they read your bit, and that they are ready for the next, is with another wire, the "clock" line.

The receiver puts a high level voltage on the clock line, and then a low level, like a "tick" of the clock. In electronics jargon that's called "pulling the clock high or low". This is an encoded message to you! It says: I'm done with your data bit.
 
Then you - the sender - should detect that tick and send the next bit. This computer uses two data lines, one for input, one for output. How do you know who you're talking to? Read up on the Enable register ("E").
 
#### SCL - SCH (Set Serial Clock Low / High)
 
The pulling high or pulling low of the clock line, that's all that **SCL** and **SCH** do.  So these two are out of the way in our explaining.
 
Now remember, you can only transmit or receive one bit at a time. There are two registers that help you a lot in this regard. SIR and SOR.
 
#### SSI (Shift Serial Bit In)
 
SIR is the Serial Input Register. One bit of it is connected to the serial input wire, and it has what is called a shift register. Every time you say "SSI", the bit that is on the serial input line is shifted into SIR, pushing out its highest order bit into nothing. So with every "SSI" instruction you execute, the SIR is slowly filling with bits, one by one, until 8 of them have been read in. Then you should stop and store the value in SIR somewhere, using a pair instruction with **source** "S" (for serial).
 
Just to clarify, you do get a value every time you read S, so might even check on progress after each shift. And remember what we said about serial communication, after each bit, you need to "tick" the clockline using the SCL and SCH instructions. The other document explains this in much more detail.
 
#### SSO (Shift Serial Bit Out)
 
SSO does the opposite! Just like the SIR, for turning input bits into bytes, there is another shift register for turning a byte into output bits. That register is called SOR. It stands for Shift Serial Out.
  
The way it works is like so: You copy your data byte into the SOR using a pair instruction with **target** "S".
Then you do 8 SSO instructions, and with each of them, the current high-order bit of SOR gets shifted out onto the serial output line, the low order places slowly filling with zeros.
Do remember what we said about serial communication, after each bit, you need to "tick" the clockline using the SCL and SCH instructions. The other document explains this in much more detail.

#### RTS (Return from Subroutine)

Three more to go and then we're done with the instruction set! RTS - Return from Subroutine. Assuming that you understand the call effect (_C), this reverses its effect. RTS increments L, so that the caller will have his local page back. Then it puts whatever is in B into C, and whatever is in O into the PC. So these two numbers (address B:O) had better point to the exact address you need to return to, because the next instruction fetch will occur there!

#### RTI (Return from TRAPS and Interrupts)

RTI (Return from Interrupt) is similar in operation but clears the "BUSY" flag. As long as this flag is set, the computer will not accept an interrupt. An interrupt when it is accepted by the computer is literally a "Trap0" instruction that you can't predict will happen. The computer makes you run a subroutine call to an interrupt service routine without asking you if this is a good time for this kind of thing. In the process, it sets the BUSY flag.

In contrast to RTS, a trap call and RTI do **not** use B:O at all. It does not alter your base pointer, so that if the trap handler does a good job, you will be returned to where you left off to exactly the same state you had and not notice a thing.

They use the amenity pointer "IA", for interrupt address, to store and retrieve the return address, which was explained in the section on BOP instructions.

Eventually the service routine will return, ending the little "excursion", and in order to really revert **everything** to normal, it clears the busy flag, too.

#### COR

And - drum roll! - the last of the lot: the COR instruction. COR stands for "Coroutine". This word should always be in the plural, since when you are using COR, you become the coroutine of another subroutine, and you are its coroutine.

COR puts whatever is in the base pointer (B:O) into C:PC, but keeps a copy (the return address) before overwriting these registers. Then, just before the jump, it overwrites the base pointer with the return address, so your coroutine knows where you left off.

The idea is that you both do your thing, taking turns: you jump to the coroutine, it does a bit of work, jumps backs to you, you do a bit of work and so on.

This instruction acts very much like a call/return, but without changing local pages. Of course you can also just use it as a jump to a 16-bit address.


