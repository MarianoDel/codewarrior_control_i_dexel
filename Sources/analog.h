#ifndef _ANALOG_H_
#define _ANALOG_H_

#include "hard.h"

//Configuraciones
#define RESOLUCION_8
//#define RESOLUCION_10

#define AN_PIN0 0x01
#define AN_PIN1 0x02
#define AN_PIN2 0x04
#define AN_PIN3 0x08

#define AN_CH0 0x00
#define AN_CH1 0x01
#define AN_CH2 0x02
#define AN_CH3 0x03

#define AN_CH26 0x1a

//--------- FUNCIONES --------//
void AdcInit (unsigned char);
unsigned short New_Sample(unsigned char);
unsigned short Get_Temp (void);


#endif
