#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "MCUinit.h"
#include "hard.h"
#include "timer.h"
#include "analog.h"
//#include "math.h"

#include "doonstack.h"

#define REDONDEO


//configuraciones del filtro
//#define FILTROV_CON_TOPE
#define TOPE_MEDIDA_V 5
#define I_ALWAYS_ON_CHANGE
//#define WITH_FILTER
#define FILTROI_CON_TOPE
#define I_WITHOUT_FILTER
#define DIF_MEDIDA_I 5
//#define DIF_MEDIDA_I 20
//con un filtro de 32 taps de largo tengo problemas de medición desde 1.22A para arriba
//con filtro de 8 taps tengo problemas debajo de 0.7A
//#define STEP_RESPONSE

//#define LARGO_FILTRO 8
//#define DIVISOR      3   //problemas desde 0.8A para arriba
//#define LARGO_FILTRO 16
//#define DIVISOR      4   //problemas arriba de 1.22A
#define LARGO_FILTRO 32
#define DIVISOR      5   //muy bien entre 0.2 y 1A aunque luego de un tiempo queda hasta 0.8A DIF_MEDIDA=5

#define DIF_A_KI 10
#define DIF_A_KI2 20	//KI * 2 para hysteresis
#define HYST 2
#define TIMER_HYST 5000	//50 son 10ms timer de activacion de hysteresis

//---- VARIABLES EXTERNAS ----//
unsigned int timer_standby;
unsigned int timer_1_10;
unsigned int timer_grabado;
unsigned char timer_overflow;

#ifdef POR_INTERRUPCION
volatile short pwm = 0;
#endif



//---- VARIABLES GLOBALES ----//
unsigned short vma_1a10 [LARGO_FILTRO + 1];
#ifdef I_WITHOU_FILTER
unsigned short vma_isense [LARGO_FILTRO + 1];
#endif
#ifdef FILTROV_CON_TOPE
short last_medida_1a10 = 0;
#endif
#ifdef FILTROI_CON_TOPE
short last_medida_Isense = 0;
#endif

//const char source_data[] = {"HOLA"} @0x0000E600;
char modo_func[] = {" A"};  //A normal, B fundido, C definir
//const char a @0x0000E600;   //parece que hay un problema en la forma de
                            //definir la direccion base, si lo pongo asi la toma como protegida
char a @0x0000f000;                            

unsigned short data_lenght;
//unsigned short result_ma_ant = 0;
#ifdef STEP_RESPONSE 
unsigned char step_low = 0;
#endif
unsigned short last_1a10, last_isense;
#ifdef POR_INTERRUPCION
short sum_ki2 = 0;
#else
unsigned short sum_ki = 0;
unsigned char last_ki_neg = 0;
unsigned short vsense = 0;
#endif




//---- Funciones -----//
void Flash_Check (void);
void Flash_Clock_Init (void);
void Grabar_Modo (char);
unsigned short Get_1_10 (void);
unsigned short Get_Isense (void);
unsigned short Get_V_Out (void);
unsigned short Module(short);


