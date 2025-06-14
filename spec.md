## Myth CPU/Micro-Controller
## OVERVIEW

Myth is a spinoff with minor changes of an earlier micro-controller built successfully using just under 100 74HC series chips, some CMOS memory and passive components.

Myth is an educational 8-bit toy CPU with a reduced, but hopefully enjoyable feature set. It has no microcoded complex instructions, but care has been taken to allow for practicable programming that should be intuitive to the assembler programmer.

Published schematics and documentation of the earlier, working prototype:
https://github.com/michaelmangelsdorf/Sonne8

## PART 1

### Computations

The two registers A and X are called the accumulator. A holds the primary result of ALU (Arithmetic Logic Unit) operations. X holds the secondary result. When writing a value into A (instructions xA and GETA), the accumulator functions as a two-element push-down stack: The old value of A is saved into X before overwriting it with the new value.

The ALU can run the following opcodes:

     0 NOT   Set A to one's complement of A, X unchanged
     1 ALX   Flag (A<X) in A (255 if true, 0 if false), X unchanged
     2 AEX   Flag (A==X) in A (255 if true, 0 if false), X unchanged
     3 AGX   Flag (A>X) in A (255 if true, 0 if false), X unchanged
     4 AND   Set A to (A AND X), X unchanged
     5 IOR   Set A to (A OR X), X unchanged
     6 EOR   Set A to (A XOR X), X unchanged
     7 XA    Set A equal to X, X unchanged
     8 AX    Set X equal to A
     9 SWAP  Swap A and X
    10 SHL   Shift A left, result in A, set X to previous MSB of A as LSB (0 or 1)
    11 SHR   Shift A right logically, result in A, set X to previous LSB of A as MSB (0 or 80h)
    12 ASR   Shift A right arithmetically, set X to previous LSB of A as MSB (0 or 80h)
    13 ADDC  Add A to X, result in A, CARRY bit in X (0 or 1)
    14 ADDV  Add A to X, result in A, OVERFLOW flag in X (255 if OVF, else 0)
    15 SUBB  Subtract A from X, result in A, BORROW bit in X (0 or 1)

In C, this presents as follows:
    
    /* Execute ALU instruction
    */
    void
    alu( uint8_t opcode)
    {
            int i;
            uint8_t a0 = a;
            uint8_t ovf = ((a&0x80)^(x&0x80))==0    /* Addends have same sign */
                            && ((i&0x80)^(a&0x80)); /* But result has different sign */
            
            switch(opcode & 15){
                    case NOT: a = ~a;                                   break;
                    case ALX: a = (a<x)  ? 255:0;                       break;
                    case AEX: a = (a==x) ? 255:0;                       break;
                    case AGX: a = (a>x)  ? 255:0;                       break;
                    case AND: a = a & x;                                break;
                    case IOR: a = a | x;                                break;
                    case EOR: a = a ^ x;                                break;
                    case XA: a=x;                                       break;
                    case AX: x=a;                                       break;
                    case SWAP: a=x; x=a0;                               break;
                    case SHL: a<<=1; x=(a0 & 0x80)? 1:0;                break;
                    case SHR: a>>=1; x=(a0 & 1)? 0x80:0;                break;
                    case ASR: a=(a>>1)+(a0 & 0x80); x=(a0 & 1)? 0x80:0; break;
                    case ADDC: i = x+a; a=i&0xFF; x=(i>255)? 1:0;       break;
                    case ADDV: i = x+a; a=i&0xFF; x=ovf?255:0;          break;
                    case SUBB: i = x-a; x=(i<0)? 0:1;                   break;
            }
    }

#### Memory Layout

The CPU has access to 64 kilobytes of RAM. This memory is organized into 256 pages of 256 bytes.

A memory address is composed of a page index (high order address) and a byte offset (low order address) within that page. For example address 0x6502 has a page index of 0x65 and a byte offset of 0x02.

##### Offset Registers

Without exception, the byte offset for data memory access is stored in O (offset), and the byte offset for fetching instructions and literals is stored in the program counter register (PC).

When reading memory data, the O register must be set to a suitable page-offset value at any time, except when using GETPUT instructions or fetching a literal placed in the code.

##### Page-Index Registers

There are four dedicated page-index registers: B for Base, C for Code, R for Resident, and L for Local.

###### B

Without exception, the B register (base) is used together with O (offset) as the memory pointer to the address where read or write operations occur.

When reading memory data, the B:O register pair must be set to a suitable memory pointer, except when using GETPUT instructions or fetching a literal placed in the code.

The 16-bit value B:O is called the Base Pointer.

###### C and G

When code is running, the byte offset of the current instruction is invariably stored in the program counter register (PC). The page number where this offset applies, however, is stored in either C (Code) or G (Guest). Every page has a "guest" page, the upper half of which appears at the upper half (offset 128) of the (host) page. A page can be its own guest: initially, and after each subroutine or coroutine call or return instruction, the guest page index stored in G is set to the current code page index stored in C. Thus the guest-page setting is volatile.

