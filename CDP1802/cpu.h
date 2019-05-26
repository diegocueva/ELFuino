/***********************************************************************
 * 
 *   * https://bitbucket.org/don/cosmac-1802-emulator/src/2158bb8bca59f9a36afbd901e9d46f2be876fdc8/1802%20Emulator/CPU%20Emulation.c?at=master&fileviewer=file-view-default* 
 * 
 * REFERENCES 
 * http://www.cosmacelf.com/publications/data-sheets/cdp1802.pdf
 * http://www.cosmacelf.com/publications/books/cosmac-elf-manual.pdf
 * http://bitsavers.trailing-edge.com/components/rca/cosmac/MPM-201A_User_Manual_for_the_CDP1802_COSMAC_Microprocessor_1976.pdf
 * http://www.elf-emulation.com/1802.html
 * http://www.cosmacelf.com/publications/books/short-course-in-programming.html
 ***********************************************************************/
#ifndef __CPU_H__
#define __CPU_H__


#define SET_R_LOW(a,b)  {cpu.R[a]&=0xFF00; cpu.R[a]|=b;}
#define SET_R_HIGH(x,y) {cpu.R[x]&=0x00FF; cpu.R[x]|=(((uint16_t)y)<<8);}
#define GET_R_LOW(x)    ((uint8_t)(cpu.R[x]))
#define GET_R_HIGH(x)   ((uint8_t)(cpu.R[x]>>8))


/**
 * CDP1802 CPU Definition
 * 
 * sizeof(cpu)=45
 */
typedef struct CDP1802{
  uint16_t R[16]; // 16 Bits 1 of 16 Scratchpad Registers
    
  uint8_t  D;     // 8 Bits Data Register (Accumulator)
  uint8_t  T;     // 8 Bits Holds X and P during interrupt, X is high nybble, not directly accessable  
    
  uint8_t  P:4;   // 4 Bits Designates which register is Program Counter
  uint8_t  X:4;   // 4 Bits Designates which register is Data Pointer
  uint8_t  I:4;   // 4 Bits Holds High-Order Instruction Digit, not directly accessable
  uint8_t  N:4;   // 4 Bits Low nybble of instruction byte, not directly accessable

  uint8_t  DF:1;  // 1-Bit Data Flag (ALU Carry/borrow)  
  uint8_t  IE:1;  // 1-Bit Interrupt Enable

  /**
    Single bit output from the CPU which can be set or reset
    under program control. During SEQ or REQ instruction execution, 
    Q is set or reset between the trailing edge of TPA and
    the leading edge of TPB. 
   */
  uint8_t  Q:1;   // 1-Bit Output Flip/Flop

  /**
    EF1 to EF4 (4 Flags)
    These inputs enable the I/O controllers to transfer status
    information to the processor. The levels can be tested by the
    conditional branch instructions. They can be used in conjunction 
    with the INTERRUPT request line to establish interrupt priorities. 
    These flags can also be used by I/O devices
    to “call the attention” of the processor, in which case 
    the program must routinely test the status of these flag(s). 
    The flag(s) are sampled at the beginning of every S1 cycle.
   */
  uint8_t  EF1:1; // External Flag 1
  uint8_t  EF2:1; // External Flag 2
  uint8_t  EF3:1; // External Flag 3
  uint8_t  EF4:1; // External Flag 4

  /** Cycles counter */
  uint64_t cycles;
};

extern CDP1802 cpu;

void    cpu_reset();
uint8_t cpu_fetch();
void    cpu_execute();
void    cpu_testFlags();
void    cpu_output(uint8_t data, uint8_t Nlines);
uint8_t cpu_input (uint8_t Nlines);
void    cpu_outputQ();
void    dump(char*line);
void    cpuStatus(char*line);

#endif