//---- Principal -----//
void main(void) {
	
#ifndef POR_INTERRUPCION
	unsigned short pwm = 0;	
#endif

  unsigned short med_1a10, med_isense, med_1a10_error;
  unsigned char state, isense_dither = 0;
  unsigned char flag_apagado = 0;
//#ifdef DRIVER100W_V2
  unsigned short arrancando = 0;
  unsigned short solapado = 0;
  unsigned short error= 0;
//#endif


  


  MCU_init();

  EnableInterrupts; /* enable interrupts */
  /* include your code here */
  
  //Unset Watchdog
  SOPT1_COPE = 0;
      
  //inicializo las patas
  //MOSFET_OFF;
  //PTADD |= 0xf9;  //PTA1 y PTA2 in; 0 y 3 out -> la PTA0 la utiliza el TPMCH0
  //PTBDD |= 0xff;  //PTB out
  
  PTADD |= 0xf1;  //PTA0 sal PTA1 a PTA3 in
  PTBDD |= 0xff;  //PTB out
  
  //PTBPE_PTBPE7 = 1; //pull up enable en PTB7
  //PTBPE_PTBPE4 = 1; //pull up enable
  //PTBPE_PTBPE5 = 1; //pull up enable
#ifdef DRIVER100W_V14
  OUT_OFF;
#else
  MOSFET_OFF;
#endif
  
  //inicializo timer
  RtiInit();
  
  //inicializo el timer channel para el control d
  TpmCh0Init();

  //inicializo el timer channel para el MOSFET de encendido
#ifndef DRIVER100W_V14
  TpmCh1Init();
#endif



  //inicializo el modulo timer
  MtimInit();

  //inicializo el ADC
  AdcInit(AN_PIN1 | AN_PIN2 | AN_PIN3);
    
  //espero 200ms que se estabilice la tensión de alimentación
#ifndef DRIVER100W_V14
  Gen_Arranque_Disabled();
#endif
  
  Gen_40K_Disabled();
  Wait_ms(10);

  //Wait_ms(1000);
  //while(1);
  
  //PRUEBA PWM FIJO
#ifdef CONSTANT  
  MOSFET_ON;
  Gen_40K_Enabled();
  Set_40K(700);
  while (1);
#endif

#ifdef OPEN_LOOP  
  //PRUEBA STEP RESPONSE SIN REALIM
#ifdef DRIVER100W_V14
  OUT_ON;
#else
  MOSFET_ON;
#endif
//  Gen_Arranque_Enabled();
//  Set_Arranque(2000); //si paso el valor del modulo queda siempre en 1

  Gen_40K_Enabled();
  while (1)
  {
	  /*
	  //CON 47K A MASA EN FB 3.5A -> PIN16 2.04V
	  //					 0.5A -> PIN16 2.28V
	  while (timer_standby);
	  INDICACION_OFF;
	  Set_40K(1250);
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(1100);
	  timer_standby = 10000;
	  */
	  /*
	  //CON 4K7 A MASA EN FB 3.5A -> PIN16 1.3V
	  //					 0.5A -> PIN16 2.04V
	  while (timer_standby);
	  INDICACION_OFF;
	  Set_40K(1100);
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(700);
	  timer_standby = 10000;
	  */
	  for (state = 0; state < 3; state++)
	  {
		  timer_standby = 10000;
		  while (timer_standby);
	  }
	  INDICACION_OFF;
#ifdef OPTO_KINGBRIGHT
	  //CON OPTO KINGBRIGHT Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
#ifdef GEN_4K
	  Set_40K(1092);	//1295 bajo hasta 0.32A
#else
	  Set_40K(546);	//1295 bajo hasta 0.32A
#endif
	  	  	  	  	  	//1320 bajo hasta 0.24A

	  for (state = 0; state < 3; state++)
	  {
		  timer_standby = 10000;
		  while (timer_standby);
	  }
	  INDICACION_ON;
#ifdef GEN_4K
	  //Set_40K(976);	//da 2.58A max
	  Set_40K(855);		//da 3.95A max
	  //Set_40K(735);	//da 4.84A max
	  //Set_40K(475);	//da 5.52A max
#else	  
	  Set_40K(488);
#endif
#endif

#ifdef OPTO_KINGBRIGHTV2
	  //CON OPTO KINGBRIGHTV2 Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(922);	//900 arranca 0.44
	  	  	  	  	  	//1320 bajo hasta 0.24A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(650);
	  timer_standby = 10000;
#endif

#ifdef OPTO_KINGBRIGHTV22
	  //CON OPTO KINGBRIGHTV2 Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(922);	//900 arranca 0.44
	  	  	  	  	  	//1320 bajo hasta 0.24A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(776);
	  timer_standby = 10000;
#endif
	  
#ifdef OPTO_KINGBRIGHTV21
	  //CON OPTO KINGBRIGHTV2 Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(900);	//900 arranca 0.44
	  	  	  	  	  	//1320 bajo hasta 0.24A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(650);
	  timer_standby = 10000;
#endif

#ifdef OPTO_KINGBRIGHTV3
	  //CON OPTO KINGBRIGHTV2 Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(1005);	//900 arranca 0.44
	  	  	  	  	  	//1320 bajo hasta 0.24A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(650);
	  timer_standby = 10000;
#endif

#ifdef OPTO_KINGBRIGHTV4
	  //CON OPTO KINGBRIGHTV2 Y BC546B STD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(1105);	//900 arranca 0.44
	  	  	  	  	  	//1320 bajo hasta 0.24A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(650);
	  timer_standby = 10000;
#endif

#ifdef OPTO_SHARP
	  //CON OPTO SHARP Y BC546B FAIRCHILD 
	  //CON 4K7 A MASA EN FB + RE 220	3.55A -> PIN16 1.52V
	  //					 			0.45A -> PIN16 2.34V
	  Set_40K(850);	//870 arranca en 0.2A
	  	  	  	  	//850 arranca en 0.33A  	  
	  timer_standby = 10000;
	  while (timer_standby);
	  INDICACION_ON;
	  Set_40K(40);		//480 es 3.15A
	  	  	  	  	  	//440 es 3.25A
	  timer_standby = 10000;
#endif
  }
#endif
  
#ifdef DRIVER100W_V14
  last_1a10 = 0;
  last_isense = 0;
  timer_standby = 0;
  timer_1_10 = 0;
  state = 0;  
  
  //prueba fija
  /*
  OUT_ON;
  Gen_40K_Enabled();
  Set_40K(936);
  while (1);
  */

  while (1)
  {  
     //revisar el valor de 1 a 10V, indicacion en PTB0 me dice con que frecuencia reviso
	  //ahora me devuelve 0 a 255 filtrado y en no mas de un paso por cada vez que la llamo
	  //ademas tiene un filtro que muestrea cada 6.8ms (revisar PTB0)

	  switch (state)
	  {
		  case ARRANQUE_RESET:
			  //primero dejo que la tensión de salida caiga a 18V y se estabilice
			  OUT_OFF;
			  Gen_40K_Enabled();
			  Set_40K(PWM_ZERO);
			  //Wait_ms(3500);	//OK pero lento
			  //Wait_ms(200);		//no es estable arranca con mucha corriente
			  Wait_ms(500);
			  state++;
			  break;
			  
		  case ARRANQUE_01:
			  if (timer_1_10 == 0)
			  {
				  //if (Get_1_10() > 16)	//si tengo mas de 0.6V en 1 a 10  
				  //if (Get_1_10() > 19)	//si estoy a mas del 8%
				  if (Get_1_10() > 23)	//si estoy a mas del 9%
				  {
					  state++;		
					  timer_1_10 = 10;
				  }
				  else
					  timer_1_10 = 40;
			  }
			  break;

		  case ARRANQUE_02:
			  if (!timer_1_10)	//arranco entre 0,88V y 0,44V en la pata (20V a 10V)
			  {
				  timer_1_10 = 10;	//veo cada 1ms
				  if ((Get_V_Out() < 68) && (Get_V_Out() > 34))		//para 20V
					  state++;
	  		  }
			  break;
			  
		  case ARRANQUE_03:			  
			  if (timer_1_10 == 0)
			  {
				  //28-08-15 OJO R32 ESTABA CAMBIADA POR R20!!!!
				  OUT_ON;		//PARA PRUEBAS NO OLVIDAR DESCOMENTAR!!!!!! 16-5-15
				  arrancando = 100;	//PARA PRUEBAS NO OLVIDAR DESCOMENTAR!!!!!! 16-5-15
				  //arrancando = 1000;	//PARA PRUEBAS NO OLVIDAR DESCOMENTAR!!!!!! 08-06-16
				  //arrancando = 0;	//PARA PRUEBAS COMENTAR!!!!!! 16-5-15
				  state++;
				  
				  //precargo el pwm
#ifdef DRIVER40W
				  pwm = 861;
#else
				  pwm = VALOR_TPMMOD_EDGE - 996;		//Arranque OK partida 16-5-15
				  //pwm = VALOR_TPMMOD_EDGE - 1112;		//Arranque OK partida 16-3-15				  
#endif
				  	  	  	  	  	  	  	  	  	  	
				  
				  //PRUEBA TRABO EL PROGRAMA
				  //CON UN SET 900 ARRABCO ALREDEDOR DE 0,70A y voy hasta 1,2A
				  //Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
				  /*
				  while (1)
				  {
					  if ((Get_1_10() > 22) && (arrancando == 0))
					  {
						  OUT_ON;
						  arrancando = 1;
						  Set_40K(900);
					  }
					  else if ((Get_1_10() < 18) && (arrancando == 1))
					  {
						  OUT_OFF;
						  arrancando = 0;
						  Set_40K(0);
					  }
				  }
				  */
						  
				  
			  }
			  break;
			  
	  	  case TOMO_MUESTRA_1A10:
	  		  //tomo la medida
	  		  //tomo una muestra cada 10ms y ademas me viene filtrada
	  		  if (timer_1_10 == 0)	//200us cada tick
	  	      {
	  	    	  //INDICACION_ON;	//me sirve para ver cada cuanto estoy haciendo la medicion
	  	    	  med_1a10 = Get_1_10();
	  	    	  med_1a10_error = med_1a10;
	  	    	  if (arrancando)
	  	    	  {
	  	    		  timer_1_10 = 5;	//mejora bastante	500us
	  	    		  arrancando--;
	  	    	  }
	  	    	  else
	  	    		timer_1_10 = 40;	//mejora bastante	 4ms 	    	  

	  	          //fijo un maximo de corriente
	  	          if (med_1a10 > MAX_1A10)
	  	        	  med_1a10 = MAX_1A10;
	  	          
	  	          //fijo un minimo de corriente en 0.32A
	  	          if (med_1a10 < 22)
	  	        	  med_1a10 = 22;
	  	          
	  	          if (last_1a10 != med_1a10)
	  	          {
	  	        	//CAMBIO_ON_1A10;
	  	        	last_1a10 = med_1a10;
	  	          }
	  	    	  state = TOMO_MUESTRA_ISENSE;
	  	      }
	  		  break;

	  	  case TOMO_MUESTRA_ISENSE:
  	    	  med_isense = Get_Isense();
  	    	  state = HAGO_CAMBIOS;
	  		  break;
	  	     
	  	  case HAGO_CAMBIOS:	  	       	    
		  	  //No quiero que se mueva mas de 1 punto (de los 200 posibles) cada 10ms
	  		  //muevo de a un punto, pero cuando estoy cerca le agrego el termino integral
	  		  //OJO trabaja negado a mas pwm mas corriente sal

	  		  if (med_1a10 != med_isense)
	  		  {
					  //if (med_1a10 > (med_isense + DIF_A_KI))
					  if ((med_1a10 - DIF_A_KI) > med_isense)		//ver que el minimo no llegue a 0
					  {
						  pwm++;
						  sum_ki = 0;
					  }
					  else if (med_1a10 > med_isense)				//activo el termino integral
					  {
						  if (last_ki_neg == 0)
						  {
							  sum_ki++;
							  if (sum_ki > 128)
							  //if (sum_ki > 32)		  			  
							  {
								  if (CAMBIO_ISENSE)
									  CAMBIO_OFF_ISENSE;
								  else
									  CAMBIO_ON_ISENSE;
						
								  pwm++;
								  sum_ki = 0;
							  }
						  }
						  else
						  {
							  sum_ki = 0;
							  last_ki_neg = 0;
						  }
					  }
						
					  //else if (med_1a10 < (med_isense - DIF_A_KI))
					  if ((med_1a10 + DIF_A_KI) < med_isense)	//ver que el maximo no supere los 255
					  {
						  if (pwm)
							  pwm--;
						  sum_ki = 0;
					  }
					  else if (med_1a10 < med_isense)	//activo el termino integral
					  {
						  if (last_ki_neg == 1)
						  {
							  sum_ki++;
							  if (sum_ki > 128)
							  //if (sum_ki > 32)
							  {
								  if (CAMBIO_1A10)
									  CAMBIO_OFF_1A10;
								  else
									  CAMBIO_ON_1A10;
		
								if (pwm)
									pwm--;
								sum_ki = 0;
							  } 			  
						  }
						  else
						  {
							  sum_ki = 0; 
							  last_ki_neg = 1;
						  }
					  }

//					  if (pwm < PWM_ARRANQUE2)
//						  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
//					  else
//						  pwm = PWM_ARRANQUE2;		//modificacion 24-05-2016 soluciona problema
//					  //cuando la pata 15 (1 a 10V) es distinta de la 14 (Current sense)
//					  //despues de un tiempo se apaga y vuelve a prender

					  if (pwm < PWM_MAX_IN_FUNC)
						  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
					  else
						  pwm = PWM_MAX_IN_FUNC;		//modificacion 24-05-2016 soluciona problema
					  	  	  	  	  	  	 	 	//cuando la pata 15 (1 a 10V) es distinta de la 14 (Current sense)
					  	  	  	  	  	  	  	  	//despues de un tiempo se apaga y vuelve a prender
	  		  }
			  else	//deben ser iguales
			  {
				  if (INDICACION)
					  INDICACION_OFF;
				  else
					  INDICACION_ON;

				  sum_ki = 0;
				  last_ki_neg = 0;
			  }

//	  		  }  		  //si son iguales no actualizo el pwm

/*			  	  
				  if ((med_1a10 - DIF_A_KI) > med_isense)		//ver que el minimo no llegue a 0
				  {
					  pwm++;
					  sum_ki = 0;
				  }
				  //else if (med_1a10 < (med_isense - DIF_A_KI))
				  else if ((med_1a10 + DIF_A_KI) < med_isense)	//ver que el maximo no supere los 255
				  {
					  if (pwm)
						  pwm--;
					  sum_ki = 0;
				  }
				  else if (med_1a10 > med_isense)	//activo el termino integral
				  {
					  if (last_ki_neg == 0)
					  {
						  sum_ki++;
						  //if (sum_ki > 128)
						  if (sum_ki > 32)		  			  
						  {
							  if (CAMBIO_ISENSE)
								  CAMBIO_OFF_ISENSE;
							  else
								  CAMBIO_ON_ISENSE;
	
							pwm++;
							sum_ki = 0;
						  }
					  }
					  else
					  {
						  sum_ki = 0;
						  last_ki_neg = 0;
					  }
				  }
				  else if (med_1a10 < med_isense)	//activo el termino integral
				  {
					  if (last_ki_neg == 1)
					  {
						  sum_ki++;
						  //if (sum_ki > 128)
						  if (sum_ki > 32)
						  {
							  if (CAMBIO_1A10)
								  CAMBIO_OFF_1A10;
							  else
								  CAMBIO_ON_1A10;
	
							if (pwm)
								pwm--;
							sum_ki = 0;
						  } 			  
					  }
					  else
					  {
						  sum_ki = 0; 
						  last_ki_neg = 1;
					  }
				  }
				  else	//deben ser iguales
				  {
					  if (INDICACION)
						  INDICACION_OFF;
					  else
						  INDICACION_ON;
	
					  sum_ki = 0;
					  last_ki_neg = 0;
				  }
				  
			  	  if (pwm < PWM_ARRANQUE2)
			  		  Set_40K(PWM_ARRANQUE2 - pwm);	//sin pnp en pata 16		  	  		  	  
*/			  	  
	  	      
		  	  state = TOMO_MUESTRA_1A10;
	  	      //Apago todos los flags
	  	      //INDICACION_OFF;
	  	      //CAMBIO_OFF_1A10;
	  	      //CAMBIO_OFF_ISENSE;
		  	  break;

	  	  case V1A10_FAIL_01:
	  		  if ((Get_1_10() > 25) && (timer_1_10 == 0))
	  		  {
		  			INDICACION_OFF;
		  			state = ARRANQUE_RESET;					
	  		  }
	  		  break;

	  	  default:
	  		  state = ARRANQUE_RESET;
	  		  break;
	  }
	  //Resolver cuestiones que no tengan que ver con muestras ACA
	  //APAGADO POR TENSION
	
	  if ((!timer_standby) && (state != V1A10_FAIL_01)
		&& (state != ARRANQUE_RESET)
		&& (state != ARRANQUE_01)
		&& (state != ARRANQUE_02)
		&& (state != ARRANQUE_03))
		  {
		   	  if (med_1a10_error < 20) 
			  {				  
				  timer_1_10 = 10000;	//3 seg
				  OUT_OFF;	  
				  Gen_40K_Disabled();				
				  state = V1A10_FAIL_01;
				  INDICACION_ON;
				  med_1a10_error = 255;		//para que no vuelva a entrar al toque
			  }
	  	  }
		  
  }//cierra while 1    
}//cierra main
#endif //DRIVER100W_V14