As has been said, the guest page appears at offset 128. Hence the most significant bit of the program counter value determines, whether an instruction's page-index is read from C or from G: If the MSB of PC is set (offset of the instruction byte in the page is 128 or higher), the page-index in G is used. Otherwise (the offset of the instruction byte is below 128), the page-index in C is used.

Conceptually, this creates two independent code segments, a lower segment (offsets < 128) and an upper (guest) segment. The dedicated instruction "xG" sets G to a given page number and jumps to offset 128. The page-index in G gets reset to C during interpage jumps: Subroutine and coroutine calls or return instructions set G equal to C again.

Note that the __lower__ half of the page whose index number is stored in G, and which thus provides the upper code segment, is not available to the running code, if G is different from C. The reason for this is that when an intrapage jump to an address below 128 (the lower segment) occurs, bit 7 of the program counter toggles to zero, enabling the page-index in C to take over: Any instruction fetch in the lower segment occurs at C:PC, not at G:PC.

###### L

The purpose of the L (local) page index register is to provide single page stack frames for subroutines. During subroutine calls, the page index in L is decremented, so that memory reads and writes using the L page index transparently access memory which is local to the currently running subroutine. When the subroutine returns, the page index in L is incremented, so that the previous stack frame (or L page) is restored to the calling subroutine.

#### GETPUT Instructions and Stack Frames

Instructions of type GETPUT complement the behaviour explained in the section on L. GETPUT instructions have mnemonics that consist of a digit and a letter. The letter names one of the allowed registers for this type of instruction: B, O, A, or D. The letter is a digit between 1 and 8, used as a short-hand to reference offsets 0xF8 to 0xFF in the local page (page index L). These memory locations are also referred to as L1 to L8. By means of the GETPUT instructions, L1 to L8 become quick-access memory locations that can be used as local variables within the current stack frame.

The position of the digit relative to the letter determines the direction of the data move. For example: 1a transfers the memory content at offset 0xF8 into register A. And d5 transfers register D (down-counter) into memory at offset 0xFC of the local page.

The local-page index can be manually decremented using ENTER, and manually incremented using LEAVE.


#### PAIR Instructions and Effect Registers

Data move instructions of type PAIR have mnemonics that consist of two letters, one for the source, followed by one for the target of the data move. For example, in order to write register B into A, there is an instruction with the mnemonic "ba".

Some registers are "conventional" data registers, such as B and A, which correspond to physical registers, but there are also EFFECT registers, which source or distribute data indirectly, or trigger conditional actions. One of them is the F register (fetch). It extracts the next byte in the instruction stream, and then skips to the next instruction.

Example: "fa" fetches the byte following the current instruction in memory, and stores it into A. It then increments the program counter by 1 so that the next instruction is fetched instead of the literal.


#### Intrapage Control Flow and D Register

Branching inside of the current code page (page index C or R) is controlled by writing into the five effect registers J (Jump), W (While), H (Hot/Not zero), Z (Zero), or N (Negative).

By writing a branch offset into J, the program counter is set to this new offset without condition. Writing to H loads the PC with the new offset only if register A (Accumulator) is non-zero ("hot"). Writing to Z loads PC with the new offset only if A is zero. W (while) works in conjunction with the D (down-counter) register; when a branch offset is written into W, PC is loaded with the new offset only while/if the D register is non-zero. Then, in either case, the D register is decremented.

#### Interpage Control Flow and TRAP Instructions

Branching to code in a different page is done by writing into one of the effect registers C (call) or R (resident), by executing a return instruction (RTS or RTI), the COR (coroutine) instruction, or executing a TRAP instruction.

TRAP instructions have immediate opcodes that encode a target page-index for a call. When executed, an implicit subroutine call to this encoded page-index occurs within a single instruction. A TRAP to page 0 sets the BUSY flag and disables interrupts. The BUSY is cleared by executing RTI.

When executing RTS or RTI, the code page index register C is loaded with the value in B (base), the program counter register PC is loaded with the value of register O (offset), and the local-page index in register L is incremented.

While executing a trap or when writing a target page-index into the effect register C (call), the program counter is set to 0 (!) and the target page index is loaded into C, so that execution in the target page always starts at offset zero, the page "head".

When writing a target page index into C (call), the previous value of C is first saved into B (base), and the value of PC is saved into the O (offset) register before PC is set to zero, so that the called subroutine can save this information in order to return to the caller.

When writing R to run a resident routine in the upper segment, care must be taken so that either the current instruction is in the lower segment, or that the mapped resident routine contains the intended code at the current position of PC, since the switch is immediate.

#### PAIR Memory access - Effect Register M

Writing into M (xM) stores the value into memory at page index B offset O. Conversely, reading from M (Mx) transfers the value stored into that memory cell into the target of the PAIR instruction. There are no other memory transfer instructions besides xMx, the GETPUT instructions, and Fx.

#### Scrounged PAIR opcodes

Inherent NOP instructions such as BB, OO, AA, and EE, and impractical instructions such as FM and MM (same-cycle memory load-store) are repurposed ("scrounged"), and their respective opcodes execute different instructions.

	FM routed to: CODE (set B:O to C:PC or R:PC, depending on PC bit 7)
	MM routed to: LOCAL (set B:O to L:0xF7 - "L0")
	BB routed to: LEAVE (increment L)
	OO routed to: ENTER (decrement L)
	AA routed to: INC (increment A)
	EE routed to: DEC (decrement A)


