
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    Verilog sketch for the "Myth" Microcontroller                        //
//    Just for reference                                                   //
//    Project files: https://github.com/michaelmangelsdorf/myth            //
//    Author: Michael <mim@ok-schalter.de>                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


//Decoder driven control signals/////////////////////////////////////////////

reg NOP, SSI, SSO, SCL, SCH, RTS, RTI, COR;         // SYS
reg RBO, BOR, WBO, BOW, IBO, BOI, SBO, BOS;         // BOP
reg DUP, SWAP, NOTA, NOTX, SLA, SLX, SRA, SRX;      // ALU
reg AND, IOR, EOR, ADD, OVF, ALX, AEX, AGX;
reg TRAP;                                           // TRAP
reg GETB, GETO, GETA, GETD, PUTB, PUTO, PUTA, PUTD; // GETPUT
reg Fx, Mx, Bx, Ox, Ax, Ex, Sx, Px;                 // PAIR SRC
reg xU, xM, xB, xO, xA, xE, xS, xP;                 // PAIR DST
reg xD, xW, xJ, xH, xZ, xN, xR, xC;
reg CODE, LOCAL, LEAVE, ENTER, INCA, DECA;          // PAIR SCROUNGE


//Register declarations//////////////////////////////////////////////////////

// Page-Index Registers:
reg [7:0] C;        // Code
reg [7:0] R;        // Resident
reg [7:0] L;        // Local
reg [7:0] B;        // Base

// Page-Offset Registers:
reg [7:0] O;        // Offset
reg [7:0] PC;       // Program Counter

// Accumulator Registers:
reg [7:0] A;        // Accumulator A
reg [7:0] X;        // Accumulator X

// Input-Output Registers:
reg [7:0] E;        // Enable
reg [7:0] SIR, SOR; // Serial I/O Registers
reg [7:0] PIR, POR; // Parallel I/O Registers
reg MISO, MOSI, SCLK;

// Utility Registers:
reg [7:0] D;        // Down-Counter
reg [7:0] I;        // Instruction

// System Inputs:
reg MISO;
reg IRQ;
reg rst;
reg clk;

// System Outputs:
reg MOSI;
reg SCLK;
reg BUSY;

//Derivative signals/////////////////////////////////////////////////////////

//                      -- Opcode Bits --
//                         MSB     LSB
//    all 0: OPC_SYS       00000   xxx    b0-2: Index of SYS Instruction
//     else: OPC_BOP       00001   xxx    b0-2: Index of BOP Instruction
//           OPC_ALU       0001   xxxx    b0-3: Index of ALU Instruction
//           OPC_TRAP      001   xxxxx    b0-4: DESTPAGE
//           OPC_GETPUT    01 xx x xxx    b0-2: OFFS, b3: GET/PUT, b4-5: REG
//           OPC_PAIR      1  xxx xxxx    b0-3: DST, b4-6: SRC

// Opcode byte in instruction register I
// Detect priority encoded opcode group, one of the following one-hot:
wire OPC_SYS    = ~|I[7:3];
wire OPC_BOP    = ~|I[7:4] & I[3];
wire OPC_ALU    = ~|I[7:5] & I[4];
wire OPC_TRAP   = ~|I[7:6] & I[5];
wire OPC_GETPUT = ~I[7] & I[6];
wire OPC_PAIR   = I[7];

wire GET = OPC_GETPUT & ~I[3];
wire PUT = OPC_GETPUT & I[3];

sum = A + X;
carry = sum[8];
overflow = (~(A[7] ^ X[7]) & (sum[7] ^ A[7]));

// Used in O block and PC block:
reg [7:0] PC_old;


///Combinational decoder block///////////////////////////////////////////////

// Opcode byte in instruction register I
// Update CPU control signals based on detected opcode group

