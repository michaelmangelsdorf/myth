
### Instructions of Type PAIR

Pair instructions are best explained by looking at their mnemonics. A pair mnemonic consists of two letters, the first being the source register, the second being the target register. Pair instructions copy a value from their source to their target.

There are 8 sources: F, M, B, O, A, D, S, P.

All sources can also be targets, except F.

In addition to these 7 sources we can use as targets, there are 9 additional targets which in turn can't be sources: C, E, K, U, W, J, H, Z, N.

So in total, there are 16 targets.

You can now combine these to form "pairs": FK (Copy F to K), AE (Copy A to E), and so forth.

Regarding pair instructions, there is one additional rule, and one exception to this rule to remember.

#### Scrounging

The rule is that when both letters (source and target) are the same, that means something different! - another instruction entirely.

This is called "scrounging" the original pair. The simplified reason for this feature is that copying the value from one register into itself (AA - copy A to A, for example) is not overly useful, so we are doing something else instead!

Now, the exception to this rule of scrounging, just something to remember, is that since F is not a target register, we scrounge FM instead, even though F and M are not the same letters.

Here are all the scrounged combinations, and what they "redirect" to, which instruction they do instead:

FM: KEY, MM: CODE, BB: LOCAL, OO: LEAVE, AA: ENTER, DD: INC, SS: DEC, PP: EA

Each of these will be explained in the appropriate section.

#### Effects

Some of the sources and targets are what are called effects. Effect means that instead of being a little physical storage location (called a "register"), reading or writing it triggers a specific side effect. It has an action. We could say that effects are just names for their actions, instead of names for a storage cell or register.

The actions can be quite surprising, they can enable hardware, or jump to a different subroutine, so be prepared, we will explain them all as we go on.

We are now going to take a systematic look at each of the 8 sources.

### F

F stands for fetch. F is an effect, and it can't be used as a target. Its action is that when you read from it, the computer looks at the byte that *would be* the next instruction in memory. It reads that byte and uses it as the source value. Then it skips over the byte, when it looks for the next instruction. This process is called "fetching", and the byte is called a literal. The way you use this in your program is like so: "FB 2", for example. The computer sees the FB pair instruction (copy F to B), and this triggers the read action of F. The next instruction would be the literal 2, but F reads and then skips it, instead of fetching it as an instruction in the next cycle. It then takes the 2 as the source value and copies it to B.

### M

M stands for memory. M can be a source or a target. This computer has only one memory pointer, in the form of the B:O register pair. This register pair is called the base pointer, and B is called the base register. The O register is called the offset register. To read a sixteen bit address, you have to set B to the high order byte (the "base" address or "page"), and O to the low order byte (the "offset") of the address. This is best viewed in hexadecimal: Setting B to 20h and O to 30h means setting B:O to the 16-bit address 2030h.
To actually access the memory data pointed to by B:O, you then have to use a pair instruction that uses M as source or target. M is an effect. Its actions are as follows: When you read from M, you get as source the value of the memory cell at the 16-bit address formed by B:O as we said before. When you write to M, the written value is stored at the B:O location in memory.

B stands for base, as has been explained. B can be a source or a target register. The same goes for O, which stands for offset, and has also been explained already.

### A

A stands for accumulator. A can be source or target. The accumulator is the heart of the computer, and it is both a very important effect and a very important register. It takes its name from the fact that it accumulates results. Although A takes its name from the word, the accumulator of this computer is actually made up of **two** registers working together. It shares its effect with a reclusive friend, the X register. X stands for mystery. You may have noticed that we didn't mention X when we listed the pair sourcesor targets. So how do you get at it? We will explain more about A and X in the section about doing math and magic. For now, let's just explain one curious thing about A: When you read from it, it's just a little storage cell with a value, but it has a write-"action". Writing to it does two things. First, the computer copies A into X. And only then it stores the source value that you are writing into A. It works like a little push down stack - you "push" the value into A, which in turn pushes its previous value down into X, to maybe save it for later.
"A" really is an effect register, it is both! Finally, as this is a good place for it, let's reveal three of the infamous "scrounge instructions" (remember, these are combinations of equal pair letters: BB, OO, and so one, that are replaced by other 
, more useful instructions): unspectacularly, INC increments A, DEC decrements A. And then there is EA, which pushes E into A, using A's action that we have just explained. This instruction is the only way to get to the value of E!