#### B:O Pointer Registers (BOPs)

As mentioned, registers B (base) and O (offset) form a 16-bit pointer for memory access. The xU (update) instruction is used to add an 8-bit signed number to this pointer for doing address arithmetic.

There are four 16-bit registers into which the B:O pointer can be saved, or from which it can be loaded in a single instruction.

The K pointer ("Key") can be used as a set of two scratch registers, for passing arguments during function calls for instance.

Q should be used as the global stack pointer. I and T should be used to implement threaded code, or other system-internal pointers. Restore Q, I and T in your subroutines after using them for different purposes.

	KBO Load R into BO
	BOK Save BO into K
	
	QBO Load Q into BO
	BOQ Save BO into Q
	
	IBO Load I into BO
	BOI Save BO into I

	TBO Load T into BO
	BOT Save BO into T

## PART 2

### External Communication (I/O)

#### Device Selection

The CPU is designed to handle serial and parallel communication with external components. This is facilitated by dedicated hardware-registers and instructions.

##### E (Enable) register

The 8-bit E register is used to control the select state of devices attached to the serial or parallel bus lines. To this end, the register is divided into two independent four-bit groups for device selection.

Each four-bit group (L for the low-order, H for the high-order) drives a 4-to-16 line decoder, which maps the bit pattern encoded by that group to 1 of 16 possible, mutually exclusive select signals (SL0-15 and SH0-15) per group.

###### Special Purpose Selectors

Select signals SL0 and SH0 are reserved to select a NULL device ("nothing").

SL1 corresponds to the internal POR register output enable signal (POE). SH1 corresponds to the internal PIR register latch enable signal (PLE).

In order to latch the current value of the GPIO bus into the PIR, the PLE signal must be set by the high-order nybble of E. Selecting the POE signal by the low-order nybble of E enables the output of the POR register onto the GPIO bus.

All remaining selectors can be used freely.

#### Communication Registers

SOR (Serial Output Register)

A write-only parallel-to-serial shift register for serialising an output byte, modelled after a 74HC165 chip. Writing an output value for serialisation is done by writing the value into register S.

This value is clocked out/serialized by pulsing the SCLK clock line. This is achieved by alternating SCL and SCH instructions (set clock low/high).

SCL-SCH-SCL generates a positive clock edge. SCH-SCL-SCH generates an inverted clock. Eight clock cycles are required to send-out a byte.

SIR (Serial Input Register)

A read-only serial-to-parallel shift register for de-serialising an incoming bit stream into an input byte, modelled after a 74HC595 chip.

Receiving a byte is done by executing the SCL/SCH instructions eight times as explained above. Reading the deserialised input byte is done by reading register S.

POR (Parallel Output Register)

A tri-state register with 8-bit parallel output, modelled after a 74HC574 chip. Writing an output byte onto the parallel bus is a two step process. First, the data byte must be latched into the register by writing it into P. Then, the register output must be enabled by selecting POE in the E register, as described above.

PIR (Parallel Input Register)

A read-only 8-bit parallel input register, modelled after a 74HC574 chip. Latching the current 8-bit value of the parallel bus into the register is done by selecting PLE in E. The latched data byte can then be read from P. The bus operates in weak pull-down mode, so when all bus-devices are in tri-state mode, a zero value is registered.

#### Communication Instructions

#### SERIAL

The following instructions contained in the SYS group operate on the communication registers:

SSI (Shift Serial In)

This instruction receives a serial bit via the serial input line. It then shifts SIR left and sets its least significant bit (LSB) to the received bit state.

SSO (Shift Serial Out)

This instruction outputs the most significant bit (MSB) of SOR onto the serial output line and then shifts SOR left.

SCH (Serial clock high)

This instruction sets the clock line to HIGH.

SCL (Serial clock low)

This instruction sets the clock line to LOW.

#### PARALLEL

The CPU interfaces to an external bidirectional 8-bit wide bus (GPIO bus).

It can communicate on this bus by writing a data byte into P (POR register), and then enabling POE in the E register by setting its lower nybble to 1. Setting the bit to 1 switches the POR from tri-state output to active output, so that the byte value is output on the bus lines.

While the output is active, other devices on the bus can read the data byte. Usually, such a device will be controlled or synchronised by the Myth controller. It does this by enabling or disabling latches or outputs of the required device in E as explained above. This generates output signals made available to external devices on the micro-controller pins. Exactly two output signals (one per nybble in E) can be active at the same time.

Deselecting POE in E again (setting the low-order nybble to a value different from 1) tristates the POR output, so that other devices can put data bytes on the GPIO bus.

Enabling PLE in E (setting the high-order nybble to 1) latches a data byte into the PIR. This byte can then be read from the P register.

Once a data byte has been read, the PIR input should be deselected again in E by setting the high-order nybble to a value different from 1.

### Serial Communication

The Serial Peripheral Interface (SPI) protocol can be implemented using the device enable register E, serial registers SIR and SOR, and instructions SCL, SCH, SSI, and SSO.

#### Device Selection

