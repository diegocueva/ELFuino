/********************************************************
 * There is no warranty for this software.
 * This software you have permission to be copied, 
 * distributed and/or modify for any purposes, 
 * except commercial purposes. 
 * For commercial purposes contacted me:
 *    diegocueva@gmail.com
 *    www.diegocueva.com
 ********************************************************/
#ifndef __HW_H__
#define __HW_H__

void    cpu_testFlags();
void    cpu_output(uint8_t data, uint8_t Nlines);
uint8_t cpu_input (uint8_t Nlines);
void    cpu_outputQ();
void    cpu_idle();

#endif 