#ifdef DRIVER40W
  last_1a10 = 0;
  last_isense = 0;
  timer_standby = 0;
  timer_1_10 = 0;
  state = 0;  
  
  //prueba fija
  /*
  OUT_ON;
  Gen_40K_Enabled();
  Set_40K(936);
  while (1);
  */

  while (1)
  {  
     //revisar el valor de 1 a 10V, indicacion en PTB0 me dice con que frecuencia reviso
	  //ahora me devuelve 0 a 255 filtrado y en no mas de un paso por cada vez que la llamo
	  //ademas tiene un filtro que muestrea cada 6.8ms (revisar PTB0)

	  switch (state)
	  {
		  case ARRANQUE_RESET:
			  //primero dejo que la tensión de salida caiga a 18V y se estabilice
			  OUT_OFF;
			  Gen_40K_Enabled();
			  Set_40K(PWM_ZERO);
			  //Wait_ms(3500);	//OK pero lento
			  //Wait_ms(200);		//no es estable arranca con mucha corriente
			  Wait_ms(500);
			  state++;
			  break;
			  
		  case ARRANQUE_01:
			  if (timer_1_10 == 0)
			  {
				  //if (Get_1_10() > 16)	//si tengo mas de 0.6V en 1 a 10  
				  //if (Get_1_10() > 19)	//si estoy a mas del 8%
				  if (Get_1_10() > 23)	//si estoy a mas del 9%
				  {
					  state++;		
					  timer_1_10 = 10;
				  }
				  else
					  timer_1_10 = 40;
			  }
			  break;

		  case ARRANQUE_02:
			  if (!timer_1_10)	//arranco entre 0,88V y 0,44V en la pata (20V a 10V)
			  {
				  timer_1_10 = 10;	//veo cada 1ms
				  if ((Get_V_Out() < 68) && (Get_V_Out() > 34))		//para 20V
				  {
					  //arrancando = 500;	//ok pero hace como un pestaneo
					  arrancando = 300;	//
					  state++;
					  OUT_ON;
					  //pwm = VALOR_TPMMOD_EDGE - 1035;	//para dif 835
					  //pwm = VALOR_TPMMOD_EDGE - 835;	//para dif 835 vuelve menos pero tarda en subir
					  //pwm = VALOR_TPMMOD_EDGE - 735;	//para dif 735 vuelve menos
					  //pwm = VALOR_TPMMOD_EDGE - 635;	//para dif 635 vuelve menos
					  pwm = VALOR_TPMMOD_EDGE - 200;	//el numero es el pwm activo
					  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
				  }
	  		  }
			  break;
			  
		  case ARRANQUE_03:			  //durante 300ms dejo el control en minimo
			  if (!timer_1_10)
			  {
				  if (arrancando)
				  {
					  timer_1_10 = 10;	//mejora bastante	1ms
					  arrancando--;
				  }
				  else
				  {
					  state = ARRANQUE_04;
					  arrancando = 344;
					  solapado = 200;
				  }
			  }
			  break;

		  case ARRANQUE_04:			  //
			  if (!timer_1_10)
			  {
				  if (arrancando)
				  {
					  timer_1_10 = 10;	//mejora bastante	1ms
					  arrancando--;
					  solapado += 1;			//solapado va desde 200 (que es el arranque)
					  	  	  	  	  	  	  	//hasta 544 que es PWM_MAX_IN_FUNC  
					  pwm = VALOR_TPMMOD_EDGE - solapado;	//el numero es el pwm activo
					  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
				  }
				  else
				  {
					  state = TOMO_MUESTRA_1A10;
					  arrancando = 2000;
					  //pwm = VALOR_TPMMOD_EDGE - 635;	//este solapa ok
				  }
			  }
			  break;

	  	  case TOMO_MUESTRA_1A10:	//durante 500ms hago un update muy lento
	  		  //tomo la medida
	  		  //tomo una muestra cada 10ms y ademas me viene filtrada
	  		  if (timer_1_10 == 0)	//200us cada tick
	  	      {
	  	    	  //INDICACION_ON;	//me sirve para ver cada cuanto estoy haciendo la medicion
	  	    	  med_1a10 = Get_1_10();
	  	    	  med_1a10_error = med_1a10;

				  if (arrancando)
				  {
					  //timer_1_10 = 200;	//update cada 1ms pico de 1.58A
					  timer_1_10 = 10;	//update cada 1ms pico de 1.58A
					  arrancando--;
				  }
				  else
				  {
		  	    	  timer_1_10 = 40;	//mejora bastante	 4ms
				  }

	  	          //fijo un maximo de corriente
	  	          if (med_1a10 > MAX_1A10)
	  	        	  med_1a10 = MAX_1A10;
	  	          
	  	          //fijo un minimo de corriente en 0.32A
	  	          if (med_1a10 < 22)
	  	        	  med_1a10 = 22;
	  	          
	  	          if (last_1a10 != med_1a10)
	  	          {
	  	        	//CAMBIO_ON_1A10;
	  	        	last_1a10 = med_1a10;
	  	          }
	  	    	  state = TOMO_MUESTRA_ISENSE;
	  	      }
	  		  break;

	  	  case TOMO_MUESTRA_ISENSE:
  	    	  med_isense = Get_Isense();
  	    	  state = HAGO_CAMBIOS;
	  		  break;
	  	     
	  	  case HAGO_CAMBIOS:	  	       	    
		  	  //No quiero que se mueva mas de 1 punto (de los 200 posibles) cada 10ms
	  		  //muevo de a un punto, pero cuando estoy cerca le agrego el termino integral
	  		  //OJO trabaja negado a mas pwm mas corriente sal

//	  		  if (arrancando)
//	  		  {
//	  			  //lo paso a PID
//	  			  if (med_1a10 > med_isense)
//	  				  pwm += (med_1a10 - med_isense) >> 3;
//	  			  else if (med_1a10 < med_isense)
//	  				  pwm -= (med_isense - med_1a10) >> 3;
//	  			  
//				  if (pwm < PWM_MAX_IN_FUNC)
//					  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
//				  else
//					  pwm = PWM_MAX_IN_FUNC;		//modificacion 24-05-2016 soluciona problema
//	  		  }
//	  		  else if (med_1a10 != med_isense)
	  		  if (med_1a10 != med_isense)
	  		  {
					  //if (med_1a10 > (med_isense + DIF_A_KI))
					  if ((med_1a10 - DIF_A_KI) > med_isense)		//ver que el minimo no llegue a 0
					  {
						  pwm++;
						  sum_ki = 0;
					  }
					  else if (med_1a10 > med_isense)				//activo el termino integral
					  {
						  if (last_ki_neg == 0)
						  {
							  sum_ki++;
							  if (sum_ki > 128)
							  //if (sum_ki > 32)		  			  
							  {
								  if (CAMBIO_ISENSE)
									  CAMBIO_OFF_ISENSE;
								  else
									  CAMBIO_ON_ISENSE;
						
								  pwm++;
								  sum_ki = 0;
							  }
						  }
						  else
						  {
							  sum_ki = 0;
							  last_ki_neg = 0;
						  }
					  }
						
					  //else if (med_1a10 < (med_isense - DIF_A_KI))
					  if ((med_1a10 + DIF_A_KI) < med_isense)	//ver que el maximo no supere los 255
					  {
						  if (pwm)
							  pwm--;
						  sum_ki = 0;
					  }
					  else if (med_1a10 < med_isense)	//activo el termino integral
					  {
						  if (last_ki_neg == 1)
						  {
							  sum_ki++;
							  if (sum_ki > 128)
							  //if (sum_ki > 32)
							  {
								  if (CAMBIO_1A10)
									  CAMBIO_OFF_1A10;
								  else
									  CAMBIO_ON_1A10;
		
								if (pwm)
									pwm--;
								sum_ki = 0;
							  } 			  
						  }
						  else
						  {
							  sum_ki = 0; 
							  last_ki_neg = 1;
						  }
					  }

					  if (pwm < PWM_MAX_IN_FUNC)
						  Set_40K(VALOR_TPMMOD_EDGE - pwm);	//sin pnp en pata 16
					  else
						  pwm = PWM_MAX_IN_FUNC;		//modificacion 24-05-2016 soluciona problema
					  	  	  	  	  	  	 	 	//cuando la pata 15 (1 a 10V) es distinta de la 14 (Current sense)
					  	  	  	  	  	  	  	  	//despues de un tiempo se apaga y vuelve a prender
	  		  }
			  else	//deben ser iguales
			  {
				  if (INDICACION)
					  INDICACION_OFF;
				  else
					  INDICACION_ON;

				  sum_ki = 0;
				  last_ki_neg = 0;
			  }
	  	      
		  	  state = TOMO_MUESTRA_1A10;
		  	  break;

	  	  case V1A10_FAIL_01:
	  		  if ((Get_1_10() > 25) && (timer_1_10 == 0))
	  		  {
		  			INDICACION_OFF;
		  			state = ARRANQUE_RESET;					
	  		  }
	  		  break;

	  	  default:
	  		  state = ARRANQUE_RESET;
	  		  break;
	  }
	  //Resolver cuestiones que no tengan que ver con muestras ACA
	  //APAGADO POR TENSION
	
	  if ((!timer_standby) && (state != V1A10_FAIL_01)
		&& (state != ARRANQUE_RESET)
		&& (state != ARRANQUE_01)
		&& (state != ARRANQUE_02)
		&& (state != ARRANQUE_03))
		  {
		   	  if (med_1a10_error < 20) 
			  {				  
				  timer_1_10 = 10000;	//3 seg
				  OUT_OFF;	  
				  Gen_40K_Disabled();				
				  state = V1A10_FAIL_01;
				  INDICACION_ON;
				  med_1a10_error = 255;		//para que no vuelva a entrar al toque
			  }
	  	  }
		  
  }//cierra while 1    
}//cierra main
#endif //DRIVER40W

