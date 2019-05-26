#ifndef __HW_H__
#define __HW_H__


// IN push button
#define _IN_DOWN 10
#define _IN_UP   11

// Q pin mapped to microcontroller PIN
#define _Q    13

#define readIN_DOWN() readPin(_IN_DOWN)
#define readIN_UP()   readPin(_IN_UP)

void    hw_init();
uint8_t readSwitches();
uint8_t readHWSwitches();
uint8_t readControlSwitches();
uint8_t readPin(uint8_t pin);
void    loopSystem();
void    dump(char * line);
void    displayCpuInfo();
void    doEditingOperation(uint8_t state, uint8_t isDown);
void    doOperateMode(uint8_t mode);
void    writeHWLeds(uint16_t data);
void    doReset(uint8_t isDown);

// OPTIONS
#define  ST_OP_RESET  0b0000
#define  ST_OP_NONE2  0b0010
#define  ST_OP_HWTEST 0b0110
#define  ST_OP_NONE4  0b0100

#define  ST_OP_LOAD   0b1101
#define  ST_OP_SAVE   0b1110
#define  ST_OP_NONEB  0b1111

// RUN
#define  ST_RN_RUN    0b0001
#define  ST_RN_PAUSE  0b0011
#define  ST_RN_SLOW   0b0111
#define  ST_RN_FAST   0b0101

// EDIT READ
#define  ST_EDR_READ  0b1000
#define  ST_EDR_PAGE  0b1001
#define  ST_SET_ADDR  0b1010
#define  ST_OP_NONEB  0b1011 // Insert delete

// EDIT WRITE
#define  ST_EDW_WRITE 0b1100


#endif 


