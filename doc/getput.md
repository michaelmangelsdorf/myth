### Getput

Getput instructions are the second largest group of instructions, after pair instructions. There are 64 of them, and just like pairs, there exist only that many because there are many combinations of the same idea.

#### Local Variables

To understand what getput instructions do, you need to know what the L register is for, and what local variables are. L stands for local, and it contains a page number. Whenever you call a subroutine, that number is decremented, and whenever you return from a subroutine, the number increments.

So when you call a subroutine, it gets its own page, and when it returns, you get the same page number back that you had before the call.

This mechanism allows you to store "your stuff" into your page, and the subroutine can store "its stuff" in its page. This is where the name local comes from, it simply means "local" to a specific subroutine. The advantage of this, and this may not be obvious!, is that when the subroutine returns, your own local variables are guaranteed to be just as before.

#### L1 to L8

The 8 very last bytes at the end of a local page are special: They are called L1 to L8, and they are what getput instructions operate on. So L8 corresponds to the last offset in page L, at offset FFh. Then L7 is at FEh, and so on.

These instructions allow you to load (get) the registers B, O, A, or D from one of these 8 memory locations, or to store (put) these registers there. 

The syntax is very simple, you combine the number of the location (1 to 8) with the name of one of the four registers. If the number comes first (1a - L1 into A) it means you want do store the memory variable into the register. If the register comes first (a1 - A into L1) then you're storing the register into the memory variable.

#### Instruction LOCAL and L0

Time to disclose three more of the infamous "scrounge" instructions. Let's start with LOCAL. This instruction sets the B register to L, and the offset to point to the memory cell just below L1. You guessed it, that memory cell is called L0.

So if you want to quickly store away A somewhere, you can say: "LOCAL AM", and this will copy A to L0. If you are unsure how AM works, read the section on pair instructions. In a nutshell: A_ means "take A as a source", so M is the target. M is an "effect", the storage effect. When you write to it, it stores the value at address B:O (the register pair is set by LOCAL, as we said) in memory.

Note that you can obtain the page number stored in L by saying LOCAL and inspecting B.

#### ENTER and LEAVE

The other two scrounge instructions that belong in this section are ENTER (decrement L), and LEAVE (increment L). The words evoke entering a new local page (like a subroutine does), and leaving the page again (when the subroutine returns).

The L register cannot be set directly to a specific page, but if you really, really want to, you can say "LOCAL BD (puts L into D to know what page it is and then somehow calculate how far it is to where you want it), and then say "ENTER" or "LEAVE" to wind L to the page you need.

If you just say LEAVE inside a subroutine, your local variables will switch to those of the calling subroutine, be careful.

But if you need **more** local variable, you say ENTER in your subroutine, and get another page, with another set of L0 to L8 (but your previous ones will not be availabe unless you switch back to them doing EXIT).

Now you can see why there are 64 getput instructions: 8 memory cells (L0-L8), 2 directions (get and put), and 4 registers (B, O, A, D). 8x2x4 combinations = 64.