#ifdef POR_INTERRUPCION
  last_1a10 = 0;
  last_isense = 0;
  timer_standby = 0;
  timer_1_10 = 0;
  state = 0;
  while (1)
  {  
     //revisar el valor de 1 a 10V, indicacion en PTB0 me dice con que frecuencia reviso
	  //ahora me devuelve 0 a 255 filtrado y en no mas de un paso por cada vez que la llamo
	  //ademas tiene un filtro que muestrea cada 6.8ms (revisar PTB0)

	  switch (state)
	  {
		  case ARRANQUE_RESET:
			  //primero dejo que la tensión de salida caiga a 18V
			  Gen_Arranque_Disabled();
			  Gen_40K_Enabled();
			  Set_40K(PWM_ARRANQUE2);
			  timer_1_10 = 5500;	//cuenta cada 100us
			  
			  state++;
			  break;

		  case ARRANQUE_01:
			  if (!timer_1_10)
			  {
				  //espero que haya como mínimo 0.6V en 1 a 10V
				  //el divisor desde 1 a 10 es 3.06
				  //0.6 / 3.06 = 0.196V
				  //0.196V / 0.013V = 15.09
	  			  timer_1_10 = 5;
				  if (Get_1_10() > 16)
				  {
					  //esta prendiendo el sistema
					  Set_40K(PWM_MIN);
					  
					  //prendo el mosfet de salida
					  Gen_Arranque_Enabled();
					  Set_Arranque(2000); //si paso el valor del modulo queda siempre en 1
					  timer_1_10 = 250;	//espero 25ms con el pwm al minimo
					  state++;
				  }
	  		  }
			  break;

		  case ARRANQUE_02:
			  //Espero que se estabilice el pwm 25ms al minimo
			  if (timer_1_10 == 0)
			  {
				  state++;
				  timer_1_10 = 10;
				  pwm = 0;
			  }
			  break;
			  
		  case ARRANQUE_03:			  
			  if (timer_1_10 == 0)
			  {
				  state++;
			  }
			  break;
			  
	  	  case TOMO_MUESTRA_1A10:
	  		  //tomo la medida
	  		  //tomo una muestra cada 10ms y ademas me viene filtrada
	  		  if (timer_1_10 == 0)	//500us cada tick
	  	      {
	  	    	  INDICACION_ON;	//me sirve para ver cada cuanto estoy haciendo la medicion
	  	    	  med_1a10 = Get_1_10();
	  	    	  //med_1a10 = 255;	//le fijo la referencia al 100%
	  	    	  //med_1a10 = 241;	//le fijo la referencia al 95%
	  	    	  //med_1a10 = 192;	//le fijo la referencia al 75%
	  	    	  //med_1a10 = 128;	//le fijo la referencia al 50%
	  	    	  //med_1a10 = 60;	//le fijo la referencia al 25%
	  	    	  //med_1a10 = 32;	//le fijo la referencia al 12.5%
	  	    	  //med_1a10 = 16;	//le fijo la referencia al 6.25%
#ifdef STEP_RESPONSE
	  	    	  if (step_low)
	  	    	  {
	  	    		  med_1a10 = 18;	//le fijo la referencia al 6.25%
	  	    	  }
	  	    	  else
	  	    	  {
	  	    		  med_1a10 = 64;	//le fijo la referencia al 6.25%
	  	    	  }
#endif	  	    		  
	  	          timer_1_10 = 22;
	  	          
	  	          if (med_1a10 > MAX_1A10)
	  	        	  med_1a10 = MAX_1A10;
	  	          
	  	          
	  	          if (last_1a10 != med_1a10)
	  	          {
	  	        	CAMBIO_ON_1A10;
	  	        	last_1a10 = med_1a10;
	  	          }
	  	    	  state++;
	  	      }
	  		  break;

	  	  case TOMO_MUESTRA_ISENSE:
  	    	  med_isense = Get_Isense();
  	    	  state++;
	  		  break;

	  	  case VEO_MODIFICACIONES:
#ifdef I_ALWAYS_ON_CHANGE	  		  
	  		  state++;
#else
  			  if (med_1a10 != med_isense)
  				  state++;
  			  else
  			  {
  				  state = TOMO_MUESTRA_1A10;
  				  //Apago todos los flags
  				  INDICACION_OFF;
  				  CAMBIO_OFF_1A10;
  				  CAMBIO_OFF_ISENSE;  				  
  			  }

#endif
	  	     break;
	  	     
	  	  case HAGO_CAMBIOS:	  	       	    
		  	  //No quiero que se mueva mas de 1 punto (de los 200 posibles) cada 10ms
	  		  //muevo de a un punto, pero cuando estoy cerca le agrego el termino integral
	  		  //OJO trabaja negado a mas pwm mas corriente sal

	  		  /*
	  		  //last_isense = pwm;
		  	  if (med_1a10 > (med_isense + DIF_A_KI))
		  	  {
		  		  DisableInterrupts;
		  		  pwm = med_1a10 - med_isense;
		  		  EnableInterrupts;
		  		  
		  		  //CAMBIO_ON_ISENSE;
		  		  sum_ki = 0;
		  	  }
		  	  else if (med_1a10 < (med_isense - DIF_A_KI))
		  	  {
		  		  DisableInterrupts;
		  		  pwm = med_isense - med_1a10;
		  		  EnableInterrupts;
		  		  
		  		  //CAMBIO_ON_ISENSE;
		  		  sum_ki = 0;
		  	  }
		  	  else if (med_1a10 > med_isense)	//activo el termino integral
		  	  {
	  			  if (!last_ki_neg)
	  			  {
		  			  sum_ki++;
		  			  if (sum_ki > 128)
		  			  {
		  				CAMBIO_ON_ISENSE;
		  				DisableInterrupts;
			  			pwm++;
		  				EnableInterrupts;

		  				sum_ki = 0;
		  			  }
	  			  }
	  			  else
	  			  {
	  				  sum_ki = 0;
	  				  last_ki_neg = 0;
	  			  }
		  	  }
		  	  else if (med_1a10 < med_isense)	//activo el termino integral
		  	  {
	  			  if (last_ki_neg)
	  			  {
		  			  sum_ki++;
		  			  if (sum_ki > 128)
		  			  {
		  				CAMBIO_ON_ISENSE;
		  				DisableInterrupts;
		  				pwm--;
		  				EnableInterrupts;
		  				sum_ki = 0;
		  			  } 			  
	  			  }
	  			  else
	  			  {
	  				  sum_ki = 0; 
	  				  last_ki_neg = 1;
	  			  }
	  		  }
		  	  //sino deben ser iguales o algo asi
		  	  */
	  		  
/*	  		  
	  		  //TODO: poner hyst aca
	  		  //me fijo si se activo el termino integral
	  		  if ((Module(med_1a10 - med_isense) < DIF_A_KI2) && (sum_ki2 != 0))
	  		  {
				  //uso termino integral
				  pwm = 0;
				  if ((med_1a10 - med_isense) > 0)	//activo el termino integral
				  {
					  sum_ki2++;
					  if (sum_ki2 > 111)
					  {
						CAMBIO_ON_ISENSE;
						DisableInterrupts;
						pwm++;
						EnableInterrupts;
						sum_ki2 = 1;
					  }
				  }
				  else if ((med_1a10 - med_isense) < 0)	
				  {
					   sum_ki2--;
					   if (sum_ki2 < -111)
					   {
						   CAMBIO_ON_ISENSE;
						   DisableInterrupts;
						   pwm--;
						   EnableInterrupts;
						   sum_ki2 = -1;
						}
				  }	  			  
				  else	//deben ser iguales
				  {
					  sum_ki2 = 1;
				  }
	  		  }
	  		  else	//NO TENIA TERMINO INTEGRAL POR AHORA
	  		  {
				  if (Module(med_1a10 - med_isense) < DIF_A_KI)
				  {
					  //activo termino integral
					  pwm = 0;
					  if ((med_1a10 - med_isense) > 0)	//activo el termino integral
					  	  sum_ki2++;
					  else
						  sum_ki2--;
				  }
				  else
				  {
					  CAMBIO_ON_1A10;
					  sum_ki2 = 0;
					  DisableInterrupts;
					  pwm = med_1a10 - med_isense;	//si med_isense es mayor pwm es negativo
					  EnableInterrupts;
				  }
	  		  }
*/	  			  
	  		  //TODO: poner hyst aca
	  		  //me fijo si se activo el termino integral
	  		  if (Module(med_1a10 - med_isense) < DIF_A_KI)
	  		  {
				  //uso termino integral
				  pwm = med_1a10 - med_isense;				  
				  if (pwm == 0)
				  {
					  sum_ki2 = 0;
				  }
				  
				  if (pwm < 0)	
				  {
					  sum_ki2--;
					  if (sum_ki2 < -110)
					  {
						  CAMBIO_ON_ISENSE;
						  DisableInterrupts;
						  pwm = -1;
						  EnableInterrupts;
						  sum_ki2 = 0;
					  }
				  }	  			  
				  
				  if (pwm > 0)
				  {
					  sum_ki2++;
					  if (sum_ki2 > 110)
					  {
						CAMBIO_ON_ISENSE;
						DisableInterrupts;
						pwm = 1;
						EnableInterrupts;
						sum_ki2 = 0;
					  }
				  }
	  		  }
	  		  else	//NO TENIA TERMINO INTEGRAL POR AHORA
	  		  {
				  CAMBIO_ON_1A10;
				  sum_ki2 = 0;
				  DisableInterrupts;
				  pwm = med_1a10 - med_isense;	//si med_isense es mayor pwm es negativo
				  EnableInterrupts;
	  		  }
	  		  
	  		  /*
		  	  if (med_1a10 != med_isense)
		  	  {
		  		  DisableInterrupts;
		  		  pwm = med_1a10 - med_isense;	//si med_isense es mayor pwm es negativo
		  		  EnableInterrupts;		  		  
		  	  }
			  */
	  	      
		  	  state = TOMO_MUESTRA_1A10;
		  	  break;
		  	  
	  	  case ARRANQUE_FAIL_01:
	  		  
	  		  if (!timer_1_10)
	  		  {
	  			//INDICACION_ON;
		  		  //la corriente estuvo en 0 durante 250ms
		  		  //Set_40k(PWM_MIN);
	  			/*
		  		  timer_1_10 = 100;	//1 ms
		  		  
		  		  if (Get_Isense() > 6)
		  		  {
		  			if (flag_apagado)
		  				flag_apagado--;
		  			else
		  			{
		  				//el flag_apagado vale 0 porque la corriente comenzo a crecer
		  				state = ARRANQUE_03;
		  				//pwm = 20;
		  				pwm = 5;
		  				INDICACION_OFF;
		  			}	  			  
		  		  }
		  		  */
	  			state = ARRANQUE_RESET;
	  		  }
	  		  
	  		  //Wait_ms(3000);
	  		  //state = ARRANQUE_RESET;
	  		  break;
		  	     
	  	  default:
	  		  state = ARRANQUE_RESET;
	  		  break;
	  }
	  //Resolver cuestiones que no tengan que ver con muestras ACA
	  

#ifdef STEP_RESPONSE	  
	  if (!timer_standby)
	  {
		  timer_standby = 400;
		  if (step_low)
			  step_low = 0;
		  else
			  step_low = 1;
	  }
#endif
	  

	  //reviso el estado de apagado
	  //si veo la corriente cerca de 0 durante 250ms espero 3 segundos y reseteo	  
	  if ((!timer_standby) && (state != ARRANQUE_FAIL_01)
			  && (state != ARRANQUE_RESET)
			  && (state != ARRANQUE_01)
			  && (state != ARRANQUE_02)
			  && (state != ARRANQUE_03))
	  {
		  if (med_isense <= 10)
		  {
			  flag_apagado++;
		  }
		  else if (flag_apagado)
			  flag_apagado--;
			  
		  timer_standby = 100;	//int cada 100us reviso cada 10ms
		  
		  if (flag_apagado > 25)
		  {
			  //si entro aca desconecto todo y espero 3 segundos
			  //INDICACION_ON;
			  //Set_40K(PWM_MIN_ARRANQUE_FAIL);
			  flag_apagado = 0;
			  state = ARRANQUE_FAIL_01;
			  timer_1_10 = 30000;	//3 seg
			  Gen_Arranque_Disabled();			  
			  Gen_40K_Disabled();


		  }
	  }

      //Apago todos los flags
      INDICACION_OFF;
      CAMBIO_OFF_1A10;
      CAMBIO_OFF_ISENSE;

  }//cierra while 1    
}//cierra main
  
