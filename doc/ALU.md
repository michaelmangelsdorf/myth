### Doing Maths and Things to do with Bits

This section is all about the accumulator: register A and its reclusive  sister register, X. Well, there is actually a third player that also has an important role: the Arithmetic Logic Unit, or ALU.

Accumulator review: When you read A, you get its value back. When you write A, its current value gets copied into X, and then the new value is stored in A (like a little push-down stack).

#### ALU

What's an ALU? It's a very common, central part in a computer that takes input from registers, performs one of a number of possible operations (the instruction tells it which one it is) and then stores the result back in some register.

In this computer, the ALU sees what is in A and X, so it bases its operation on those two values. There are 16 possible operations, 16 instructions. Let's look at them in a systematic way.

Something noteworthy is that the first half of the ALU instructions in numerical order just produce a primary result ("the" result) and it gets stored in A. X remains unchanged.

The other half of the instructions gives you the primary result in A (the one you **probably** want, but also another useful aside, stored in X).

#### Operations that leave X alone

##### NOT

This instruction just inverts the bits in A. Those bits that are 0 become 1, those that are 1 become 0. This is also called the "one's complement". When you add 1 to it, it becomes "minus" the original number, the negative version of it. Look up how this works, it's fascinating: This is called the two's complement. It is how the computer actually does subtraction behind then scenes.

Did you know that doing "EOR FFh" to a byte has the same effect as NOT? This is beclause Exclusive-OR is 1 if and only if the two input bits are different. The number FFh has all its bits set to 1. So if a bit in the other number is 1, that is the same as the corresponding bit in FFh, so the result bit is 0. And if the bit is 0, well then it's different from the 1 bit in FFh, so the result is 1.

##### ALX, AEX, AGX

These stand for A-less-than-X, A-equal-to-X, and A-greater-than-X. They produce a number in A that is 0, if the named condition is false, and 255 (all bits set) if the condition is true. If you are wondering why 255: You can say NOT and get the opposite, for example to turn "A-less-than-X" into "A-greater-than-or-equal-than-X".

##### AND, IOR, EOR

These compute A AND X, A OR X (inclusive or), and A XOR X (exclusive or), and store their result in A.

#### XA

Aha! So you **can** get the value of X. With this instruction, it gets stored in A.

#### Operations with secondary result in X 

#### AX SWAP

AX: Stores A into X. Swap: Swaps A and X.

#### SHL SHR ASR
* SHL shifts every bit in A to the left by one position. Position one - the first bit - is set to zero. So far so good. At this point, importantly, X gets zeroed. Now. The highest order bit from the original number (the bit that got "pushed" out) is put **back** as the first bit of X. So at the end, X either contains 0 or 1.

    This instruction is exactly the same as multiplying A by two, or adding A to itself. Think about it.

* SHR is the opposite. Every bit is shifted one position to the right. The highest position - bit 7 - is set to zero. So far so good. X gets cleared again. Now. The lowest order bit from the original number (the bit that got "pushed" out) is put **back** as the highest-order bit of X. So at the end, X contains either 0 or 80h (bit 7 set).

  This instruction is exactly the same as diving A by two, if you prefer to use the larger "unsigned" range of your bytes.

* ASR is just like SHR, with one little difference:

  Every bit is shifted one position to the right. The highest position - bit 7 - is set to **whatever the highest-order bit of the original number was**. So far so good. X gets cleared again. Now. The lowest order bit from the original number (the bit that got "pushed" out) is put **back** as the highest-order bit of X. So at the end, X is either 0 or 80h.

  The reason why the high-order bit is copied down has to do with negative binary numbers. As we said in an earlier section, negative binary numbers have their highest bit set to 1. The highest bit is often abbreviated to MSB (Most Significant Bit). When you use SHR (the other shift-right instruction), the MSB changes from a possible 1 to a zero. Practically, let's say you divide -6 by 2, with SHR you would get 3, not -3. Whereas ASR gives the correct result in this case. For this reason, ASR stands for Arithmetic Shift Right.
  
  This instruction is exactly the same as diving A by two, if you prefer to use the smaller, "signed" range of your bytes.

