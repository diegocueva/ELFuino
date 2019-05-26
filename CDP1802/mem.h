#ifndef __MEM_H__
#define __MEM_H__

// Memory
#define MEM_SIZE 512
extern uint8_t mem[];

// Memory access macros
#define RD_M(x)   (mem[(x)%MEM_SIZE])
#define WR_M(x,y) (mem[(x)%MEM_SIZE]=y)

void dumpMem(uint16_t daddr, char * line);



#endif 