### D

D stands for "down" or "decrement". D can be source or target. D is a register, just a little storage location. But there is a "while" effect that always subtracts one from it and treats it like a loop counter.

### S

S stands for serial. S can be source or target. Serial stands for two registers or storage locations, depending on whether you read or write. Reading S gets you what's in the "serial input register" (SIR), and writing S puts a byte into the "serial output register" (SOR).

### P

P stands for parallel. And what has been said for S applies verbatim to P, if you change the letter S for a P: SIR becomes PIR, SOR becomes POR, and so on.

So now that we have finished explaining the source registers and effects, let's look at the targets, the second letters that occur in each pair instruction. As has been said, the 8 sources we have explained in the last section - except F! - can also be used as target.
We will concentrate now on the 9 remaining targets: C, E, K, U, W, H, Z, N. These can't occur as sources!

### _C

Target "C" is another effect register, like A. It acts like a register and stores a value, but also has a powerful effect. C stands for "call". When you write a value to C, it is understood that this value is the high-byte of a memory address where a subroutine is stored. A subroutine is a program that your code jumps to, and which jumps back to your own program at the exact place where you left off, once it's done.
It can do this because it remembers the memory address in your program it needs to return to. The "C" target effect helps with that: it stores the high-byte of where the subroutine needs to return to in B (the base) and the low-byte in O (the offset). Then it "calls" the subroutine. Subroutines in this computer always start at the beginning of a page of memory, where the offset is zero. Therefore, when you write a value into C, the program counter (remember: it provides the offset of the next instruction in memory) is set to zero, and the C register is set to the value that is copied from the source of the pair instruction. This has the effect that the next instruction will be the first of the subroutine.
So to resume: when you write a value into the C effect register, the current value of C is copied into B, and PC is copied into O. This is so that the subroutine can return to you. Then comes the actual call: The source value is copied into C, and PC is set to zero.

### _E

"E", which stands for "enable" is another effect register. It stores a value, but it also has an action when you write to it. The bit pattern in the source byte selects or deselects hardware that is connected to your computer. This is explained in the section on input/output. Don't write to E if you don't understand yet how it works, just because the hardware may do strange things as a result. But don't worry, it isn't that complicated!

### _K

The pair target "K", for "key", is purely an effect, but it's tied to the K storage register via one of the infamous "scrounge" instructions we mentioned, aptly called KEY. The KEY instruction copies B into K.
But we are here to discuss the target effect "K". Its action is: Copy K into B, and set O to the source value. So it sets the memory pointer B:O to the address of a particular byte in page K - a shortcut you can use for accessing a table of variables in a less verbose way.

### _U

The "U" effect (update) also has an action that involves  the B:O pointer. It treats B:O as a 16-bit number and adds the source value to it as a signed number. So essentially it allows you to add or subtract a constant to or from B:O.

The remaining four targets are all effects: W, H, Z and N. All of them are about jumping to a new offset within the memory page you are currently in.

### _W

Perhaps the most interesting is W (while). As we mentioned, W works together with D (down-counter) register. When you write a value to W, it is understood that the source value is an address offset in the current program page, a new value that you want to set PC to. Now, the action of W is the following: If D is not zero, set PC to the source value (jump to this location within the page). Otherwise just continue what you are doing. Then, however it turned out, decrement D by 1.
So you can see that this is essentially a while loop, with a dedicated register.

### _H

The action of the H effect is similar to W, but PC is set to the source value and does the jump, if the accumulator register A is not zero. Otherwise we just continue.
The letter H stands for "hot". That's a term used in electronics for when a data word has at least one bit that is not zero.
On other computers the action of this instruction would be called "branch if not zero".

### _Z

The action of the Z effect is almost identical, but this time we jump if A **is** in fact zero.

### _N

Then, finally, the N effect. N stands for negative, so we will jump if the value in A is negative. A byte is negative, by a very practical convention called the two's complement, if its highest order bit (bit 7 if count from zero) is one.
So this action can be used for checking that bit, too.

Now you know what all the pair instructions do! They make up half of the entire instruction set of this computer, there are just so many combinations - exactly 128, including the "scrounges".












