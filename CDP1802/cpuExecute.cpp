#include <Arduino.h>
#include "cpu.h"
#include "io.h"
#include "hw.h"
#include "mem.h"

void cpu_execute(){
    uint8_t  bkp8, msb, lsb, bus8;
    uint16_t sum16, sub16, bkp16;

    // Fecth
    uint8_t opcode = cpu_fetch();

    // Decode opcode
    switch(opcode){
      
        // I = 0, N = 0, IDL
        // The 1802 (CPU) repeatedly cycles on the same instruction, waiting for an I/O request 
        // DMA-IN, DM
        // IDL  Idle  Wait for DMA or Interrupt M(R(0))->Bus
        case 0x00:
            cpu_idle();
            cpu.cycles+=2;
        break;

        // I = 0, N = 1 ~ F, LDN
        // This is the LOAD VIA N instruction. The CPU looks at the contents of register ‘N’, which
        // specifies which of the sixteen scratch-pad registers (except R(0) in this instance) will contain the
        // address of a memory location, and then the CPU goes to that memory location, gets the data byte
        // stored there, and saves it in the accumulator (‘D’ register). If ‘N’ = 0, then this is not an LDN
        // instruction; rather it is the IDL instruction instead
        // LDN  Load via N  
        //  M(R(N))->D; For N not 0
                   case 0x01: case 0x02: case 0x03: 
        case 0x04: case 0x05: case 0x06: case 0x07: 
        case 0x08: case 0x09: case 0x0A: case 0x0B: 
        case 0x0C: case 0x0D: case 0x0E: case 0x0F: 
            cpu.D = RD_M(cpu.R[cpu.N]);
            cpu.cycles+=2;
        break;
        
        // I = 1, N = 0 ~ F, INC
        // This is the INCREMENT REGISTER N instruction. The scratch-pad register specified by
        // register ‘N’, R(N), is incremented. If the specified register contains FFFF and is incremented, it
        // rolls over to 0000. There is no ‘overflow’ bit in the CPU which would be set in this instance. 
        // INC  Increment reg N  
        //   R(N)+1->R(N)
        case 0x10: case 0x11: case 0x12: case 0x13: 
        case 0x14: case 0x15: case 0x16: case 0x17: 
        case 0x18: case 0x19: case 0x1A: case 0x1B: 
        case 0x1C: case 0x1D: case 0x1E: case 0x1F: 
            cpu.R[cpu.N]++;
            cpu.cycles+=2;
        break;
        
        // I = 2, N = 0 ~ F, DEC
        // This is the DECREMENT REGISTER N instruction. The scratch-pad register specified by
        // register ‘N’, R(N), is decremented. If the specified register contains 0000 and is decremented, it
        // rolls over to FFFF. There is no ‘underflow’ bit in the CPU which would be set in this instance
        // DEC  Decrement reg N  
        //   R(N)-1->R(N)
        case 0x20: case 0x21: case 0x22: case 0x23: 
        case 0x24: case 0x25: case 0x26: case 0x27: 
        case 0x28: case 0x29: case 0x2A: case 0x2B: 
        case 0x2C: case 0x2D: case 0x2E: case 0x2F: 
            cpu.R[cpu.N]--;
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 0, BR
        // This is the UNCONDITIONAL SHORT BRANCH instruction which operates by replacing the
        // program counter’s low-order (least significant) byte with the data from the byte immediately
        // following this instruction. Remember that the program counter is the scratch-pad register pointed
        // to by the current contents of the ‘P’ register, and this is R(0) by default. The low-order byte’s
        // name is R(P).0. Examples: If the program counter is at 0E1A when the BR instruction is
        // executed, the CPU will look at memory location 0E1B for a data byte. Assume that byte contains
        // 3C. The CPU will overwrite the low-order byte of the program counter with 3C, so it will end up
        // being 0E3C, and the next instruction fetched will be from that memory location. 
        // BR  Branch 
        //    M(R(P))->R(P).0
        case 0x30:
            bkp8=RD_M(cpu.R[cpu.P]);
            SET_R_LOW(cpu.P, bkp8);
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 1, BQ
        // This SHORT BRANCH IF Q = 1 instruction operates similarly to the BR instruction described
        // above, except the CPU ignores and skips over this instruction if the status of the 1-bit ‘Q’
        // register is NOT = 1. So if ‘Q’ =1, the branch is implemented, but if ‘Q’ = 0 the branch is ignored
        // and the next instruction is fetched; the CPU knows to skip over the data byte following the BQ
        // instruction in this case. 
        // BQ  Branch if Q=1         
        //   if Q=1, 
        //      M(R(P))->R(P).0 
        //   else 
        //      R(P)+1->R(P)
        case 0x31:
            if(cpu.Q){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 2, BZ
        // This SHORT BRANCH IF D = 0 instruction operates similarly to the BQ instruction described
        // above, except whether the branch is implemented or not depends on the contents of the ‘D’
        // register instead of the ‘Q’ register. If ‘D’ = 0 the branch is implemented, and if ‘D’ is some other
        // value, the CPU skips to the next instruction. 
        // BZ  Branch if D=0
        //    If D=0,
        //      M(R(P))->R(P).0 
        //    else 
        //      R(P)+1->R(P)
        case 0x32:
            if(cpu.D==0x00){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        

        // BDF  Branch if DF=1        
        //   if DF=1, 
        //     M(R(P))->R(P).0 
        //   else 
        //     R(P)+1->R(P)
        case 0x33:
            if(cpu.DF){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 4, B1
        // This SHORT BRANCH IF EF1 = 1 instruction operates similarly to the BQ instruction
        // described above, except whether the branch is implemented or not depends on the status of the 1-
        // bit ‘EF1’ input pin on the 1802, instead of the ‘Q’ register. Since EF1 is an inverse acting input,
        // a low (0 Volt, or grounded) signal on this input means that the CPU will regard this input as
        // being logically true, or =1. If ‘EF1’ = 1 the branch is implemented, and if ‘EF1’ = 0, the CPU
        // skips to the next instruction
        // B1  Branch if EF1=1        if EF1=1, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x34:
            cpu_testFlags();
            if(cpu.EF1){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 5, 6, 7, B2, B3, B4
        // These branch instructions operate identically to the B1 instruction described above, except they
        // test different pins on the 1802, namely EF2, EF3, and EF4 respectively
        // B2  Branch if EF2=1  if EF2=1, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x35:
            cpu_testFlags();
            if(cpu.EF2){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // B3  Branch if EF3=1  if EF3=1, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x36:
            cpu_testFlags();
            if(cpu.EF3){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // B4  Branch if EF4=1  if EF4=1, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x37:
            cpu_testFlags();
            if(cpu.EF4){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 8, SKP (aka NBR)
        // This is the SHORT SKIP, or NO SHORT BRANCH instruction. When the CPU encounters this
        // instruction, the address immediately following the instruction is skipped over. In this way, it
        // behaves just like any of the preceding branch instructions, except that it does not test anything to
        // determine if the branch should be implemented or not; this branch (or skip) is ALWAYS
        // implemented. This instruction can be used a placeholder for another two-byte branch instruction.
        // NBR  No Branch  
        //   R(P)+1->R(P)
        case 0x38:
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = 3, N = 9, BNQ
        // This SHORT BRANCH IF Q = 0 instruction operates identically to the BQ instruction described
        // above, except that it tests for the ‘Q’ register to = 0, so a branch occurs only if ‘Q’ = 0. 
        // BNQ  Branch if Q=0  
        //   if Q=0, 
        //      M(R(P))->R(P).0 
        //   else 
        //      R(P)+1->R(P)
        case 0x39:
            if(!cpu.Q){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = A, BNZ
        // This SHORT BRANCH IF D NOT 0 instruction operates identically to the BZ instruction
        // described above, except the branch is implemented only if the bye in register ‘D’ is NOT equal
        // to 0. 
        // BNZ  Branch if D<>0  
        //   If D<>0, 
        //      M(R(P))->R(P).0 
        //   else 
        //      R(P)+1->R(P)
        case 0x3A:
            if(cpu.D!=0x00){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = B, BNF (aka BM, BL)
        // This SHORT BRANCH IF DF = 0, or SHORT BRANCH IF POSITIVE OR ZERO, or SHORT
        // BRANCH IF EQUAL OR GREATER instruction operates identically to the BDF instruction
        // described above, except the branch is implemented if ‘DF’ = 0
        // BNF  Branch if DF=0  
        //   if DF=0, 
        //     M(R(P))->R(P).0 
        //   else 
        //     R(P)+1->R(P)
        case 0x3B:
            if(!cpu.DF){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 3, N = C, D, E, F, BN1, BN2, BN3, BN4
        // These branch instructions operate identically to the B1 ~ B4 instructions described above, except
        // they test for the EF1 ~ EF4 pins to be logically = 0 (which means the corresponding pins on the
        // 1802 must be at (or near) the same voltage potential as the CPU’s positive power supply, i.e.
        // NOT at 0V / grounded). 
        // BN1  Branch if EF1=0  
        //   if EF1=0, 
        //     M(R(P))->R(P).0 
        //   else 
        //     R(P)+1->R(P)
        case 0x3C:
            cpu_testFlags();
            if(!cpu.EF1){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // BN2  Branch if EF2=0  if EF2=0, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x3D:
            cpu_testFlags();
            if(!cpu.EF2){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // BN3  Branch if EF3=0  if EF3=0, M(R(P))->R(P).0 else R(P)+1->R(P)
        case 0x3E:
            cpu_testFlags();
            if(!cpu.EF3){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // BN4  Branch if EF4=0  if EF4=0, M(R(P))->R(P).0
        case 0x3F:
            cpu_testFlags();
            if(!cpu.EF4){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_LOW(cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]++;
            }
            cpu.cycles+=2;
        break;
        
        // I = 4, N = 0 ~ F, LDA
        // This is the LOAD ADVANCE instruction. The scratch-pad register specified by register ‘N’,
        // R(N), contains the address of a memory location; the CPU fetches the data from that location and
        // stores it in the ‘D’ register. Then the CPU increments the contents of R(N). 
        // LDA  Load Advance  
        //   M(R(N))->D; 
        //   R(N)+1->R(N)
        case 0x40: case 0x41: case 0x42: case 0x43: 
        case 0x44: case 0x45: case 0x46: case 0x47: 
        case 0x48: case 0x49: case 0x4A: case 0x4B: 
        case 0x4C: case 0x4D: case 0x4E: case 0x4F: 
            cpu.D=RD_M(cpu.R[cpu.N]);
            cpu.R[cpu.N]++;
            cpu.cycles+=2;
        break;
        
        // I = 5, N = 0 ~ F, STR
        // This is the STORE VIA N instruction. The scratch-pad register specified by register ‘N’, R(N),
        // contains the address of a memory location; the CPU fetches the data from the ‘D’ register and
        // stores it in the specified memory location. The contents of ‘D’ are not changed. 
        // STR  Store via N  
        //   D->M(R(N))
        case 0x50: case 0x51: case 0x52: case 0x53: 
        case 0x54: case 0x55: case 0x56: case 0x57: 
        case 0x58: case 0x59: case 0x5A: case 0x5B: 
        case 0x5C: case 0x5D: case 0x5E: case 0x5F: 
            WR_M(cpu.R[cpu.N], cpu.D);
            cpu.cycles+=2;
        break;
        
        // I = 6, N = 0, IRX
        // This is the INCREMENT REGISTER X instruction. The scratch-pad register specified by
        // register ‘X’, R(X), is incremented. 
        // IRX  Increment reg X  
        //   R(X)+1->R(X)
        case 0x60:
            cpu.R[cpu.X]++;
            cpu.cycles+=2;
        break;
        
        // I = 6, N = 1, 2, 3, 4, 5, 6, 7, OUT
        // This is the OUTPUT TO I/O instruction. The scratch-pad register specified by register ‘X’,
        // R(X), contains the address of a memory location. The CPU retrieves the data from that memory
        // location and places it on the data bus. Then the three low-order bits of register ‘N’ are used to
        // control the status of the three output pins of the 1802 called N0, N1, and N2. These pins may be
        // connected to logic that selects various I/O devices. If the binary pattern on N0 ~ N2 corresponds
        // to a logically gated I/O device (e.g. the eight toggle switches or the 2-digit hex display of the
        // ELF), then the CPU’s I/O control system automatically takes care of writing the data on the bus
        // to the logically selected I/O device (it must be a device capable of being written to, since this is
        // an output instruction). Normally, the three N outputs (or any subset of these three outputs) may
        // be gated along with the 1802’s MRD output (the MRD pin must be low). The R(X) is
        // automatically incremented after writing the data to the I/O device. If subsequent outputs of data
        // from the same memory location are required, the value in R(X) must be changed back to point to
        // the correct device before executing this instruction again. Note that the ‘N’ part of this
        // instruction can have a value from 1 ~ 7, and this gets written into the ‘N’ register before
        // executing the rest of the instruction. Hence, the ‘N’ part of this instruction influences which I/O
        // device will be written to. Since there is no option for ‘N’ to be = 0 with this instruction, then the
        // three low-order bits of register ‘N’ cannot be equal to 0, and thus at least one of them must be =
        // 1. Because of this clever arrangement, at least one of the N outputs will be on during execution
        // of this instruction, and therefore an I/O device will be selected. If this instruction allowed for ‘N’
        // to = 0, then none of the N outputs would be on and no I/O device would be selected.
        //   M(R(X))->Bus; 
        //   R(X)+1->R(X); 
                   case 0x61: case 0x62: case 0x63: 
        case 0x64: case 0x65: case 0x66: case 0x67:
            cpu_output(RD_M(cpu.R[cpu.X]), cpu.N);
            cpu.R[cpu.X]++;
            cpu.cycles+=2;
        break;
                
        // ESC
        // EXTENDED   1805 extended (68) instructions
        case 0x68:
            cpu.cycles+=2;
        break;
        
        // I = 6, N = 9, A, B, C, D, E, F, INP
        // This is the INPUT FROM I/O instruction. This instruction works similarly to the OUT
        // instruction described above, but in reverse, reading a byte of data from an I/O device. The
        // scratch-pad register specified by register ‘X’, R(X), contains the address of a memory location.
        // The ‘N’ part of the instruction is 9 ~ F, and this is written in the ‘N’ register. Because the I/O
        // system only uses the three low-order bits of the ‘N’ register, the most significant bit is ignored.
        // As a result, if 9 is written into ‘N’, the I/O system sees 1 (9 = 1001 binary, and if dropping the
        // most significant bit the binary number becomes 0001, or 1). Similarly a written A is read by the
        // I/O system as 2, etc; up through a written F being read as 7. In this way, this instruction
        // corresponds to the OUT instruction’s ‘N’ part which can be 1 ~ 7. Normally, the three N outputs
        // (or any subset of these three outputs) may be gated along with the 1802’s MRD output (the MRD
        // pin must be high). Then the CPU’s I/O control system automatically takes care of reading the
        // data from the selected I/O device (it must be a device capable of being read from, since this is an
        // input instruction). The data is written into the memory location specified by R(X), and the same
        // data is also saved to the ‘D’ register. The R(X) register is NOT incremented.
        //   Bus->M(R(X)); 
        //   Bus->D; 
                   case 0x69: case 0x6A: case 0x6B:
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:
            bus8 = cpu_input(cpu.N&0b111);
            WR_M(cpu.R[cpu.X], bus8);
            cpu.D=bus8;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 0, RET
        // This is the RETURN instruction. The single hex digits in the ‘X’ and ‘P’ registers are replaced
        // by the data stored in the memory byte addressed by scratch-pad register R(X), and R(X) is then
        // incremented. The 1-bit ‘IE’ register (Interrupt Enable) is set = 1. Even if the other aspects of this
        // instruction are not required, it can be used solely to set the ‘IE’ bit. 
        // RET  Return  
        //   M(R(X))->(X,P); 
        //   R(X)+1->R(X); 
        //   1->IE
        case 0x70:
            bkp16 = RD_M(cpu.R[cpu.X]);
            cpu.X = (bkp16>>4) & 0x0F;
            cpu.P =  bkp16     & 0x0F;
            cpu.R[cpu.X]++;
            cpu.IE=1;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 1, DIS
        // This is the DISABLE instruction, and it is similar to the RET instruction described above,
        // except instead of setting ‘IE’ = 1, ‘IE’ is reset to = 0. Even if the other aspects of this instruction
        // are not required, it can be used solely to reset the ‘IE’ bit. 
        // DIS  Disable  
        //   M(R(X))->(X,P); 
        //   R(X)+1->R(X); 
        //   0->IE
        case 0x71:
            bkp16 = RD_M(cpu.R[cpu.X]);
            cpu.X = (bkp16>>4) & 0x0F;
            cpu.P =  bkp16     & 0x0F;
            cpu.R[cpu.X]++;
            cpu.IE=0;        
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 2, LDXA
        // This is the LOAD ‘D’ VIA ‘X’ AND ADVANCE instruction. The data in the memory location
        // addressed by the scratch-pad register specified by ‘X’, R(X), is placed into register ‘D’, and the
        // address in R(X) is incremented. The contents of the memory location specified by R(X) are not
        // changed. 
        // LDXA  Load via X and advance  
        //   M(R(X))->D; 
        //   R(X)+1->R(X)
        case 0x72:
            cpu.D=RD_M(cpu.R[cpu.X]);
            cpu.R[cpu.X]++;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 3, STXD
        // This is the STORE ‘D’ VIA ‘X’ AND DECREMENT instruction. The byte in ‘D’ is stored in
        // the memory location addressed by the contents of the register specified by ‘X’, R(X), and R(X)
        // is decremented. 
        // STXD  Store Via X and dec.  
        //   D->M(R(X)); 
        //   R(X)-1->R(X)
        case 0x73:
            WR_M(cpu.R[cpu.X], cpu.D);
            cpu.R[cpu.X]++;
            cpu.cycles+=2;
        break;

        // I = 7, N = 4, ADC
        // This is the ADD WITH CARRY instruction. The memory byte addressed by the scratch-pad
        // register specified by ‘X’, R(X) is added to the contents of the ‘D’ register, and also the contents
        // of the 1-bit ‘DF’ register is also added to ‘D’. The 8-bit result of the double addition is stored in
        // ‘D’. Regardless of what the status of ‘DF’ was before the operation, afterwards ‘DF’ will be set
        // = 1 if the addition resulted in a carry, and if not it will be reset = 0. 
        // ADC  Add with carry  
        //   M(R(X))+D+DF->DF,D
        case 0x74:
            sum16 = RD_M(cpu.R[cpu.X])+cpu.D+cpu.DF;
            cpu.D  = (uint8_t)(sum16 & 0xFF);
            cpu.DF = (sum16&0xFF00)?1:0;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 5, SDB
        // This is the SUBTRACT WITH BORROW instruction. The value of the ‘D’ register is subtracted
        // from the memory byte addressed by the scratch-pad register specified by ‘X’, R(X), and then the
        // inverted contents of the 1-bit ‘DF’ register is also subtracted from the same memory location,
        // and the result of the double subtraction is stored in the ‘D’ register. The ‘DF’ is also changed if
        // necessary as a result of the subtraction. 
        // SDB  Sub. D with borrow  
        //   M(R(X))-D-(NOT DF)->DF; D
        case 0x75:
            sub16 = RD_M(cpu.R[cpu.X])-cpu.D-(!cpu.DF);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1; 
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 6, SHRC (aka RSHR)
        // This is the SHIFT RIGHT WITH CARRY, or RING SHIFT RIGHT instruction. The binary
        // contents of the ‘D’ register are shifted one bit position to the right, or least significant direction.
        // The ‘0’, or lowest order, position of the byte is moved to the CARRY bit ‘DF’, while the
        // previous contents of ‘DF’ are moved to the ‘7’, or highest order, position of the ‘D’ register
        //              D register   DF
        //     BEFORE   0000 1100    1
        //     AFTER    1000 0110    0        
        // Shift D right; Shift right with carry 
        //   lsb(D)->DF; 
        //   DF->msb(D)
        case 0x76:
            lsb = cpu.D&0x01;
            cpu.D>>=1;
            cpu.D |= (cpu.DF?0b10000000:0b00000000);
            cpu.DF = lsb;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 7, SMB
        // This is the SUBTRACT MEMORY WITH BORROW instruction. The byte in memory
        // addressed by R(X), plus the borrow (indicated by register ‘DF = 0) is subtracted from the ‘D’
        // register. The result is placed in the ‘D’ register, and if a new borrow occurred as a result of the
        // operation, ‘DF’ will be reset to = 0. This instruction is similar to SDB described above, but with
        // the operands (‘D’ and the memory location) reversed. 
        // SMB  Sub. Mem. with borrow  
        //   D - M(R(X)) - (NOT DF)->DF, D
        case 0x77:
            sub16 = cpu.D-RD_M(cpu.R[cpu.X])-(!cpu.DF);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 8, SAV
        // This is the SAVE instruction. The byte contained in the 8-bit ‘T’ register is stored at the
        // memory location addressed by the scratch-pad register specified by ‘X’, R(X). Subsequent
        // execution of a RETURN (RET) or DISABLE (DIS) instruction, as described above, can then
        // replace the original ‘X’ and ‘P’ values to resume, or return to, normal program execution.
        // SAV  Save  
        //   T->M(R(X))
        case 0x78:
            WR_M(cpu.R[cpu.X], cpu.T);
            cpu.cycles+=2;
        break;
        
        // I = 7, N = 9, MARK
        // This is the PUSH X, P TO STACK instruction. The current contents of the two 4-bit registers
        // ‘X’ and ‘P’ are combined as ‘XP’ and stored in the ‘T’ register, and the same 8-bit value is also
        // stored in the memory location addressed by scratch-pad register R(2). The contents of ‘P’ are
        // then stored in ‘X’ and the 16-bit value in R(2) is decremented. 
        // MARK  Push X,P to stack  
        //    (X,P) ->T; 
        //    (X,P) ->M(R(2)) 
        //    then P->X; 
        //    R(2)-1->R(2)
        case 0x79:
            cpu.T = cpu.X;
            cpu.T <<= 4;
            cpu.T |=  cpu.P;
            WR_M(cpu.R[2], cpu.T);
            cpu.X = cpu.P;
            cpu.R[2]--;
            cpu.cycles+=2;
        break;

        // I = 7, N = A, REQ
        // This is the RESET ‘Q’ instruction. The contents of the 1-bit register ‘Q’ is reset = 0, and since
        // ‘Q’ controls the ‘Q’ output of the 1802, that output goes low (to 0V or circuit ground potential).  
        // REQ  Reset Q  0->Q
        case 0x7A:
            cpu.Q = 0;
            cpu_outputQ();
            cpu.cycles+=2;
        break;
        
        // I = 7, N = B, SEQ
        // This is the SET ‘Q’ instruction. The contents of the 1-bit register ‘Q’ is set = 1, and since ‘Q’
        // controls the ‘Q’ output of the 1802, that output goes high (to the same potential as the power
        // supply voltage to the 1802 IC). 
        // SEQ  Set Q  1->Q
        case 0x7B:
            cpu.Q = 1;
            cpu_outputQ();        
            cpu.cycles+=2;
        break;
        
        // I = 7, N = C, ADCI
        // This is the ADD WITH CARRY - IMMEDIATE instruction. The contents of the memory
        // location addressed by the scratch-pad register specified by ‘P’, R(P), is added to the contents of
        // the ‘D’ register, and the results are stored in the ‘D’ register. The final state of the ‘DF’ register
        // indicated whether or not a carry occurred as a result of the addition. Register R(P) is
        // incremented. 
        // ADCI  Add with carry imm.  
        //   M(R(P)) + D + DF->DF,D; 
        //   R(P)+1->R(P)
        case 0x7C:
            sum16 = RD_M(cpu.R[cpu.P])+cpu.D+cpu.DF;
            cpu.D  = (uint8_t)(sum16 & 0xFF);
            cpu.DF = (sum16&0xFF00)?1:0;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = D, SDBI
        // This is the SUBTRACT D WITH BORROW - IMMEDIATE instruction. Similar to the SBD
        // instruction described above, except instead of the minuend coming from an addressed memory
        // location, it comes from the byte immediately following this instruction. 
        // SDBI  Sub. D with borrow imm.  
        //   M(R(P)) - D - (Not DF) -> DF, D; 
        //   R(P) + 1 -> R(P)
        case 0x7D:
            sub16 = RD_M(cpu.R[cpu.P])-cpu.D-(!cpu.DF);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = E, SHLC (aka RSHL)
        // This is the SHIFT LEFT WITH CARRY, or RING SHIFT LEFT instruction. It is similar to the
        // SHRC instruction described above, except the data is shifted to the left instead of to the right.
        // The contents of ‘DF’ are moved into the least significant bit of ‘D’, and the most significant bit
        // of ‘D’ is moved into ‘DF’. 
        // Shift D left; 
        //   msb(D)->DF; 
        //   DF->lsb(D)
        case 0x7E:
            msb = cpu.D&0b10000000?1:0;
            cpu.D<<=1;
            cpu.D |= (cpu.DF?0b00000001:0b00000000);
            cpu.DF = msb;
            cpu.cycles+=2;
        break;
        
        // I = 7, N = F, SMBI
        // This is the SUBTRACT MEMORY WITH BORROW - IMMEDIATE instruction. It is similar
        // to the SMB function described above, except instead of the subtrahend coming from an
        // addressed byte in memory, it comes from the byte immediately following the instruction. 
        // SMBI  Sub. Mem. w/borrow imm.  
        //   D-M(R(P))-(NOT DF) -> DF, D; 
        //   R(P) + 1 -> R(P)
        case 0x7F:
            sub16 = cpu.D-RD_M(cpu.R[cpu.P])-(!cpu.DF);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = 8, N = 0 ~ F, GLO
        // This is the GET LOW REG N instruction. The low-order byte of the scratch-pad register
        // specified by ‘N’ is placed on the ‘D’ register. 
        // GLO  Get low reg N  
        //   R(N).0->D
        case 0x80: case 0x81: case 0x82: case 0x83: 
        case 0x84: case 0x85: case 0x86: case 0x87: 
        case 0x88: case 0x89: case 0x8A: case 0x8B: 
        case 0x8C: case 0x8D: case 0x8E: case 0x8F: 
            cpu.D=GET_R_LOW(cpu.N);
            cpu.cycles+=2;
        break;
        
        // I = 9, N = 0 ~ F, GHI
        // This is the GET HIGH REG N instruction. The high-order byte of the scratch-pad register
        // specified by ‘N’ is placed on the ‘D’ register. 
        // GHI  Get high reg N  
        //   R(N).1->D
        case 0x90: case 0x91: case 0x92: case 0x93: 
        case 0x94: case 0x95: case 0x96: case 0x97: 
        case 0x98: case 0x99: case 0x9A: case 0x9B: 
        case 0x9C: case 0x9D: case 0x9E: case 0x9F: 
            cpu.D=GET_R_HIGH(cpu.N);
            cpu.cycles+=2;
        break;
        
        // I = A, N = 0 ~ F, PLO
        // This is the PUT LOW REG N instruction. The data in the ‘D’ register is copied to the low-order
        // byte of the scratch-pad register specified by ‘N’. The contents of ‘D’ are not changed. 
        // PLO  Put low reg N  
        //  D->R(N).0
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: 
        case 0xA4: case 0xA5: case 0xA6: case 0xA7: 
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: 
        case 0xAC: case 0xAD: case 0xAE: case 0xAF: 
            SET_R_LOW(cpu.N, cpu.D);
            cpu.cycles+=2;
        break;
        
        // I = B, N = 0 ~ F, PHI
        // This is the PUT HIGH REG N instruction. The data in the ‘D’ register is copied to the high-order
        // byte of the scratch-pad register specified by ‘N’. The contents of ‘D’ are not changed. 
        // PHI  Put high reg N  D->R(N).1
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: 
        case 0xB4: case 0xB5: case 0xB6: case 0xB7: 
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: 
        case 0xBC: case 0xBD: case 0xBE: case 0xBF: 
            SET_R_HIGH(cpu.N, cpu.D);
            cpu.cycles+=2;
        break;
     
        // I = C, N = 0, LBR
        // This is the unconditional LONG BRANCH instruction. The two bytes of data following the
        // instruction are copied and then written into the full 16-bit scratch-pad register specified by ‘P’,
        // R(P). For example, if the program contains these three bytes in sequence: C0 25 3A, then the
        // contents of R(P) will be 253A, which is treated as an address. The CPU will execute its next
        // instruction at that address, jumping forward or backwards as required to do so.
        // LBR  Long Branch  
        //   M(R(P))  ->R(P).1; 
        //   M(R(P)+1)->R(P).0
        case 0xC0:
            bkp8 = RD_M(cpu.R[cpu.P]);
            SET_R_HIGH(cpu.P, bkp8);
            bkp8 = RD_M(cpu.R[cpu.P]+1);
            SET_R_LOW (cpu.P, bkp8);
            cpu.cycles+=3;
        break;

        //  I = C, N = 1, LBQ
        // This is the LONG BRANCH IF Q instruction. It is similar to the LBR instruction above, except
        // it only implements the branch if the ‘Q’ register = 1. 
        // LBQ  Branch if Q=1  
        //   if Q=1 then 
        //     M(R(P))->R(P).1; 
        //     M(R(P)+1)->R(P).0 
        //   else 
        //     R(P)+2->R(P)
        case 0xC1:
            if(cpu.Q){
                bkp8 = RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8 = RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 2, LBZ
        // This is the LONG BRANCH IF D = 0 instruction. It is similar to the LBQ instruction above,
        // except it only implements the branch if the contents of the ‘D’ register = 0. 
        // LBZ  Branch if D=0  
        //   if D=0 then 
        //      M(R(P))->R(P).1; 
        //      M(R(P)+1)->R(P).0 
        //   else 
        //     R(P)+2->R(P)
        case 0xC2:
            if(cpu.D==0){
                bkp8 = RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8 = RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 3, LBDF
        // This is the LONG BRANCH IF DF = 1 instruction. It is similar to the LBQ instruction above,
        //e xcept it only implements the branch if the contents of the ‘DF’ register = 1. 
        // LBDF  Branch if DF=1  
        //   if DF=1 then 
        //     M(R(P))->R(P).1; 
        //     M(R(P)+1)->R(P).0 
        // else 
        //     R(P)+2->R(P)
        case 0xC3:
            if(cpu.DF){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8=RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 4, NOP
        // This is the NO OPERATION instruction. The CPU does nothing with this instruction, and
        // executes the instruction at the next address. 
        // NOP  No operation  Continue
        case 0xC4:
            cpu.cycles+=3;
        break;
        
        // I = C, N = 5, LSNQ
        // This is the LONG SKIP IF Q = 0 instruction. If ‘Q’ = 0, the next two bytes following this
        // instruction are skipped. For example, if the following bytes appear in sequence: C5 55 25 F2,
        // and the C5 is executed, the CPU will check to see if Q = 0. If it is, then the next instruction
        // executed will be the F2. If it is not, then the next instruction executed will be the 55. 
        // LSNQ  Skip if Q=0 
        //   if Q=0, 
        //     R(P)+2->R(P) 
        //   else 
        //   Continue
        case 0xC5:
            if(!cpu.Q){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 6, LSNZ
        // This is the LONG SKIP IF D IS NOT 0 instruction. It works like the LSNQ instruction above,
        // except the test is for the ‘D’ register being any value other than 0. 
        // LSNZ  Skip if D<>0  
        //   if D<>0, 
        //     R(P)+2->R(P) 
        //   else 
        //     Continue
        case 0xC6:
            if(cpu.D!=0){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 7, LSNF
        // This is the LONG SKIP IF DF = 0 instruction. It works like the LSNQ instruction above, except
        // the test is for the ‘DF’ register being = 0. 
        // LSNF  Skip if DF=0  
        //   if DF=0, 
        //     R(P)+2->R(P) 
        //   else 
        //     Continue
        case 0xC7:
            if(!cpu.DF){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = 8, LSKP (aka NLBR)
        // This is the unconditional LONG SKIP, or NO LONG BRANCH instruction. The two bytes
        // following the C8 instruction are skipped and the CPU executes the instruction that follows the
        // skipped bytes. The alternate instruction mnemonic implies that the two skipped bytes represent
        // an unused branch address. 
        // NLBR  No long branch  
        //   R(P)+2->R(P)
        case 0xC8:
            cpu.R[cpu.P]+=2;
            cpu.cycles+=3;
        break;
        
        // I = C, N = 9, LBNQ
        // This is the LONG BRANCH IF Q = 0 instruction. It is similar to the LBQ instruction above,
        // except it only implements the branch if the ‘Q’ register = 0. 
        // LBNQ  Branch if Q=0  
        //   if Q=0 then 
        //     M(R(P))->R(P).1; 
        //     M(R(P)+1)->R(P).0 
        //   else 
        //     R(P)+2->R(P)
        case 0xC9:
            if(!cpu.Q){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8=RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = A, LBNZ
        // This is the LONG BRANCH IF D NOT 0 instruction. It is similar to the LBZ instruction above,
        // except it only implements the branch if the contents of the ‘D’ register is some value 
        // other than 0. 
        // LBNZ  Branch if D<>0  
        //   if D<>0 then 
        //     M(R(P))->R(P).1; 
        //     M(R(P)+1)->R(P).0 
        //   else 
        //     R(P)+2->R(P)
        case 0xCA:
            if(cpu.D!=0){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8=RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = B, LBNF
        // This is the LONG BRANCH IF DF = 0 instruction. It is similar to the LBDF instruction above,
        // except it only implements the branch if the contents of the ‘DF’ register = 0. 
        // LBNF  Branch if DF=0  
        //   if DF=0 then 
        //     M(R(P))->R(P).1; 
        //     M(R(P)+1)->R(P).0 
        //   else 
        //     R(P)+2->R(P)
        case 0xCB:
            if(!cpu.DF){
                bkp8=RD_M(cpu.R[cpu.P]);
                SET_R_HIGH(cpu.P, bkp8);
                bkp8=RD_M(cpu.R[cpu.P]+1);
                SET_R_LOW (cpu.P, bkp8);
            }else{
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = C, LSIE
        // This is the LONG SKIP IF IE = 1 instruction. It works like the LSNQ instruction above, except
        // the test is for the 1-bit ‘IE’ register being = 1. 
        // LSIE  Skip if IE=1  
        //   if IE=0, 
        //     R(P)+2->R(P) 
        //   else 
        //     Continue
        case 0xCC:
            if(!cpu.IE){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = D, LSQ
        // This is the LONG SKIP IF Q = 1 instruction. It works like the LSNQ instruction above, except
        // the test is for the ‘Q’ register being = 1. 
        // LSQ  Skip if Q=1  
        //   if Q=1, 
        //      R(P)+2->R(P) 
        //    else 
        //      Continue
        case 0xCD:
            if(cpu.Q){
                cpu.R[cpu.P]+=2;
            }        
            cpu.cycles+=3;
        break;
        
        // I = C, N = E, LSZ
        // This is the LONG SKIP IF D = 0 instruction. It works like the LSNZ instruction above, except
        // the test is for the ‘D’ register being = 0. 
        // LSZ  Skip if D=0  
        //   if D=0, 
        //     R(P)+2->R(P) 
        //   else 
        //     Continue
        case 0xCE:
            if(cpu.D==0){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = C, N = F, LSDF
        // This is the LONG SKIP IF DF = 1 instruction. It works like the LSNF instruction above, except
        // the test is for the ‘DF’ register being = 1. 
        // LSDF  Skip if DF=1  
        //   if DF=1, 
        //     R(P)+2->R(P) 
        //   else 
        //     Continue
        case 0xCF:
            if(cpu.DF){
                cpu.R[cpu.P]+=2;
            }
            cpu.cycles+=3;
        break;
        
        // I = D, N = 0 ~ F, SEP
        // This is the SET P instruction. The single hex digit (4-bit binary) currently in the ‘N’ register is
        // copied to the ‘P’ register. This is used to specify which of the scratch-pad registers will be used
        // as the program counter (remember that R(P) is the defined program counter). When ‘P’ is set by
        // this instruction, the CPU immediately jumps to the instruction sequence beginning at the
        // memory address stored in the scratch-pad register specified originally by ‘N’. 
        // SEP  Set P  N->P
        case 0xD0: case 0xD1: case 0xD2: case 0xD3: 
        case 0xD4: case 0xD5: case 0xD6: case 0xD7: 
        case 0xD8: case 0xD9: case 0xDA: case 0xDB: 
        case 0xDC: case 0xDD: case 0xDE: case 0xDF: 
            cpu.P=cpu.N;
            cpu.cycles+=2;
        break;
        
        // I = E, N = 0 ~ F, SEX
        // This is the SET X instruction. The single hex digit (4-bit binary) currently in the ‘N’ register is
        // copied to the ‘X’ register. This is used to designate R(X) for arithmetic instructions and I/O byte
        // transfer operations. 
        // SEX  Set X  N->X
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: 
        case 0xE4: case 0xE5: case 0xE6: case 0xE7: 
        case 0xE8: case 0xE9: case 0xEA: case 0xEB: 
        case 0xEC: case 0xED: case 0xEE: case 0xEF: 
            cpu.X = cpu.N;
            cpu.cycles+=2;
        break;
        
        // I = F, N = 0, LDX
        // This is the LOAD VIA X instruction. The contents of the memory byte addressed by the contents
        // of the scratch-pad register specified by ‘X’, R(X), will be written to the ‘D’ register. This
        // instruction does not increment the address in the way that the LDA instruction does. The
        // contents of the memory location are not changed. 
        // LDX  Load via X  
        //   M(R(X))->D
        case 0xF0:
            cpu.D = RD_M(cpu.R[cpu.X]);
            cpu.cycles+=2;
        break;
        
        // I = F, N = 1, OR
        // This is the OR instruction. The individual bits of the two 8-bit operands are combined according
        // to the rules for logical OR. The byte currently in the ‘D’ register is one of the operands, while the
        // contents of the byte in the memory location addressed by R(X) is the second operation. The
        // results of the OR operation are stored in the ‘D’ register, replacing that operand. This instruction
        // is particularly useful in setting individual bits in a byte of data. 
        // OR  Or  
        //   M(R(X)) or D->D
        case 0xF1:
            cpu.D |= RD_M(cpu.R[cpu.X]);
            cpu.cycles+=2;
        break;
        
        // I = F, N = 2, AND
        // This is the AND instruction. It works similarly to the OR instruction described above, except the
        // logical operation is AND. This instruction is particularly useful to test if one or more particular
        // bits in a byte of data are currently = 1, as well as to mask individual bits. 
        // AND  And  
        //   M(R(X)) and D->D
        case 0xF2:
            cpu.D &= RD_M(cpu.R[cpu.X]);
            cpu.cycles+=2;
        break;
        
        // I = F, N = 3, XOR
        // This is the EXCLUSIVE OR instruction. It works similarly to the OR instruction described
        // above, except the logical operation is XOR. This instruction is particularly useful to compare two
        // data bytes for equality since identical values will result in all 0’s in ‘D’. 
        // XOR  Exclusive or  
        //   M(R(X)) xor D->D
        case 0xF3:
            cpu.D ^= RD_M(cpu.R[cpu.X]);
            cpu.cycles+=2;
        break;

        // I = F, N = 4, ADD
        // This is the ADD instruction. It works similarly to the OR instruction described above, except
        // instead of a logical operation on the two operands, an addition is done. The result is stored in
        // ‘D’, and ‘DF’ =1 if a carry occurred, and ‘DF’ = 0 otherwise. 
        // ADD  Add  
        //   M(R(X))+D->DF,D
        case 0xF4:
            sum16 = RD_M(cpu.R[cpu.X])+cpu.D;
            cpu.D  = (uint8_t)(sum16 & 0xFF);
            cpu.DF = (sum16&0xFF00)?1:0;
            cpu.cycles+=2;
        break;
        
        // I = F, N = 5, SD
        // This is the SUBTRACT D instruction. The byte in ‘D’ is subtracted from the data in the memory
        // location addressed by R(X). The 8-bit result is stored in the ‘D’ register, replacing the
        // subtrahend. The ‘DF’ register is modified as a result of this operation; ‘DF’ = 0 if there was a
        // borrow, and ‘DF’ = 1 is there was no borrow. 
        // SD  Subtract D  
        //   M(R(X))-D->DF,D
        case 0xF5:
            sub16 = RD_M(cpu.R[cpu.X])-cpu.D;
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1; 
            cpu.cycles+=2;
        break;
        
        // I = F, N = 6, SHR
        // This is the SHIFT RIGHT instruction. It is similar to the SHRC instruction described on page 18,
        // except nothing (‘DF’ or any other source) is shifted into the most significant bit of ‘D’; that bit
        // will always be 0 after this instruction has executed. 
        // SHR  Shift right  
        //   Shift D right; 
        //   lsb(D)->DF; 
        //   0->msb(D)
        case 0xF6:
            cpu.D>>=1;
            cpu.cycles+=2;
        break;
        
        // I = F, N = 7, SM
        // This is the SUBTRACT MEMORY instruction. It is the opposite of the SD instruction. The byte
        // of data in the memory location addressed by R(X) is subtracted from the data byte currently in
        // ‘D’. The 8-bit result is stored in the ‘D’ register, replacing the minuend. The ‘DF’ register is
        // modified as a result of this operation; ‘DF’ = 0 if there was a borrow, and ‘DF’ = 1 is there was
        // no borrow. 
        // SM  Subtract memory  
        //   D-M(R(X))->DF,D
        case 0xF7:
            sub16  = cpu.D-RD_M(cpu.R[cpu.X]);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1; 
            cpu.cycles+=2;
        break;
        
        // I = F, N = 8, LDI
        // This is the LOAD IMMEDIATE instruction. The data byte immediately following the current
        // instruction’s byte will be copied to the ‘D’ register. The program counter R(P) will be
        // incremented such that it points at the memory location following the data byte, so it is ready for
        // the next instruction fetch. 
        // LDI  Load immediate  
        //   M(R(P))->D; 
        //   R(P)+1->R(P)
        case 0xF8:
            cpu.D = RD_M(cpu.R[cpu.P]);
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = 9, ORI
        // This is the OR IMMEDIATE instruction. It works similarly to the OR instruction described
        // above, except that the second operand is in the byte immediately following this instruction
        // instead of coming from an addressed memory location. 
        // ORI  Or immediate  
        //   M(R(P)) or D->D; 
        //   R(P)+1->R(P)
        case 0xF9:
            cpu.D = RD_M(cpu.R[cpu.P]) | cpu.D;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = A, ANI
        // This is the AND IMMEDIATE instruction. It works similarly to the AND instruction described
        // above, except that the second operand is in the byte immediately following this instruction
        // instead of coming from an addressed memory location. 
        // ANI  And immediate  
        //   M(R(P)) and D->D; 
        //   R(P)+1->R(P)
        case 0xFA:
            cpu.D = RD_M(cpu.R[cpu.P]) & cpu.D;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = B, XRI
        // This is the EXCLUSIVE OR IMMEDIATE instruction. It works similarly to the XOR
        // instruction described above, except that the second operand is in the byte immediately following
        // this instruction instead of coming from an addressed memory location. 
        // XRI  Exclusive or immediate  
        //   M(R(P)) xor D->D; 
        //   R(P)+1->R(P)
        case 0xFB:
            cpu.D = RD_M(cpu.R[cpu.P]) ^ cpu.D;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = C, ADI
        // This is the ADD IMMEDIATE instruction. It works similarly to the ADD instruction described
        // above, except that the second operand is in the byte immediately following this instruction
        // instead of coming from an addressed memory location. 
        // ADI  Add immediate  
        //   M(R(P))+D->DF,D; 
        //   R(P)+1->R(P)
        case 0xFC:
            sum16  = RD_M(cpu.R[cpu.P])+cpu.D;
            cpu.D  = (uint8_t)(sum16&0xFF);
            cpu.DF = (sum16&0xFF00)?1:0;
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = D, SDI
        // This is the SUBTRACT IMMEDIATE instruction. It works similarly to the SD instruction
        // described above, except that the second operand is in the byte immediately following this
        // instruction instead of coming from an addressed memory location. It is similar to the SMI
        // instruction below except that the operands are reversed. 
        // SDI  Subtract D immediate  
        //   M(R(P))-D->DF,D; 
        //   R(P)+1->R(P)
        case 0xFD:
            sub16  = RD_M(cpu.R[cpu.P])-cpu.D;
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1; 
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
        
        // I = F, N = E, SHL
        // This is the SHIFT LEFT instruction. This works similarly to SHR except the bits are shifted to
        // the left instead of to the right. The final low order bit of ‘D’ will always be 0 after this instruction
        // has executed. 
        // SHL  Shift left  
        //   Shift D left; 
        //   msb(D)->DF; 
        //   0->lsb(D)
        case 0xFE:
            cpu.D<<=1;
            cpu.cycles+=2;
        break;
        
        // I = F, N = F, SMI
        // This is the SUBTRACT MEMORY IMMEDIATE instruction. It works similarly to the SM
        // instruction described above, except that the second operand is in the byte immediately following
        // this instruction instead of coming from an addressed memory location. It is similar to the SDI
        // instruction above except that the operands are reversed. 
        // SMI  Subtract Mem. imm.  
        //   D-M(R(P))->DF,D; 
        //   R(P)+1->R(P)
        case 0xFF:
            sub16  = cpu.D-RD_M(cpu.R[cpu.P]);
            cpu.D  = (uint8_t)(sub16 & 0xFF);
            cpu.DF = (sub16&0xFF00)?0:1; 
            cpu.R[cpu.P]++;
            cpu.cycles+=2;
        break;
    }

}