always @* begin

	// Clear 73 decoder signals 
    
    NOP=0; SSI=0;  SSO=0;  SCL=0;  SCH=0; RTS=0; RTI=0; COR=0;   // Clear SYS
    
    RBO=0; BOR=0;  WBO=0;  BOW=0;  IBO=0; BOI=0; SBO=0; BOS=0;   // Clear BOP
    
    DUP=0; SWAP=0; NOTA=0; NOTX=0; SLA=0; SLX=0; SRA=0; SRX=0;   // Clear ALU
    AND=0; IOR=0;  EOR=0;  ADD=0;  OVF=0; ALX=0; AEX=0; AGX=0;
    
    TRAP=0;                                                     // Clear TRAP
    
    GETB=0;GETO=0; GETA=0; GETD=0;                            // Clear GETPUT
    PUTB=0; PUTO=0; PUTA=0; PUTD=0;
    
    Fx=0; Mx=0; Bx=0; Ox=0; Ax=0; Ex=0; Sx=0; Px=0;             // Clear PAIR
    xU=0; xM=0; xB=0; xO=0; xA=0; xE=0; xS=0; xP=0;
    xD=0; xW=0; xJ=0; xH=0; xZ=0; xN=0; xR=0; xC=0;
    
    CODE=0; LOCAL=0; LEAVE=0; ENTER=0; INCA=0; DECA=0;      // Clear SCROUNGE

    if (OPC_SYS) begin
        case (I[2:0])
            3'd0: NOP = 1;
            3'd1: SSI = 1;
            3'd2: SSO = 1;
            3'd3: SCL = 1;
            3'd4: SCH = 1;
            3'd5: RTS = 1;
            3'd6: RTI = 1;
            3'd7: COR = 1;
        endcase
    end

    if (OPC_BOP) begin
        case (I[2:0])
            3'd0: RBO = 1;
            3'd1: BOR = 1;
            3'd2: WBO = 1;
            3'd3: BOW = 1;
            3'd4: IBO = 1;
            3'd5: BOI = 1;
            3'd6: SBO = 1;
            3'd7: BOS = 1;
        endcase
    end

    if (OPC_ALU) begin
        case (I[3:0])
            4'd0:  DUP = 1;
            4'd1:  SWAP = 1;
            4'd2:  NOTA = 1;
            4'd3:  NOTX = 1;
            4'd4:  SLA = 1;
            4'd5:  SLX = 1;
            4'd6:  SRA = 1;
            4'd7:  SRX = 1;
            4'd8:  AND = 1;
            4'd9:  IOR = 1;
            4'd10: EOR = 1;
            4'd11: ADD = 1;
            4'd12: OVF = 1;
            4'd13: ALX = 1;
            4'd14: AEX = 1;
            4'd15: AGX = 1;
        endcase
    end

    if (OPC_TRAP) TRAP = 1;

    if (OPC_GETPUT) begin
        case ({I[4], I[5], I[3]})
            3'd0: GETB = 1;
            3'd1: GETO = 1;
            3'd2: GETA = 1;
            3'd3: GETD = 1;
            3'd4: PUTB = 1;
            3'd5: PUTO = 1;
            3'd6: PUTA = 1;
            3'd7: PUTD = 1;
        endcase
    end

    if (OPC_PAIR) begin
        case ({I[6:4], // "Scrounge" certain source/destination combos
               I[3:0]})
            {3'd0, 4'd1}: CODE  = 1; // Scrounge FxM
            {3'd1, 4'd1}: LOCAL = 1; // Scrounge MxM
            {3'd2, 4'd2}: LEAVE = 1; // Scrounge BxB
            {3'd3, 4'd3}: ENTER = 1; // Scrounge OxO
            {3'd4, 4'd4}: INCA  = 1; // Scrounge AxA
            {3'd5, 4'd5}: DECA  = 1; // Scrounge ExE
            default: begin
                case (I[6:4])        // Source
                    3'd0: Fx = 1;
                    3'd1: Mx = 1;
                    3'd2: Bx = 1;
                    3'd3: Ox = 1;
                    3'd4: Ax = 1;
                    3'd5: Ex = 1;
                    3'd6: Sx = 1;
                    3'd7: Px = 1;
                endcase
                case (I[3:0])        // Destination
                    4'd0: xU = 1;
                    4'd1: xM = 1;
                    4'd2: xB = 1;
                    4'd3: xO = 1;
                    4'd4: xA = 1;
                    4'd5: xE = 1;
                    4'd6: xS = 1;
                    4'd7: xP = 1;
                    4'd8: xD = 1;
                    4'd9: xW = 1;
                   4'd10: xJ = 1;
                   4'd11: xH = 1;
                   4'd12: xZ = 1;
                   4'd13: xN = 1;
                   4'd14: xR = 1;
                   4'd15: xC = 1;
                endcase
            end
        endcase
    end
