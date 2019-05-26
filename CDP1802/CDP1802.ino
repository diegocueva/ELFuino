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


