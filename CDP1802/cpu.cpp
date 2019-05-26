/********************************************************
 * There is no warranty for this software.
 * This software you have permission to be copied, 
 * distributed and/or modify for any purposes, 
 * except commercial purposes. 
 * For commercial purposes contacted me:
 *    diegocueva@gmail.com
 *    www.diegocueva.com
 ********************************************************/
#include <Arduino.h>
#include "cpu.h"
#include "hw.h"
#include "mem.h"
#include "io.h"

CDP1802 cpu;

/**
 * Reset
 * Registers l, N, Q are reset, lE is set and 0’s (VSS) are placed
 * on the data bus ... 
 * ... and register X, P, and R(0) are reset. 
 */
void cpu_reset(){
  cpu.I  = 0;
  cpu.N  = 0;
  cpu.Q  = 0;
  cpu.IE = 1;

  cpu.P    = 0;
  cpu.X    = 0;  
  cpu.R[0] = 0;
  
  cpu.cycles = 0;
  cpu_outputQ();
}

/**
 *  FETCH 
 *    MRP -> I,N; 
 *    RP + 1 -> RP 
 * 
 */
uint8_t cpu_fetch(){
    uint8_t opcode = RD_M(cpu.R[cpu.P]);
    cpu.I = opcode>>4;
    cpu.N = opcode;
  
    cpu.R[cpu.P]++;
    return opcode;
}

/**
 * Shows formated cpu internal values
 * 
 */
void cpuStatus(char *line){
   sprintf(line, 
           "D%02X P%XX%X:I%XN%X R0=0x%04X R1=0x%04X cy%ld\n", 
            cpu.D, cpu.P, cpu.X, cpu.I, cpu.N, cpu.R[0], cpu.R[1], cpu.cycles); 
}

