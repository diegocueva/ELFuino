/********************************************************
 * There is no warranty for this software.
 * This software you have permission to be copied, 
 * distributed and/or modify for any purposes, 
 * except commercial purposes. 
 * For commercial purposes contacted me:
 *    diegocueva@gmail.com
 *    www.diegocueva.com
 ********************************************************/
#include "cpu.h"
#include "hw.h"
#include "mem.h"
#include "io.h"

void setup() {  
  hw_init();
  loadEEPROM();
}

void loop() {
  loopSystem();
}


