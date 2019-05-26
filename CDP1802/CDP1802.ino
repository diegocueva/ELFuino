#include "cpu.h"
#include "hw.h"
#include "mem.h"
#include "io.h"

void setup() {  
  hw_init();
  unitTest();
}

void loop() {
  loopSystem();
}

uint8_t prgTest[] = {0x7A, 0xF8, 0x10, 0xB1, 0x21, 0x91, 0x3A, 0x04, 0X31, 0X00, 0X7B, 0X30, 0X01, 0X00 };
//uint8_t prgTest[] = {0xF8, 0x05, 0x3F, 0x02, 0x37, 0x04, 0xFF, 0x01, 0x3A, 0x02, 0x7B, 0x30, 0x0A, 0x00 };
//uint8_t prgTest[] = {0xE1, 0x90, 0xB1, 0xF8, 0x0A, 0xA1, 0x6C, 0x64, 0x30, 0x00, 0x00};

void unitTest(){
  char buff[100];
  sprintf(buff, "sizeof(cpu)=%d\n", sizeof(cpu)); Serial.print(buff);
  cpu_reset();

  cpu.R[4]=0xFFFF;
  SET_R_LOW(4,0xCD);
  sprintf(buff, "cpu.R[4] = 0x%04X\n", cpu.R[4]); Serial.print(buff); 
  SET_R_HIGH(4,0xAB);
  sprintf(buff, "cpu.R[4] H=0x%02X L=0x%02X\n", GET_R_HIGH(4), GET_R_LOW(4)); Serial.print(buff); 
  
  mem[0] = 0x14; cpu_execute();
  sprintf(buff, "execute I=%X N=%X\n", cpu.I, cpu.N); Serial.print(buff); 
  sprintf(buff, "cpu.R[4] = 0x%04X\n", cpu.R[4]); Serial.print(buff); 

  for(int i=0; i<14; i++){
    WR_M(i, prgTest[i]);
  }
  dumpMem(cpu.R[cpu.P], buff); Serial.println(buff);
  cpu_reset();
  cpuStatus(buff);Serial.println(buff);
}

