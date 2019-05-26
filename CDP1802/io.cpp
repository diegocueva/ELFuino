#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "cpu.h"
#include "hw.h"
#include "io.h"

extern LiquidCrystal_I2C  lcd;

/**
  The Q Flip-Flop
  An internal flip-flop, Q, can be set or reset by instruction and
  can be sensed by conditional branch instructions. The output
  of Q is also available as a microprocessor output.
 */
void cpu_outputQ(){
     digitalWrite(_Q, cpu.Q);
}

uint8_t cpu_input (uint8_t Nlines){
    char buff[20];
    sprintf(buff, "IN Nl=%d\n", Nlines); 
    //Serial.print(buff);
    return readHWSwitches();
}

void cpu_output(uint8_t data, uint8_t Nlines){
    char buff[20];
    sprintf(buff, "OUT %02X Nl=%d\n", data, Nlines); 
    //Serial.print(buff);
    lcd.setCursor(Nlines*2, 1);
    sprintf(buff, "%02X", data);
    lcd.print(buff);

    writeHWLeds((uint16_t)data);
}

/**
 * Read External flags
 *  The external signals must be negated
 */
void cpu_testFlags(){
    cpu.EF1 = 1;
    cpu.EF2 = readPin(12);
    cpu.EF3 = readPin(11);
    cpu.EF4 = readIN_DOWN();
}

void cpu_idle(){
    while(!readIN_DOWN());
    delay(10);
    while(readIN_DOWN());  
}