#endif //POR_INTERRUPCION

#if defined DRIVER100W || defined DRIVER100W_V2
  last_1a10 = 0;
  last_isense = 0;
  timer_standby = 0;
  timer_1_10 = 0;
  state = 0;
  while (1)
  {  
     //revisar el valor de 1 a 10V, indicacion en PTB0 me dice con que frecuencia reviso
	  //ahora me devuelve 0 a 255 filtrado y en no mas de un paso por cada vez que la llamo
	  //ademas tiene un filtro que muestrea cada 6.8ms (revisar PTB0)

	  switch (state)
	  {
		  case ARRANQUE_RESET:
			  //primero dejo que la tensión de salida caiga a 18V
			  Gen_Arranque_Disabled();
			  Gen_40K_Enabled();
			  //Set_40K(PWM_ARRANQUE2);
			  Set_40K(VALOR_TPMMOD_EDGE);	
			  //timer_1_10 = 9700;	//7600 fue mas estable que 9700			  
			  //timer_1_10 = 7100;	//7600 fue mas estable que 9700 7100 produccion PARPADEA
#ifndef DRIVER100W_V2
			  timer_1_10 = 7600;	//agrego algunos milisegundos mas
			  //timer_1_10 = 7100;	//7600 fue mas estable que 9700 PARPADEA
			  //timer_1_10 = 5500;	//7600 fue mas estable que 9700
			  arrancando = 500;
#else	  
			  timer_1_10 = 100;	//7600 fue mas estable que 9700 PARPADEA
			  arrancando = 700;
#endif			  
			  state++;
			  break;
			  
		  case ARRANQUE_01:
			  if (timer_1_10 == 0)
			  {
				  //if (Get_1_10() > 16)	//si tengo mas de 0.6V en 1 a 10  
				  //if (Get_1_10() > 19)	//si estoy a mas del 8%
				  if (Get_1_10() > 23)	//si estoy a mas del 9%
					  state++;				  

				  timer_1_10 = 40;
			  }
			  break;

		  case ARRANQUE_02:
			  if (!timer_1_10)	//ahora esta alrededor de 16V de alimentacion
			  {
#ifndef DRIVER100W_V2
				  state++;
				  Gen_Arranque_Enabled();
				  Set_Arranque(2000); //si paso el valor del modulo queda siempre en 1
				  Set_40K(PWM_MIN);
				  //Set_40K(PWM_MITAD);
				  pwm = 0;
				  timer_1_10 = 40;

#else				
				  timer_1_10 = 10;	//veo cada 1ms
				  //if (Get_V_Out() > 56)	//si tengo mas de 16V de salida
				  if (Get_V_Out() > 70)	//si tengo mas de 20V de salida
				  {				  	  
					  if (pwm)
						  pwm--;
				  }		
				  else
				  {
					  if (pwm < PWM_ARRANQUE2)
						  pwm++;					 
				  }
				  Set_40K(PWM_ARRANQUE2 - pwm);	//sin pnp en pata 16
				  
				  //si estoy al +/-10% sigo
				  //if ((Get_V_Out() < 61) && (Get_V_Out() > 51))	//para 16V
				  if ((Get_V_Out() < 77) && (Get_V_Out() > 63))		//para 20V
					  state++;
#endif

	  		  }
			  break;
			  
		  case ARRANQUE_03:			  
			  if (timer_1_10 == 0)
			  {
				  Gen_Arranque_Enabled();
				  Set_Arranque(2000); //si paso el valor del modulo queda siempre en 1
				  timer_standby = 3000;
				  state++;
			  }

			  break;
			  
	  	  case TOMO_MUESTRA_1A10:
	  		  //tomo la medida
	  		  //tomo una muestra cada 10ms y ademas me viene filtrada
	  		  if (timer_1_10 == 0)	//200us cada tick
	  	      {
	  	    	  INDICACION_ON;	//me sirve para ver cada cuanto estoy haciendo la medicion
	  	    	  med_1a10 = Get_1_10();
	  	    	  med_1a10_error = med_1a10;
#ifdef STEP_RESPONSE
	  	    	  if (step_low)
	  	    	  {
	  	    		  med_1a10 = 18;	//le fijo la referencia al 6.25%
	  	    	  }
	  	    	  else
	  	    	  {
	  	    		  med_1a10 = 64;	//le fijo la referencia al 6.25%
	  	    	  }
#endif	  	    		  
	  	    	  //timer_1_10 = 5;	//salta cuando baja
	  	    	  //timer_1_10 = 10;
	  	    	  //timer_1_10 = 20;	//mejora bastante
#ifndef DRIVER100W_V2
	  	    	  if (arrancando)
	  	    	  {
	  	    		  timer_1_10 = 5;
	  	    		  arrancando--;
	  	    	  }
	  	    	  else
	  	    		  timer_1_10 = 40;
#else
	  	    	  if (arrancando)
	  	    	  {
	  	    		  timer_1_10 = 5;	//mejora bastante
	  	    		  arrancando--;
	  	    	  }
	  	    	  else
	  	    		timer_1_10 = 40;	//mejora bastante	  	    	  
#endif
	  	    	  
#ifdef DRIVER60W
	  	    	  //de la medicion dejo que llegue hasta 240 pero por corriente debe ser maximo 178	  	    	  
	  	          if (med_1a10 > MAX_1A10)
	  	        	  med_1a10 = MAX_1A10;

	  	    	  med_1a10 = med_1a10 * 8;
	  	    	  med_1a10 = med_1a10 / 10;
	  	          
#endif
#ifdef DRIVER100W	  	    	  
	  	          //fijo un maximo de corriente
	  	          if (med_1a10 > MAX_1A10)
	  	        	  med_1a10 = MAX_1A10;
	  	          
	  	          //fijo un minimo de corriente en 0.32A
	  	          if (med_1a10 < 22)
	  	        	  med_1a10 = 22;
#endif
	  	          
	  	          if (last_1a10 != med_1a10)
	  	          {
	  	        	//CAMBIO_ON_1A10;
	  	        	last_1a10 = med_1a10;
	  	          }
	  	    	  state++;
	  	      }
	  		  break;

	  	  case TOMO_MUESTRA_ISENSE:
  	    	  med_isense = Get_Isense();
  	    	  state++;
	  		  break;

	  	  case VEO_MODIFICACIONES:
#ifdef I_ALWAYS_ON_CHANGE	  		  
	  		  state++;
#else
  			  if (med_1a10 != med_isense)
  				  state++;
  			  else
  			  {
  				  state = TOMO_MUESTRA_1A10;
  				  //Apago todos los flags
  				  INDICACION_OFF;
  				  CAMBIO_OFF_1A10;
  				  CAMBIO_OFF_ISENSE;  				  
  			  }

#endif
	  	     break;
	  	     
	  	  case HAGO_CAMBIOS:	  	     
#ifdef I_ALWAYS_ON_CHANGE  	    
		  	  //No quiero que se mueva mas de 1 punto (de los 200 posibles) cada 10ms
	  		  //muevo de a un punto, pero cuando estoy cerca le agrego el termino integral
	  		  //OJO trabaja negado a mas pwm mas corriente sal

	  		  //last_isense = pwm;
		  	  if (med_1a10 > (med_isense + DIF_A_KI))
		  	  {
		  		  pwm++;
		  		  //CAMBIO_ON_ISENSE;
		  		  CAMBIO_ON_1A10;
		  		  sum_ki = 0;
		  	  }
		  	  else if (med_1a10 < (med_isense - DIF_A_KI))
		  	  {
		  		  if (pwm)
		  			  pwm--;
		  		  CAMBIO_ON_ISENSE;
		  		  //CAMBIO_ON_1A10;
		  		  sum_ki = 0;
		  	  }
		  	  else if (med_1a10 > med_isense)	//activo el termino integral
		  	  {
	  			  if (!last_ki_neg)
	  			  {
		  			  sum_ki++;
		  			  if (sum_ki > 128)
		  			  {
		  				//CAMBIO_ON_ISENSE;
			  			pwm++;
		  				sum_ki = 0;
		  			  }
	  			  }
	  			  else
	  			  {
	  				  sum_ki = 0;
	  				  last_ki_neg = 0;
	  			  }
		  	  }
		  	  else if (med_1a10 < med_isense)	//activo el termino integral
		  	  {
	  			  if (last_ki_neg)
	  			  {
		  			  sum_ki++;
		  			  if (sum_ki > 128)
		  			  {
		  				//CAMBIO_ON_ISENSE;
		  				if (pwm)
		  					pwm--;
		  				sum_ki = 0;
		  			  } 			  
	  			  }
	  			  else
	  			  {
	  				  sum_ki = 0; 
	  				  last_ki_neg = 1;
	  			  }
	  		  }
		  	  //sino deben ser iguales o algo asi
		  	  
		  	  //TODO revisar pwm para que PWM_MIN - pwm no llegue a 0 o algo asi como PWM_MAX
#ifndef DRIVER100W_V2
		  	  //855 me da 3.95A
		  	  if (pwm < PWM_ARRANQUE2)
		  		  Set_40K(PWM_ARRANQUE2 - pwm);	//sin pnp en pata 16		  	  		  	  
#else
		  	  if (pwm < PWM_MIN)
		  		  Set_40K(PWM_MIN - pwm);	//sin pnp en pata 16		  	  
#endif
		  	  //if (pwm < (VALOR_TPMMOD_EDGE - 20))
		  	//	  Set_40K((VALOR_TPMMOD_EDGE - 20) - pwm);	//sin pnp en pata 16

		  	  //Set_40K(480 + pwm); //con pnp en pata 16
#else
		  	  //Termino proporcional kp = 1
		  	  //vsense = 17 * med_1a10;
		  	  vsense = 16 * med_1a10;
		  	  vsense = vsense / 10;
		  	  pwm = 1418 - vsense;
		  	  
		  	  //si tengo que aumentar la corriente pwm tiene que ser mas chico
		  	  //Termino integral ki = sum / 4
		  	  /*
	  		  if (med_1a10 >= med_isense)
	  		  {
	  			  ki = med_1a10 - med_isense;
	  			  ki >>= 2;
	  			  sum_ki += ki;
	  			  pwm -= sum_ki;
	  		  }
	  		  else
	  		  {
	  			  ki = med_isense - med_1a10;
	  			  ki >>= 2;
	  			  sum_ki -= ki;
	  			  pwm += sum_ki;
	  		  }
	  		  */
		  	  //si tengo que aumentar la corriente pwm tiene que ser mas chico
		  	  //si hago un cambio en el integral lo hago de a uno
	  		  if (med_1a10 > med_isense)
	  		  {
	  			  if (last_ki_neg)
	  				  sum_ki = 0;
	  			  last_ki_neg = 0;
	  			  sum_ki++;
	  			  pwm -= (sum_ki/4);
	  			  //pwm -= sum_ki;
	  		  }
	  		  else if (med_1a10 < med_isense)
	  		  {
	  			  if (!last_ki_neg)
	  				  sum_ki = 0; 
	  			  last_ki_neg = 1;
	  			  sum_ki++;
	  			  pwm += (sum_ki/4);
	  			  //pwm += sum_ki;
	  		  }
	  		  
	  		
	  		  Set_40K(pwm);
/*	  		  
		  	  //No quiero que se mueva mas de 1 punto (de los 200 posibles) cada 10ms
	  		  if (med_1a10 > med_isense)
	  			  pwm++;
	  		  else
	  			  pwm--;
	  		
	  		  Set_40K(MIN_D - pwm);
*/	  		  
#endif
		  	  //ademas el cambio tarda 80ms o 160ms en propagar, espero ese tiempo antes de medir nuevamente
		  	  //timer_1_10 = 50 * 8;
		  	  //timer_standby = 50 * 8;

		  	  //timer_standby = TIMER_HYST;	//espero 1 seg para activar hysteresis 
		  	  
	  	      
	  		state = TOMO_MUESTRA_1A10;
	  	      //Apago todos los flags
	  	      INDICACION_OFF;
	  	      CAMBIO_OFF_1A10;
	  	      CAMBIO_OFF_ISENSE;
		  	  break;
		  	  
	  	  case ARRANQUE_FAIL_01:
	  		  
	  		  if (!timer_1_10)
	  		  {
	  			INDICACION_ON;
	  			state = ARRANQUE_RESET;
	  		  }
	  		  break;

	  	  case V1A10_FAIL_01:
	  		  if (Get_1_10() > 25)
	  		  {
	  			  /*
	  			  state = ARRANQUE_RESET;
	  			  timer_standby = 3000;
	  			  */
	  			  
	  			  /*
	  			  state = TOMO_MUESTRA_1A10;;
	  			  Gen_Arranque_Enabled();
	  			  Set_Arranque(2000);
	  			  //cuando salgo de la protección espero un rato antes de entrar de nuevo
	  			  timer_standby = 3000;
	  			  //ademas acelero la busqueda de corriente
	  			  arrancando = 500;
	  			  */
		  			INDICACION_ON;
		  			state = ARRANQUE_RESET;

					
	  		  }
	  		  break;

	  	  default:
	  		  state = ARRANQUE_RESET;
	  		  break;
	  }
	  //Resolver cuestiones que no tengan que ver con muestras ACA
#ifdef STEP_RESPONSE	  
	  if (!timer_standby)
	  {
		  timer_standby = 400;
		  if (step_low)
			  step_low = 0;
		  else
			  step_low = 1;
	  }
#endif
	  
	  //reviso el estado de apagado
	  //si veo la corriente cerca de 0 durante 800ms espero 3 segundos y reseteo
	  
	  /*
	  //APAGADO POR CORRIENTE
	  if ((!timer_standby) && (state != ARRANQUE_FAIL_01)
			  && (state != ARRANQUE_RESET)
			  && (state != ARRANQUE_01)
			  && (state != ARRANQUE_02)
			  && (state != ARRANQUE_03))
	  {
		  if (med_isense <= 8)
		  {
			  flag_apagado++;
		  }
		  else if (flag_apagado)
			  flag_apagado--;
			  
		  timer_standby = 100;	//int cada 100us reviso cada 10ms
		  
		  if (flag_apagado > 25)
		  //if (flag_apagado > 80)	//ayuda al arranque pero es lento en detectar
		  {
			  //si entro aca desconecto todo y espero 3 segundos
			  //INDICACION_ON;
			  //Set_40K(PWM_MIN);
			  flag_apagado = 0;
			  state = ARRANQUE_FAIL_01;
			  timer_1_10 = 30000;	//3 seg
			  Gen_Arranque_Disabled();			  
			  Gen_40K_Disabled();


		  }
	  }	
	    */
	  
	  //APAGADO POR TENSION
	  if ((!timer_standby) && (state != V1A10_FAIL_01)
		&& (state != ARRANQUE_RESET)
		&& (state != ARRANQUE_01)
		&& (state != ARRANQUE_02)
		&& (state != ARRANQUE_03))
		  {
		   	  if (med_1a10_error < 20) 
			  {				  
				  timer_1_10 = 30000;	//3 seg
				  Gen_Arranque_Disabled();			  
				  Gen_40K_Disabled();				
				  state = V1A10_FAIL_01;
			  }
	  	  }	  
  }//cierra while 1    
}//cierra main
#endif //((DRIVER100W) || (DRIVER100W_V2))