Before communicating with a specific device connected to the serial bus, the corresponding se<lector bit representing the device must be set in the E register.

#### Data Transmission

To transmit data to the selected device, the processor writes a data byte (8 bits) to be serialised for output into the SOR (Serial Output) register.

The SSO (Serial Shift Out) instruction is then used, which clocks the serial output shift register and produces a data bit on the MOSI line. Using the instruction sequence SCL SCH SCL (Serial Clock Low/High), a positive edge clock pulse is generated.

As each bit is shifted out, it is sent to the selected device through the serial bus. The passive device processes the transmitted bit and the cycle repeats.

#### Data Reception

To receive data from an external device, the SSI (Serial Shift In) instruction is used. It clocks the serial input shift register, allowing the processor to receive one bit of data at a time from the selected device via the MISO line. The received data can then be read from the S register. Clocking is done as above.

##### CPOL (Clock Polarity)

The CPOL parameter determines the idle state of the clock signal. The controller provides signals SCL (Serial Clock Low) and SCH (Serial Clock High) instructions which can be used to control the clock signal's state.

To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during the idle state. To configure CPOL=1 (clock idles high), execute SCH to set the clock signal high during the idle state.

##### CPHA (Clock Phase)

The CPHA parameter determines the edge of the clock signal where data is captured or changed. The Myth controller provides instructions SSI (Serial shift in) and SSO (Serial shift out) to control data transfer on each clock transition.

To configure CPHA=0 (data captured on the leading edge), execute SSI before the clock transition to capture the incoming data. To configure CPHA=1 (data captured on the trailing edge), execute SSI after the clock transition to capture the incoming data.

Similarly, to transmit data on the leading or trailing edge, execute SSO before or after the clock transition, respectively.

#### Device Deselection

After data transmission is complete, the selected device needs to be deselected to allow other devices to communicate on the bus. This is done by updating the E register with the appropriate value.


### Interrupts

An external device can make an interrupt request (IRQ) by asserting the IRQ signal.

At the beginning of each instruction cycle, the CPU checks whether an Interrupt must be serviced. There are two conditions which prevent an interrupt from being serviced by the microcontroller during a given instruction cycle. Firstly, when the CPU is running code within page 0, for example just after RESET, and secondly when the BUSY flag is set.

If the BUSY flag is not set, and the page index in C is not zero, the CPU injects a "fake" TRAP call instruction to page 0, instead of fetching a proper instruction opcode. By entering page 0, the BUSY flag is set and the interrupt service routine in page zero at address-offset 0 is run. Note: The xG (Guest) instruction can be used to map code into the upper segment as explained; as long as the page-index in C remains zero, interrupts are disabled.

To re-enable interrupts, the software must execute an RTI instruction (Return from Interrupt). RTI behaves identically to RTS (Return from Subroutine), but clears the BUSY flag. As long as the BUSY flag remains set, downstream service routines or other code will not be interrupted by interrupts.

The interrupt service subroutine once it returns resumes execution at the point in code where the interrupt occurred.

You can manually set BUSY by executing TRAP 0. Other calls to page 0 do not have this side effect.

## Opcode Format

Operation codes fall into 6 format groups, which are decodable using a priority encoder. 

                      -- Opcode Bits --
                         MSB     LSB
    all 0: OPC_SYS       00000   xxx    See table @ SYS decoder
     else: OPC_BOP       00001   xxx    See table @ BOP decoder
           OPC_ALU       0001   xxxx    See table @ ALU
           OPC_TRAP      001   xxxxx    b0-4: DESTPAGE
           OPC_GETPUT    01 xx x xxx    b0-2: OFFS, b3: GET/PUT, b4-5: REG
           OPC_PAIR      1  xxx xxxx    b0-3: DST, b4-6: SRC

## Assembler

### Casing and Phrasing

Instruction mnemonics are not case-sensitive. Using upper case can provide emphasis or visual structure.

Multiple assembler mnemonics can occur on a single line. Commas (",") and dots (".") and dashes ("-") can optionally be used for grouping "phrases" of instructions that logically belong together. These symbol can be attached to instruction mnemonics without any effect.

By default, put Fx (literal instructions) at the beginning of a new line. Also do this for TRAP instructions (*xyz), if they consume code bytes.

### Comments

Place comments into parentheses. A semicolon (";") works as a line comment.

### Number Literals

Decimal numbers from 0-255 can be included in the source text as literals, and be prefixed by an optional minus sign. Hexadecimal numbers must be in two uppercase digits and marked with the suffix "h", for instance: "80h" for 128.            Binary numbers can either be formatted as four numeric digits with the suffix "b" (1111b = 15), or in the following way, as two 4-bit groups separated by an underscore ("_") with the suffix "b", for instance: "0010_0000b" for 32.

### Strings and Characters

Characters enclosed in double quotation marks such as "Hello World" are assembled to ASCII strings of characters.

### Labels

#### Page-Index Labels

Use all UPPER "snake-case" for labels that reference page-indices. Add a colon when defining these labels.

A TRAP page label definition must begin with an asterisk and end with a colon. Writing the label without the colon encodes a trap instruction to the defined page-index.

 Multiple labels can be given to the same page-index. A given page can hold a TRAP handler (offset 0), a Guest routine (offset 80h),
