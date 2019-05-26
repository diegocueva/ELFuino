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
#include "mem.h"

uint8_t  mem[MEM_SIZE];

/**
 * Show memory in next format:
 *  aaaa : XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX - ................
 *  aaaa = Address
 *  XX   = Hex memory content
 *  .... = Char memory content
 *  
 *   @arg line must be at least 80 bytes
 */
void dumpMem(uint16_t daddr, char * line){
    char hex[11];
    sprintf(line, " %04X : ", daddr);
    for(int i=0; i<16; i++){
        uint8_t d = RD_M(daddr+i);
        sprintf(hex,"%02X ", d);
        strcat(line, hex);
    }
    strcat(line, " - ");
    for(int i=0; i<16; i++){
        char c = RD_M(daddr+i);
        if(32 <= c && c <= 126){
            sprintf(hex,"%c", c);
            strcat(line, hex);
        }else{
            strcat(line, ".");
        }
    }
}

