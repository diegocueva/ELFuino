#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MCP23017.h>
#include "hw.h"
#include "cpu.h"
#include "mem.h"

LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7 );
Adafruit_MCP23017 mcp;

uint8_t lastMode = 0b0000;
uint8_t lastSwitchs ;

void hw_init(){
    Serial.begin(115200);

    // Pin mode for control switches
    DDRC = 0b00000000;

    // THE Q pin :)
    pinMode(_Q,  OUTPUT);
    digitalWrite(_Q, LOW);

    // MCP 23017 switchs & leds
    mcp.begin(0);
    for(int i=0; i< 8; i++){
        mcp.pinMode(i, INPUT);
        mcp.pullUp (i, HIGH);
        mcp.pinMode(i+8, OUTPUT);
    }

    // Display
    lcd.begin (16, 2); 
    lcd.setBacklightPin(3, POSITIVE);
    lcd.setBacklight(1);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("ELFuino");
    lcd.setCursor(1, 1);
    lcd.print("diegocueva.com");

    writeHWLeds(0xFFFF);
    delay(700);
    writeHWLeds(0x0000);
    delay(300);
}

uint8_t readPin(uint8_t pin){
  uint8_t value = digitalRead(pin);
  for(;;){
    delay(1);
    if(digitalRead(pin) != value){
      value = digitalRead(pin);
    }else{
      return value;
    }
  }
}

uint8_t readIN(){
    if( readIN_DOWN() || readIN_UP() ){
        delay(10);
        if(readIN_DOWN() ) return 1;
        if(readIN_UP() ) return 2;
    }
    return 0;
}

uint8_t readSwitches(){
  uint8_t value = readHWSwitches();
  for(;;){
    if(readHWSwitches() != value){
      value = readHWSwitches();
    }else{
      return value;
    }
  }
}

uint8_t readHWSwitches(){
    return mcp.readGPIO(0);
}

void writeHWLeds(uint16_t data){
  mcp.writeGPIOAB(data<<8);
}

uint8_t readControlSwitches(){
   uint8_t portC = PINC;
   return portC & 0b001111;
}

char printable(uint8_t c){
      return (32<=c && c<=127)?c:'.';
}
void displaySwitches(uint8_t switchs){
    char buff[4];
    sprintf(buff, "%02X",
        switchs
    );
    lcd.setCursor(14,0);
    lcd.print(buff);
}

void displayCpuInfo(){
    char buff[20];
    sprintf(buff, "%04X:%02X %X%X %02X",
        cpu.R[cpu.P],
        RD_M(cpu.R[cpu.P]),
        cpu.I,
        cpu.N,
        cpu.D
    );
    lcd.setCursor(0,0);
    lcd.print(buff);
}

void displayEditInfo(uint8_t mode){
    char buff[20];
    uint16_t addr = cpu.R[cpu.P];
    sprintf(buff, "%04X:%02X%c    ",
        cpu.R[cpu.P],
        RD_M(cpu.R[cpu.P]),
        mode&0b0100?'w':'r'
    );
    lcd.setCursor(0,0);
    lcd.print(buff);

    sprintf(buff, "%02X %02X %02X %02X ",
      RD_M(addr-1),
      RD_M(addr),
      RD_M(addr+1),
      RD_M(addr+2)
    );
    lcd.setCursor(0,1);
    lcd.print(buff);
    
    sprintf(buff, "%c%c%c%c",
      printable(RD_M(addr-1)),
      printable(RD_M(addr)),
      printable(RD_M(addr+1)),
      printable(RD_M(addr+2))
    );
    lcd.setCursor(12, 1);
    lcd.print(buff);
}

void loopSystem(){
    uint8_t mode = readControlSwitches();
    if(mode == ST_RN_RUN){
        cpu_execute();
    }else{
        doOperateMode(mode);
    }
    lastMode = mode;
}