and various other code and data structures that are unrelated to each other. Add a label with the prefix g for pages that contain guest-code for easy
reference by the caller.

    ; Example skeleton for a TRAP page with other, unrelated named parts
    *ITSA_TRAP:
        (Service routine here, can be invoked using *ITSA_TRAP)
        ...
        
    gGUEST_CODE:
    @70h:myroutine
        ...
        
    SOME_DATA_TABLE: 1 2 4 8 16 32 64 128.
    
Once the assembler reaches a page label, the remaining bytes of the current page are padded with 0 (NOP), and the object-code pointer is set to offset 0 of the new page.

#### Offset Labels

Prefix offset labels by @. A number literal followed by a colon can occur between the at-sign and the label text to set a the object code origin. If the number literal specifies an offset __after__ the previous offset, the object code bytes that are skipped are padded with zeros (NOP).

### Label References

Labels are referenced by prefixing their identifier with < for backward references (the intended label is defined earlier in the source code than the reference to it), or with > for forward references (the intended label is defined after the reference to it in the source code).

If a label is not unique, the reference goes to the nearest occurrence of it in the given direction. A label reference is just a numerical value and can be used as a literal anywhere in the code.

## Systems Programming

The following are recommendations or useful conventions.

### Identifiers



### Parameter passing

Use the accumulator (AX) for primary arguments in general.

When writing subroutines, use local variables (L8-L3) with higher numbers first, ideally reserving L1, L2 for passing parameters to Guest functions (xG instruction). Guest functions should use L1 and L2 as scratch memory and document this, specifically.

There is a "hidden" local variable shortcut "L0". You can obtain a pointer to this memory location by executing the instruction "LOCAL". Local sets B to the Local page, and O to F7h, the byte offset just below L1. Then use MxM instructions such as "am" to read or store into the L0 variable:

    ; Store number 4 in L0:
    fa 2 shl, local am.

Pointer K (kbo, bok) is called the "key" and you can use it to pass either a 16-bit pointer, or individual values in B and O, to a subroutine.

## Tables

### Opcode Matrix

           x0    x1    x2    x3    x4    x5    x6    x7    x8    x9    xA    xB    xC    xD    xE    xF
    0x    NOP   SSI   SSO   SCL   SCH   RTS   RTI   COR   KBO   BOK   QBO   BOQ   IBO   BOI   TBO   BOT
    1x    NOT   ALX   AEX   AGX   AND   IOR   EOR    XA    AX  SWAP   SHL   SHR   ASR  ADDC  ADDV  SUBB
    2x     *0    *1    *2    *3    *4    *5    *6    *7    *8    *9   *10   *11   *12   *13   *14   *15
    3x    *16   *17   *18   *19   *20   *21   *22   *23   *24   *25   *26   *27   *28   *29   *30   *31
    4x     1b    2b    3b    4b    5b    6b    7b    8b    b1    b2    b3    b4    b5    b6    b7    b8
    5x     1o    2o    3o    4o    5o    6o    7o    8o    o1    o2    o3    o4    o5    o6    o7    o8
    6x     1a    2a    3a    4a    5a    6a    7a    8a    a1    a2    a3    a4    a5    a6    a7    a8
    7x     1d    2d    3d    4d    5d    6d    7d    8d    d1    d2    d3    d4    d5    d6    d7    d8
    8x     fu  CODE    fb    fo    fa    fe    fs    fp    fd    fw    fj    fh    fz    fn    fg    fc
    9x     mu LOCAL    mb    mo    ma    me    ms    mp    md    mw    mj    mh    mz    mn    mg    mc
    Ax     bu    bm LEAVE    bo    ba    be    bs    bp    bd    bw    bj    bh    bz    bn    bg    bc
    Bx     ou    om    ob ENTER    oa    oe    os    op    od    ow    oj    oh    oz    on    og    oc
    Cx     au    am    ab    ao   INC    ae    as    ap    ad    aw    aj    ah    az    an    ag    ac
    Dx     eu    em    eb    eo    ea   DEC    es    ep    ed    ew    ej    eh    ez    en    eg    ec
    Ex     su    sm    sb    so    sa    se    ss    sp    sd    sw    sj    sh    sz    sn    sg    sc
    Fx     pu    pm    pb    po    pa    pe    ps    pp    pd    pw    pj    ph    pz    pn    pg    pc