unsigned short Module(short a)
{
	if (a < 0)
		return -a;
	else
		return a;
}

unsigned short Get_V_Out (void)
{
	unsigned short medida;

	//veo la tension en PTA3
	medida = New_Sample (AN_CH3);	//0 -> 0V a 255 -> 3.3V; 0.0129V/sample
	
	return medida;
}


unsigned short Get_1_10 (void)
{
	unsigned short medida;
	unsigned short total_ma, result_ma;
	unsigned char j;
	
	//veo la tension en PTB0
	medida = New_Sample (AN_CH1);	//0 -> 0V a 255 -> 3.3V; 0.0129V/sample
	
	
#ifdef FILTROV_CON_TOPE	
	//hago algunas cuentas
	if (last_medida_1a10 < medida)
	{	//medida es mayor a last_medida
		//no dejo diferencias mayores a 5 puntos o a TOPE_MEDIDA_V
		if ((last_medida_1a10 + TOPE_MEDIDA_V) < medida)
			medida = last_medida_1a10 + TOPE_MEDIDA_V;
	}
	else
	{	//medida es menor a last_medida
		//solo si (last_medida - DIF_MEDIDA) > 0
		if ((last_medida_1a10 - TOPE_MEDIDA_V) > 0)
			if (medida < (last_medida_1a10 - TOPE_MEDIDA_V))
				medida = last_medida_1a10 - TOPE_MEDIDA_V;
	}
	last_medida_1a10 = (short) medida;
#endif
	
	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7 
	total_ma = 0;
	vma_1a10[LARGO_FILTRO] = medida; 
    for (j = 0; j < (LARGO_FILTRO); j++)
    {
    	total_ma += vma_1a10[j + 1];
    	vma_1a10[j] = vma_1a10[j + 1];
    }
    
    //REDONDEO
    //cuando tengo una diferencia de 1 voy a buscar redondear hacia arriba o abajo
    result_ma = total_ma >> (DIVISOR - 1);	//me quedo con el resto
    if (result_ma & 0x01)
    {
    	result_ma = total_ma >> DIVISOR;
    	result_ma += 1;
    }
    else
    	result_ma = total_ma >> DIVISOR;
          
    return result_ma;
}