void doOperateMode(uint8_t mode){
    char buff[20], rin;
    uint8_t switchs = readHWSwitches();

    // OPERATIONS
    if(mode==ST_OP_RESET || mode==ST_OP_HWTEST || mode==ST_OP_SAVE || mode==ST_OP_LOAD){ 
        switch(mode){
            // Reset
            case ST_OP_RESET:
                if(mode != lastMode){
                    doReset(1);
                }else if( rin=readIN() ){
                    doReset(rin==1);
                }
            break;
    
            // HARDWARE TEST
            case ST_OP_HWTEST:
                if(switchs != lastSwitchs || lastMode != mode){
                    sprintf(buff, "   %d%d%d%d%d%d%d%d",
                        (switchs&0b10000000?1:0),
                        (switchs&0b01000000?1:0),
                        (switchs&0b00100000?1:0),
                        (switchs&0b00010000?1:0),
                        (switchs&0b00001000?1:0),
                        (switchs&0b00000100?1:0),
                        (switchs&0b00000010?1:0),
                        (switchs&0b00000001?1:0)
                    );
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(buff);
                    displaySwitches(switchs);
                    writeHWLeds((uint16_t)switchs);
                    lastSwitchs = switchs;                    
                }
                digitalWrite(_Q, digitalRead(_IN_DOWN));
                if(digitalRead(_IN_UP)){
                    writeHWLeds(0xFFFF);
                    delay(10);
                    while(digitalRead(_IN_UP));
                    writeHWLeds((uint16_t)switchs);
                }
            break;        

            case ST_OP_SAVE:
                if(lastMode != mode){
                    lcd.clear();
                    lcd.setCursor(1, 0);
                    lcd.print("**** SAVE ****");
                }
                if( readIN()==1 ){
                    for(int i=0; i<MEM_SIZE || i <1024; i++){
                        EEPROM.write(i, RD_M(i));
                    }
                    lcd.setCursor(1, 1);
                    lcd.print("DONE");                    
                    while( readIN_DOWN() || readIN_UP() );
                }                
            break;

            case ST_OP_LOAD:
                if(lastMode != mode){
                    lcd.clear();
                    lcd.setCursor(1, 0);
                    lcd.print("Load program");
                }
                if( readIN()==1 ){
                    for(int i=0; i<MEM_SIZE || i <1024; i++){
                        WR_M(i, EEPROM.read(i));
                    }
                    lcd.setCursor(1, 1);
                    lcd.print("DONE");                    
                    while( readIN_DOWN() || readIN_UP() );
                }                
            break;            
        }
    }else if(mode & ST_EDR_READ){ // IS EDIT MODE ?
        // EDIT OPERATION WHEN PUSH
        if( rin=readIN() ){
            doEditingOperation(mode, rin==1);
        }
        if(lastSwitchs != switchs){
            displaySwitches(switchs);
        }        
        if(lastMode != mode){
            displayEditInfo(mode);
            displaySwitches(switchs);
            writeHWLeds((uint16_t)RD_M(cpu.R[cpu.P]));
        }        
    }else if(mode & ST_RN_RUN){ // IS RUN MODE ?
        // RUN DEBUG MODE
        switch(mode){
            case ST_RN_PAUSE:
              if(readIN_DOWN()){
                  delay(10);
                  if( readIN_DOWN()){
                      cpu_execute();
                      displayCpuInfo();
                      while( readIN_DOWN());
                  }
              }                
            break;
            
            case ST_RN_SLOW:
              cpu_execute();
              displayCpuInfo();
              while(readIN_UP());
              delay(1000); 
            break;
                  
            case ST_RN_FAST:
              cpu_execute();
              displayCpuInfo();
              while( readIN_UP() );
              delay(100);              
            break;
        }
        if(lastMode != mode){
            displayCpuInfo();
            displaySwitches(switchs);
        }        
        if(lastSwitchs != switchs){
            displaySwitches(switchs);
        }        
    }
    
    lastSwitchs = switchs;    
}


/******************************** EDITING OPERATION ***************************/
void doEditingOperation(uint8_t mode, uint8_t isDown){
    uint8_t value = readHWSwitches();
        
    switch(mode){
      // READ MODE
      case ST_EDR_READ:
          if(isDown){
              cpu.R[cpu.P]++;  
          }else{
              cpu.R[cpu.P]--;
          }
          writeHWLeds((uint16_t)RD_M(cpu.R[cpu.P]));
      break;

      case ST_EDR_PAGE:
          if(isDown){
              cpu.R[cpu.P] += 0x10;
          }else{
              cpu.R[cpu.P] -= 0x10;
          }
          writeHWLeds((uint16_t)RD_M(cpu.R[cpu.P]));
      break;

      case ST_SET_ADDR:          
          if(isDown){
              cpu.R[cpu.P] &= 0b1111111100000000;
              cpu.R[cpu.P] |= (value);
          }else{
              cpu.R[cpu.P] &= 0b0000000011111111;
              cpu.R[cpu.P] |= (value<<8);
          }
          writeHWLeds((uint16_t)RD_M(cpu.R[cpu.P]));
      break;
            
      
      // WRITE MODE
      case ST_EDW_WRITE:
          if(isDown){
              value = readHWSwitches();
              writeHWLeds((uint16_t)value);
              WR_M(cpu.R[cpu.P], value);
              cpu.R[cpu.P]++;            
          }else{
              cpu.R[cpu.P]--;
              writeHWLeds((uint16_t)RD_M(cpu.R[cpu.P]));
          }
      break;

      /*
      case ST_EDW_INSERT:
          for(uint16_t i=MEM_SIZE-2; i>cpu.R[cpu.P]; i--){
              value = RD_M(i);
              WR_M(i+1, value);
          }
          WR_M(cpu.R[cpu.P], 0x00);
      break;
      
      case ST_EDW_DELETE:
          for(uint16_t i=cpu.R[cpu.P];i<MEM_SIZE-2; i++){
            value = RD_M(i+1);
            WR_M(i, value);
          }
          WR_M(cpu.R[cpu.P], 0x00);
      break;
      */
    }
    displayEditInfo(mode);     
    while( readIN_DOWN() || readIN_UP() );
}

/******************************** RESET ***************************/
void doReset(uint8_t isDown){
    cpu_reset();
    lcd.clear();  
    writeHWLeds(0xFFFF);
    if(!isDown){
        for(int i=0; i<MEM_SIZE; i++){
            WR_M(i, 0x00);
        }
    }
    while( readIN_DOWN() || readIN_UP() );
    writeHWLeds(0x0000);
}