### Opcode Descriptions

    Group SYS
    
    0x00: NOP	Pass the turn (no operation)
    0x01: SSI	Shift serial bit in
    0x02: SSO	Shift serial bit out
    0x03: SCL	Set serial clock low
    0x04: SCH	Set serial clock high
    0x05: RTS	Return from subroutine
    0x06: RTI	Return from interrupt
    0x07: COR	Set C to B. Set PC to O. Save return pointer into B_O
    
    Group BOP
    
    0x08: KBO	Copy pointer K into BO
    0x09: BOK	Copy BO into pointer K
    
    0x0A: QBO	Copy pointer Q into BO
    0x0B: BOQ	Copy B_ into pointer Q
    
    0x0C: IBO	Copy pointer I into BO
    0x0D: BOI	Copy BO into pointer I
    
    0x0E: TBO	Copy pointer T into BO
    0x0F: BOT	Copy BO into pointer T
    
    Group ALU
    
    0x10: NOT	Set A to one's complement of A , X unchanged
    0x11: ALX	Flag (A<X) in A (255 if true, 0 if false), X unchanged
    0x12: AEX	Flag (A==X) in A (255 if true, 0 if false), X unchanged
    0x13: AGX	Flag (A>X) in A (255 if true, 0 if false), X unchanged
    0x14: AND	Set A to (A AND X), X unchanged
    0x15: IOR	Set A to (A OR X), X unchanged
    0x16: EOR	Set A to (A XOR X), X unchanged
    0x17: XA	Set A equal to X, X unchanged
    0x18: AX	Set X equal to A
    0x19: SWAP	Swap A and X
    0x1A: SHL	Shift A left, result in A, set X to previous MSB of A as LSB (0 or 1)
    0x1B: SHR	Shift A right logically, result in A, set X to previous LSB of A as MSB (0 or 80h)
    0x1C: ASR	Shift A right arithmetically, set X to previous LSB of A as MSB (0 or 80h)
    0x1D: ADD	Add A to X, result in A, CARRY bit in X (0 or 1)
    0x1E: OVF	Add A to X, result in A, OVERFLOW flag in X (255 if OVF, else 0)
    0x1F: SUB	Subtract A from X, result in A, BORROW bit in X (0 or 1)
    
    Group TRAP
    
    0x20: *0	Trap call to page 0, offset 0 - Set BUSY flag
    0x21: *1	Trap call to page 1, offset 0
    0x22: *2	Trap call to page 2, offset 0
    0x23: *3	Trap call to page 3, offset 0
    0x24: *4	Trap call to page 4, offset 0
    0x25: *5	Trap call to page 5, offset 0
    0x26: *6	Trap call to page 6, offset 0
    0x27: *7	Trap call to page 7, offset 0
    0x28: *8	Trap call to page 8, offset 0
    0x29: *9	Trap call to page 9, offset 0
    0x2A: *10	Trap call to page 10, offset 0
    0x2B: *11	Trap call to page 11, offset 0
    0x2C: *12	Trap call to page 12, offset 0
    0x2D: *13	Trap call to page 13, offset 0
    0x2E: *14	Trap call to page 14, offset 0
    0x2F: *15	Trap call to page 15, offset 0
    0x30: *16	Trap call to page 16, offset 0
    0x31: *17	Trap call to page 17, offset 0
    0x32: *18	Trap call to page 18, offset 0
    0x33: *19	Trap call to page 19, offset 0
    0x34: *20	Trap call to page 20, offset 0
    0x35: *21	Trap call to page 21, offset 0
    0x36: *22	Trap call to page 22, offset 0
    0x37: *23	Trap call to page 23, offset 0
    0x38: *24	Trap call to page 24, offset 0
    0x39: *25	Trap call to page 25, offset 0
    0x3A: *26	Trap call to page 26, offset 0
    0x3B: *27	Trap call to page 27, offset 0
    0x3C: *28	Trap call to page 28, offset 0
    0x3D: *29	Trap call to page 29, offset 0
    0x3E: *30	Trap call to page 30, offset 0
    0x3F: *31	Trap call to page 31, offset 0
    
    Group GETPUT
    
    0x40: 1b	Load B from L1 (M[L_F8h])
    0x41: 2b	Load B from L2 (M[L_F9h])
    0x42: 3b	Load B from L3 (M[L_FAh])
    0x43: 4b	Load B from L4 (M[L_FBh])
    0x44: 5b	Load B from L5 (M[L_FCh])
    0x45: 6b	Load B from L6 (M[L_FDh])
    0x46: 7b	Load B from L7 (M[L_FEh])
    0x47: 8b	Load B from L8 (M[L_FFh])
    
    0x48: b1	Store B into L1 (M[L_F8h])
    0x49: b2	Store B into L2 (M[L_F9h])
    0x4A: b3	Store B into L3 (M[L_FAh])
    0x4B: b4	Store B into L4 (M[L_FBh])
    0x4C: b5	Store B into L5 (M[L_FCh])
    0x4D: b6	Store B into L6 (M[L_FDh])
    0x4E: b7	Store B into L7 (M[L_FEh])
    0x4F: b8	Store B into L8 (M[L_FFh])
    
    0x50: 1o	Load O from L1 (M[L_F8h])
    0x51: 2o	Load O from L2 (M[L_F9h])
    0x52: 3o	Load O from L3 (M[L_FAh])
    0x53: 4o	Load O from L4 (M[L_FBh])
    0x54: 5o	Load O from L5 (M[L_FCh])
    0x55: 6o	Load O from L6 (M[L_FDh])
    0x56: 7o	Load O from L7 (M[L_FEh])
    0x57: 8o	Load O from L8 (M[L_FFh])
    
    0x58: o1	Store O into L1 (M[L_F8h])
    0x59: o2	Store O into L2 (M[L_F9h])
    0x5A: o3	Store O into L3 (M[L_FAh])
    0x5B: o4	Store O into L4 (M[L_FBh])
    0x5C: o5	Store O into L5 (M[L_FCh])
    0x5D: o6	Store O into L6 (M[L_FDh])
    0x5E: o7	Store O into L7 (M[L_FEh])
    0x5F: o8	Store O into L8 (M[L_FFh])
    
    0x60: 1a	Load A from L1 (M[L_F8h])
    0x61: 2a	Load A from L2 (M[L_F9h])
    0x62: 3a	Load A from L3 (M[L_FAh])
    0x63: 4a	Load A from L4 (M[L_FBh])
    0x64: 5a	Load A from L5 (M[L_FCh])
    0x65: 6a	Load A from L6 (M[L_FDh])
    0x66: 7a	Load A from L7 (M[L_FEh])
    0x67: 8a	Load A from L8 (M[L_FFh])
    
    0x68: a1	Store A into L1 (M[L_F8h])
    0x69: a2	Store A into L2 (M[L_F9h])
    0x6A: a3	Store A into L3 (M[L_FAh])
    0x6B: a4	Store A into L4 (M[L_FBh])
    0x6C: a5	Store A into L5 (M[L_FCh])
    0x6D: a6	Store A into L6 (M[L_FDh])
    0x6E: a7	Store A into L7 (M[L_FEh])
    0x6F: a8	Store A into L8 (M[L_FFh])
    
    0x70: 1d	Load D from L1 (M[L_F8h])
    0x71: 2d	Load D from L2 (M[L_F9h])
    0x72: 3d	Load D from L3 (M[L_FAh])
    0x73: 4d	Load D from L4 (M[L_FBh])
    0x74: 5d	Load D from L5 (M[L_FCh])
    0x75: 6d	Load D from L6 (M[L_FDh])
    0x76: 7d	Load D from L7 (M[L_FEh])
    0x77: 8d	Load D from L8 (M[L_FFh])
    
    0x78: d1	Store D into L1 (M[L_F8h])
    0x79: d2	Store D into L2 (M[L_F9h])
    0x7A: d3	Store D into L3 (M[L_FAh])
    0x7B: d4	Store D into L4 (M[L_FBh])
    0x7C: d5	Store D into L5 (M[L_FCh])
    0x7D: d6	Store D into L6 (M[L_FDh])
    0x7E: d7	Store D into L7 (M[L_FEh])
    0x7F: d8	Store D into L8 (M[L_FFh])
    
    Group PAIR
    
    0x80: FU	Take M[CG_PC++] as 8-bit signed number and add it to 16-bit pointer B_O
    0x81: CODE	Set pointer B_O to CG_PC
    0x82: FB	Take M[CG_PC++] into B
    0x83: FO	Take M[CG_PC++] into O
    0x84: FA	Take M[CG_PC++] into A
    0x85: FE	Take M[CG_PC++] into E
    0x86: FS	Take M[CG_PC++] into SOR
    0x87: FP	Take M[CG_PC++] into POR
    0x88: FD	Take M[CG_PC++] into D
    0x89: FW	Take M[CG_PC++] as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0x8A: FJ	Take M[CG_PC++] as page offset and store it into PC - always
    0x8B: FH	Take M[CG_PC++] as page offset and store it into PC - if A is not equal to zero
    0x8C: FZ	Take M[CG_PC++] as page offset and store it into PC - if A is equal to zero
    0x8D: FN	Take M[CG_PC++] as page offset and store it into PC - if A is negative (has bit 7 set)
    0x8E: FG	Take M[CG_PC++] as page-index, load the index into G, set PC to 80h
    0x8F: FC	Take M[CG_PC++] as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0x90: MU	Take M[B_O] as 8-bit signed number and add it to 16-bit pointer B_O
    0x91: LOCAL	Set pointer B_O to L_F7h (L0)
    0x92: MB	Take M[B_O] into B
    0x93: MO	Take M[B_O] into O
    0x94: MA	Take M[B_O] into A
    0x95: ME	Take M[B_O] into E
    0x96: MS	Take M[B_O] into SOR
    0x97: MP	Take M[B_O] into POR
    0x98: MD	Take M[B_O] into D
    0x99: MW	Take M[B_O] as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0x9A: MJ	Take M[B_O] as page offset and store it into PC - always
    0x9B: MH	Take M[B_O] as page offset and store it into PC - if A is not equal to zero
    0x9C: MZ	Take M[B_O] as page offset and store it into PC - if A is equal to zero
    0x9D: MN	Take M[B_O] as page offset and store it into PC - if A is negative (has bit 7 set)
    0x9E: MG	Take M[B_O] as page-index, load the index into G, set PC to 80h
    0x9F: MC	Take M[B_O] as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xA0: BU	Take B as 8-bit signed number and add it to 16-bit pointer B_O
    0xA1: BM	Take B into M[B_O]
    0xA2: LEAVE	Increment L
    0xA3: BO	Take B into O
    0xA4: BA	Take B into A
    0xA5: BE	Take B into E
    0xA6: BS	Take B into SOR
    0xA7: BP	Take B into POR
    0xA8: BD	Take B into D
    0xA9: BW	Take B as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xAA: BJ	Take B as page offset and store it into PC - always
    0xAB: BH	Take B as page offset and store it into PC - if A is not equal to zero
    0xAC: BZ	Take B as page offset and store it into PC - if A is equal to zero
    0xAD: BN	Take B as page offset and store it into PC - if A is negative (has bit 7 set)
    0xAE: BG	Take B as page-index, load the index into G, set PC to 80h
    0xAF: BC	Take B as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xB0: OU	Take O as 8-bit signed number and add it to 16-bit pointer B_O
    0xB1: OM	Take O into M[B_O]
    0xB2: OB	Take O into B
    0xB3: ENTER	Decrement L
    0xB4: OA	Take O into A
    0xB5: OE	Take O into E
    0xB6: OS	Take O into SOR
    0xB7: OP	Take O into POR
    0xB8: OD	Take O into D
    0xB9: OW	Take O as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xBA: OJ	Take O as page offset and store it into PC - always
    0xBB: OH	Take O as page offset and store it into PC - if A is not equal to zero
    0xBC: OZ	Take O as page offset and store it into PC - if A is equal to zero
    0xBD: ON	Take O as page offset and store it into PC - if A is negative (has bit 7 set)
    0xBE: OG	Take O as page-index, load the index into G, set PC to 80h
    0xBF: OC	Take O as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xC0: AU	Take A as 8-bit signed number and add it to 16-bit pointer B_O
    0xC1: AM	Take A into M[B_O]
    0xC2: AB	Take A into B
    0xC3: AO	Take A into O
    0xC4: INC	Increment A
    0xC5: AE	Take A into E
    0xC6: AS	Take A into SOR
    0xC7: AP	Take A into POR
    0xC8: AD	Take A into D
    0xC9: AW	Take A as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xCA: AJ	Take A as page offset and store it into PC - always
    0xCB: AH	Take A as page offset and store it into PC - if A is not equal to zero
    0xCC: AZ	Take A as page offset and store it into PC - if A is equal to zero
    0xCD: AN	Take A as page offset and store it into PC - if A is negative (has bit 7 set)
    0xCE: AG	Take A as page-index, load the index into G, set PC to 80h
    0xCF: AC	Take A as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xD0: EU	Take E as 8-bit signed number and add it to 16-bit pointer B_O
    0xD1: EM	Take E into M[B_O]
    0xD2: EB	Take E into B
    0xD3: EO	Take E into O
    0xD4: EA	Take E into A
    0xD5: DEC	Decrement A
    0xD6: ES	Take E into SOR
    0xD7: EP	Take E into POR
    0xD8: ED	Take E into D
    0xD9: EW	Take E as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xDA: EJ	Take E as page offset and store it into PC - always
    0xDB: EH	Take E as page offset and store it into PC - if A is not equal to zero
    0xDC: EZ	Take E as page offset and store it into PC - if A is equal to zero
    0xDD: EN	Take E as page offset and store it into PC - if A is negative (has bit 7 set)
    0xDE: EG	Take E as page-index, load the index into G, set PC to 80h
    0xDF: EC	Take E as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xE0: SU	Take SIR as 8-bit signed number and add it to 16-bit pointer B_O
    0xE1: SM	Take SIR into M[B_O]
    0xE2: SB	Take SIR into B
    0xE3: SO	Take SIR into O
    0xE4: SA	Take SIR into A
    0xE5: SE	Take SIR into E
    0xE6: SS	Take SIR into SOR
    0xE7: SP	Take SIR into POR
    0xE8: SD	Take SIR into D
    0xE9: SW	Take SIR as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xEA: SJ	Take SIR as page offset and store it into PC - always
    0xEB: SH	Take SIR as page offset and store it into PC - if A is not equal to zero
    0xEC: SZ	Take SIR as page offset and store it into PC - if A is equal to zero
    0xED: SN	Take SIR as page offset and store it into PC - if A is negative (has bit 7 set)
    0xEE: SG	Take SIR as page-index, load the index into G, set PC to 80h
    0xEF: SC	Take SIR as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
    
    0xF0: PU	Take PIR as 8-bit signed number and add it to 16-bit pointer B_O
    0xF1: PM	Take PIR into M[B_O]
    0xF2: PB	Take PIR into B
    0xF3: PO	Take PIR into O
    0xF4: PA	Take PIR into A
    0xF5: PE	Take PIR into E
    0xF6: PS	Take PIR into SOR
    0xF7: PP	Take PIR into POR
    0xF8: PD	Take PIR into D
    0xF9: PW	Take PIR as page offset and store it into PC - while register D is not zero. In either case, decrement D
    0xFA: PJ	Take PIR as page offset and store it into PC - always
    0xFB: PH	Take PIR as page offset and store it into PC - if A is not equal to zero
    0xFC: PZ	Take PIR as page offset and store it into PC - if A is equal to zero
    0xFD: PN	Take PIR as page offset and store it into PC - if A is negative (has bit 7 set)
    0xFE: PG	Take PIR as page-index, load the index into G, set PC to 80h
    0xFF: PC	Take PIR as page-index, load the index into C, set PC to 0. Save return pointer into B_O. Decrement L
