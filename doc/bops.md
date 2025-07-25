### BOPs

I'm sure that you're asking yourself what these might be.

This group of 8 instructions is probably the easiest one to understand. BOP stands for "B:O Pointer". As you know by now, the B:O register pair has a central place as the only address register in this computer.

In order to use it, almost invariably, you must copy suitable values into B and into O, before you can read or write memory.

In order to alleviate this repetitive task, there are four "wide" registers, into which you can save and from which you can restore the B:O register pair in one go.

These registers are the four "amenity" or BOP registers P1, P2, P3, and IA.

And all the BOP instructions do is the saving and restoring between B:O and these pointers:

**P1BO**: copy P1 into BO.
**BOP1**: copy BO into P1.

 Same for P2BO and BOP2, P3BO and BOP3, ... and IABO and BOIA.
 
 Except that IA has a dedicated job, and you must only use it for this purpose or your program can "crash". IA stores the return address during trap calls and interrupts, and the RTI instruction relies on this address to be there.

