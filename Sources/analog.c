#include "analog.h"
#include "derivative.h"


//Vbg tension de bandgap es de 1.2Vcc
//para medirlo debo tener BGBE = 1 en SPMSC1
// ADCK maximo = 8 Mhz
// ADCK minimo = 400 Khz

//---- VARIABLES EXTERNAS ----//



void AdcInit (unsigned char adc_state) {	//con un 0 reseteo y vuelvo a GPIO
  
  if (adc_state) {
    
    //ADCSC1 = 0x5f;  //hab int; modo no continuo; no selecciona canal
    ADCSC1 = 0x1f;  //disa int; modo no continuo; no selecciona canal
    ADCSC2 = 0x00;  //software trigger; comp func disa; 
  
#ifdef RESOLUCION_8
    ADCCFG = 0x00;

#else
    ADCCFG = 0x08;
  
#endif  

    APCTL1 |= adc_state; //una or con los pines a usar
  }else {
    ADCSC1 = 0x1f;  //estado de reset
    ADCSC2 = 0x00;  //estado de reset
    APCTL1 &= 0xfd;
    ADCCFG = 0x00;  //estado de reset
    //pongo 1 en la pata
    //PTADD
  }
  
}

unsigned short New_Sample (unsigned char canal) {

  unsigned short conv = 0;
  
  ADCSC1 = canal;    //inicia nueva conversion
                    //sin interrupt
                    //single conversion mode
                    //ADC channel 
                    
  while (!ADCSC1_COCO); //espero que se haga la conversion
    
#ifdef RESOLUCION_8
    //8 bits
    conv = ADCRL;
#else
   //10 bits
  conv = ADCRH;
  conv <<= 8;
  conv |= ADCRL;
#endif

  return conv;
}

unsigned short Get_Temp (void)
{
	unsigned short conv = 0;
	conv = New_Sample(AN_CH26);
	return conv;
}