unsigned short Get_Isense (void)
{
	unsigned short medida;
	
#ifdef WITH_FILTER
	unsigned short result_ma;
	unsigned char j;
	unsigned short total_ma;
#endif
	medida = New_Sample (AN_CH2);	//0 -> 0V a 255 -> 3.3V; 0.0129V/sample
       
	//hago algunas cuentas
#ifdef FILTROI_CON_TOPE
	if (last_medida_Isense < medida)
	{	//medida es mayor a last_medida
		//no dejo diferencias mayores a 5 puntos
		if ((last_medida_Isense + DIF_MEDIDA_I) < medida)
			medida = last_medida_Isense + DIF_MEDIDA_I;	//TODO meter break
	}
	else
	{	//medida es menor a last_medida
		//solo si (last_medida - DIF_MEDIDA) > 0
		if ((last_medida_Isense - DIF_MEDIDA_I) > 0)
			if (medida < (last_medida_Isense - DIF_MEDIDA_I))
				medida = last_medida_Isense - DIF_MEDIDA_I;	//TODO meter break
	}
	last_medida_Isense = (short) medida;
#endif

#ifdef WITH_FILTER
	//Kernel mejorado
	total_ma = 0;
    for (j = 0; j < (LARGO_FILTRO - 1); j++)
    {
    	total_ma += vma_isense[j];
    	vma_isense[j] = vma_isense[j + 1];
    }
    total_ma += vma_isense[LARGO_FILTRO - 1];
    
#ifdef REDONDEO
    //cuando tengo una diferencia de 1 voy a buscar redondear hacia arriba o abajo
    result_ma = total_ma >> (DIVISOR - 1);	//me quedo con el resto
    if (result_ma & 0x01)
    {
    	result_ma = total_ma >> DIVISOR;
    	result_ma += 1;
    }
    else
    	result_ma = total_ma >> DIVISOR;
#else
    result_ma = total_ma >> DIVISOR;
    
#endif
    vma_isense[LARGO_FILTRO - 1] = medida;
    return result_ma;       
#else	//without_filter
    return medida;
#endif
    
    
}