#### ADDC ADDV SUBB

* ADDC is the easiest of the three. A receives the sum of A and X. If the sum didn't fit into a byte, then X is set to 1. Else, X is set to zero.

  Example: I'm sure you know that the maximum unsigned number you can store in a byte is 255. So if A=60 and X=200, then ADDC will leave A=5 and X set to 1.

  Just like in decimal: Add 6 to 5, and you need another 1 to the left, because it's greater than 9. That one is called the carry bit. ADDC stands for ADD and CARRY. This instruction is for when you are treating your bytes as "unsigned".

  In preparation for explaining the next instruction, think about this. Our example from before, 200+60=5 looks like this in hexadecimal: C8+3C=5, CARRY=1. So the result (05h) is "wrong" in a way. But in some other way, it only can't stand by itself, the carry must be part of it. So you can fix it by prefixing it with the carry: 105h = 255 + 5 = 260.
  
  The carry bit often comes into play when you have a 16-bit address, and add a small number to the low-order byte. If the carry bit is clear, you don't have to do anything to the high-order byte. But if it is set, you must increment the high-order byte by 1 to fix the address. The _U effect does this for you. Remember, when you store a number info _U, that number gets added to the B:O register pair. And although you are just adding a byte to O, the result may spill over into B, or underflow O. And _U has your back and updates B as required.

* ADDV also puts the sum of A and X into A. But the value that will be in X is somewhat less intuitive. X gets the "overflow flag". This instruction is strictly for when you're treating your bytes as signed.

    As we said, the carry bit is for **fixing** the result when the addition of two unsigned numbers overflows a byte.

    The Overflow flag is different - you won't be able to use it directly to fix your result. All it tells you is that the addition didn't work, the two operands when added are out of range. The overflow flag is an error flag. ADDV stands for ADD and OVERFLOW.

    You can tell that your result is wrong, when you add two positive numbers (A and X) but you get a negative result. And also a positive result when you add two negative numbers is clearly wrong. And that is what the overflow flag tells you, that one of these two cases occurred.

    The interesting thing, and this does take a bit of thought, is to realise this: when your two numbers have different signs, one positive and a negative number, there is just no way that you can go wrong when adding them as bytes.

* SUBB is more easy! It means: subtract A from X, assume that they are both "unsigned" and store the "borrow" bit into X. What on earth is the borrow bit? The borrow bit is 1 if the result of the subtraction is negative.

    In other words, if A is greater than X. You are subtracting a larger positive number from a smaller positive number, so the result is negative? Boom, borrow bit is 1.

    Just like with ADDC and the carry bit, you **can** use the borrow bit to fix your result! It isn't just an error flag. While the carry meant that you have to add it to the higher-order byte of your addition, the borrow flag means you have to **subtract** it from the higher-order byte to complete your calculation. ("You need a larger number to subtract from and come out positive").
    
    The borrow bit often comes into play when you have a 16-bit address, and subtract a small number from the low-order byte. If the borrow bit is clear, you don't have to do anything to the high-order byte. But if it is set, you must decrement the high-order byte by 1 to fix the address. The _U effect does this for you. Remember, when you store a number info _U, that number gets added to the B:O register pair. And although you are just subtracting a byte from O (oh, not 0), the result may spill over into B, or underflow O. And _U has your back and updates B as required.
    
  A good way to remember that A gets subtracted from X, and not the other way around, is the actual way in which you proceed. First you push a number into A, then you realize you need to subtract 5 from it, so you push the five. At this point, the number that is subtracted (the 5) is in A, and your orginal number (the one you want to subtract from) has been pushed into X. You then do SUBB and are left with the result in A.








 