end


//Generate CPU phases////////////////////////////////////////////////////////

reg [4:0] state; // One-hot encoded FSM state (1 bit per phase)

assign FETCH  = state[0];
assign DECODE = state[1];
assign SETUP  = state[2];
assign READY  = state[3];
assign CLOSE  = state[4];

always @(posedge clk or posedge rst) begin
    if (rst)
        state <= 5'b00001;               // Initialize to FETCH (bit 0)
    else
        state <= {state[3:0], state[4]}; // Rotate left
end


//ALU and ACCUMULATOR////////////////////////////////////////////////////////

// ALU auxilliary logic
reg [7:0] alu_result;
reg [8:0] full_sum;
reg carry, overflow;
always @(*) begin
    full_sum = A + X;                                    // 9-bit result
    carry    = full_sum[8];                              // Carry out bit
    overflow = (~(A[7] ^ X[7]) & (full_sum[7] ^ A[7]));  // Signed overflow
end

always @(posedge clk or posedge rst) begin
    if (reset) begin
        A <= 8'd0;
        X <= 8'd0;
    end else begin
        if ((xA || GETA) && READY) A <= data_bus; // Load from data bus
        else if (SETUP && OPC_ALU) begin
            case (1'b1)
                DUP:  alu_result = A;
                SWAP: alu_result = X;
                NOTA: alu_result = ~A;
                NOTX: alu_result = ~X;
                SLA:  alu_result = A << 1;
                SLX:  alu_result = X << 1;
                SRA:  alu_result = A >> 1;
                SRX:  alu_result = X >> 1;
                AND:  alu_result = A & X;
                IOR:  alu_result = A | X;
                EOR:  alu_result = A ^ X;
                ADD:  alu_result = A + X;
                OVF:  alu_result = {carry, overflow, 6'b000000};
                ALX:  alu_result = (A < X)  ? 8'hFF : 8'h00;
                AEX   alu_result = (A == X) ? 8'hFF : 8'h00;
                AGX:  alu_result = (A > X)  ? 8'hFF : 8'h00;
                default: alu_result = A;
            endcase
            X <= A;          // Update X with A
            A <= alu_result; // Update A with ALU result
        end
    end
end


//B,O,POINTERS///////////////////////////////////////////////////////////////

// Auxiliary logic
reg [7:0] O_old;
reg [7:0] B_old;

// Pointer registers:
reg [15:0] RP;
reg [15:0] WP;
reg [15:0] IP;
reg [15:0] SP;

wire [15:0] BO = {B,O};  // Combined Base-Offset pointer for BOP instructions

always @(posedge clk or posedge rst) begin
    if (rst) begin
        B <=  8'b0;
        O <=  8'b0;
        R <= 16'b0;
        W <= 16'b0;
        I <= 16'b0;
        S <= 16'b0;
    end
    else if (SETUP) case (1'b1)
        xC:   B_old <= B;
        COR:  B_old <= B;
        TRAP: B_old <= B;
        default:;
    endcase
    else if (READY)
    begin
        if      (xB || GETB)  B <= data_bus;
        else if (xO || GETO)  O <= data_bus;
        else case (1'b1)
            RTS:
            RTI:

            BOW: W <= BO;
            BOR: R <= BO;
            BOI: I <= BO;
            BOS: S <= BO;

            RBO: {B, O} <= R;
            WBO: {B, O} <= W;
            IBO: {B, O} <= I;
            SBO: {B, O} <= S;
            
            CODE: {B, O} <= {PC[7] ? R:C, PC};
            default:;
        endcase
    end
end


//D,PC///////////////////////////////////////////////////////////////////////

// Status Flags
wire AZ = ~|A[7:0]; // A zero
wire AH = |A[7:0];  // A hot/not zero
wire AN = A[7];     // A negative
wire DH = ~|D[7:0]; // D hot/not zero

always @(posedge clk or posedge rst)
begin
    if (rst) begin
        D <= 8'd0;
        PC <= 8'd0;
        PC_old <= 8'd0;
    end
    else
    if (SETUP)
        case (1'b1)
            xC:   PC_old <= PC;
            COR:  PC_old <= PC;
            TRAP: PC_old <= PC;
            RTS:  PC <= O;
            RTI:  PC <= O;
            default:;
        endcase
    else
    if (READY)
        case (1'b1)
            // Interpage
            xC:   PC <= 0;
            TRAP: PC <= 0;
            COR:  PC <= O;

            // Intrapage
            xJ: PC <= data_bus;                // Unconditional jump
            xW: begin
                    if (DH) PC <= data_bus;    // Jump while D not zero
                    D <= D-1;                  // Always decrement
                end
            xH: if (AH) PC <= data_bus;        // Jump if A not zero
            xZ: if (AZ) PC <= data_bus;        // Jump if A zero
            xN: if (AN) PC <= data_bus;        // Jump if A negative
            
            // Assignments
            xD: D <= data_bus;                 // Write D
            GETD: ;                            // Load D from GETPUT location

            default:;
        endcase
    else
    if (CLOSE) case (1'b1)
        default:;
    endcase
end


//MEMORY,L,R,I///////////////////////////////////////////////////////////////

// Instantiate 64k x 8 synchronous RAM
ram vendor_ram (
  .address ( addr ),
  .clock ( clk ),
  .data ( data ),
  .wren ( wren ),
  .q ( q )
);

// Build RAM interface
reg[15:0] addr; // 16-bit memory address
reg[7:0]  data; // 8-bit write value
reg       wren; // write enable signal
wire[7:0] q;    // 8-bit RAM output value

always @(posedge clk or posedge rst) begin
    if (rst) begin
        L <= 8'h00;
        R <= 8'h00;
        I <= 8'h00; // Load with NOP opcode
    end
    else case (1'b1)
        FETCH:  begin
                    addr <= (PC[7]) ? {R,PC} : {C,PC};  // Request opcode
                    wren <= 0;                          // Reset write enable
                end
        DECODE: begin
                    if (IRQ && !BUSY && C!=0) I <= q;   // Latch opcode
                    else I <= 8'd32;                    // Inject TRAP 0
                end
        SETUP:  case (1'b1)
                    Fx:         addr <= (PC[7]) ? {R,PC} : {C,PC};
                    OPC_GETPUT: addr <= {L, 8'hF8 + I[2:0]};
                    default:    addr <= {B,O};
                endcase
        READY:  if (xM || PUT) begin
                    wren <= 1;                          // Cleared on FETCH
                    data <= data_bus;
                end
                else if (Mx || Fx || GET) data_bus <= q; // Latch memory data
                else if (ENTER || xC || OPC_TRAP) L <= L - 1;
                else if (LEAVE || RTS || RTI) L <= L + 1;
                else if (xR) R <= data_bus;
        default:;
    endcase
end


//E,SIR,SOR,PIR,POR,MISO,MOSI,BUSY,SCLK//////////////////////////////////////

always @(posedge clk or posedge rst)
begin
    if (rst) begin
         SIR <= 8'h00;
         SOR <= 8'h00;
         PIR <= 8'h00;
         POR <= 8'h00;
           E <= 8'h00;
        MOSI <= 1'b0;
        BUSY <= 1'b0;
        SCLK <= 1'b0;
    end
    else case (1'b1)
        SETUP:  case (1'b1)
                    SSI: SIR <= {SIR[6:0], MISO};
                    SSO: begin
                            MOSI <= SOR[7];
                            SOR <= {SOR[6:0], 1'b0};
                         end
                    SCL: SCLK <= 1'b0;
                    SCH: SCLK <= 1'b1;
                endcase
        CLOSE:  
                case (1'b1)
                    xE: E <= data_bus;
                    xS: SOR <= data_bus;
                    xP: POR <= data_bus;
                    TRAP: if (C==0) BUSY <= 1'b1;
                    RTI: BUSY <= 1'b0;
                    default:;
                endcase
        default:;
    endcase
end


//DATA_BUS///////////////////////////////////////////////////////////////////

reg [7:0] data_bus; // Data bus transfer value for PAIR type instructions

always @(posedge clk or posedge rst)
    if (rst || FETCH) data_bus <= 8'd0; // "Weak pull-down"
    else if (SETUP)
        case (1'b1)
            Bx: data_bus <= B;
            Ox: data_bus <= O;
            Ax: data_bus <= A;
            Ex: data_bus <= E;
            Sx: data_bus <= SIR;
            Px: data_bus <= PIR;
            default:;
        endcase
    else if (READY)
        if (Fx || Mx) data_bus <= q;
end