void Flash_Check (void)
{
  //escribir para la freq en FCDIV
  
  //primero revisar que FACCERR en FSTAT esta en 0 sino no se puede grabar
  
  //la frecuencia debe estar entre 150 y 200KHz, se determina con FBUS y FCDIV para que tfclk sea 5us
  
  //escribir el comando en FCMD
  
  //escribir 1 a FCBEF en FSTAT
  
  //FCCF nos va a indicar el fin del comando
  
  //hay que poner a 0 el FCBEF
  
}

void Flash_Clock_Init(void)
{ 
  //Para poder grabar la flash
  // initialize Flash clock for 150-200kHz range
  //FCDIV = FCDIVVAL;
  //FCDIV = 0x49;
  FCDIV = 0x27; //si esta grabado despues de reset debe mostrar 0xA7
                //en 0x1820
}

void Grabar_Modo (char modo)
{
  //reviso si ya grabe la freq en FCDIV
  if (!(FCDIV & 0x80))
    Flash_Clock_Init();
  
  //Prueba de Flash
  //16 paginas 512 bytes cada una empieza en la 0xE000 termina en la 0xFFFF

  data_lenght = 2;
  modo_func[1] = modo;
      
  DisableInterrupts;
  FlashErase(&a);   //la direccion base del bloque a borrar
      
  FlashProgBurst(&a, &modo_func[0], data_lenght); //la direccion en la cual guardar
                                                  //los datos en RAM a guardar
                                                  //el largo de los datos a guardar
  EnableInterrupts;
      
}

  



