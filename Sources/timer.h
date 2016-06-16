#ifndef _TIMER_H_
#define _TIMER_H_

#include "hard.h"

//for busclk=8Mhz (clk rate=16Mhz)
//El RTI trabaja directo con el XTAL, entonces la frecuencia de 
//referencia es de 16MHz

#define TIMER_IDLE  2
#define Sleep_ms(x) Wait_ms(x)

//para 80000 Hz GENERA MUCHO JITTER
//#define VALOR_TPMMOD 100  //tick = 125ns OJO con 100 me termina dando entre 31.3 y 30.9KHz
//#define VALOR_TPMMOD 77  //tickeq = 161.3ns => 40.7KHz con un poco de jitter en 6.8KHz (parece cuando salta RTI)
//#define VALOR_TIMER_400 100  //40KHz dividido 100

//voy a usar edge aligned GENERA MUCHO JITTER
//#define VALOR_TPMMOD 100  //tick = 125ns OJO con 100 me termina dando entre 31.3 y 30.9KHz
//#define VALOR_TPMMOD_EDGE 200  //tickeq = 161.3ns => 40.7KHz con un poco de jitter en 6.8KHz (parece cuando salta RTI)
//#define VALOR_TPMMOD_EDGE 400  //tickeq = 125ns => 20KHz
//#define VALOR_TPMMOD_EDGE 540  //tickeq = 125ns => 14.8KHz
//#define VALOR_TPMC0V_EDGE 120  //equivale al 30%
//#define VALOR_TPMC0V_EDGE 77  //40KHz dividido 100
//#define VALOR_TPMC0V_EDGE 63  //40KHz dividido 100 para 0.33 Duty
#define FREQ4K
//#define FREQ3K
//#define FREQ2K

#ifdef FREQ4K
//quiero 512 puntos entre los periodos de trabajo d
#define VALOR_TPMMOD_EDGE 1896  //tickeq = 125ns => 4220Hz
#define VALOR_TPMC0V_EDGE 493  //equivale al 6.25% y minimo d=0.26
#define MIN_D (1005 + 1)
#define MAX_D (493 - 1)
#endif

#ifdef FREQ3K
//quiero 512 puntos entre los periodos de trabajo d
#define VALOR_TPMMOD_EDGE 2666  //tickeq = 125ns => 2000Hz
#define VALOR_TPMC0V_EDGE 693  //equivale al 6.25% y minimo d=0.26
#define MIN_D (2000 + 1)
#define MAX_D (693 - 1)
#endif

#ifdef FREQ2K
//quiero 512 puntos entre los periodos de trabajo d
#define VALOR_TPMMOD_EDGE 4000  //tickeq = 125ns => 2000Hz
#define VALOR_TPMC0V_EDGE 1040  //equivale al 6.25% y minimo d=0.26
#define MIN_D (2000 + 1)
#define MAX_D (1040 - 1)
#endif

#define VALOR_TPMC1V_EDGE 493  //equivale al 6.25% y minimo d=0.26

#define TPM_40K_ON 0x28
#define TPM_40K_OFF 0x00

#ifdef FREQ2K
#ifdef OPTO_KINGBRIGHT
#define PWM_MIN 2785	//sin el pnp en el medio
#define PWM_MITAD 1392
#define PWM_ARRANQUE2 3692	//16.5V salida no prende LED
#define PWM_ZERO 0
#endif
#endif

#ifdef FREQ3K
#ifdef OPTO_KINGBRIGHT
#define PWM_MIN 1856	//sin el pnp en el medio
#define PWM_MITAD 928
#define PWM_ARRANQUE2 2460	//16.5V salida no prende LED
#define PWM_ZERO 0
#endif
#endif

#ifdef FREQ4K
#ifdef OPTO_KINGBRIGHT
#define PWM_MIN 626	//programa mas corriente
//#define PWM_MIN 688	//programa típico
#define PWM_MITAD 660
//#define PWM_ARRANQUE2 1750	//16.5V salida no prende LED
#define PWM_ARRANQUE2 (VALOR_TPMMOD_EDGE - PWM_MIN)
#define PWM_MAX_IN_FUNC (VALOR_TPMMOD_EDGE - 544)	//le dejo 1352; modificacion 24-05-16 lo dejo generar mas ciclo pwm
													//1896 - 544 = 1352
#define PWM_ZERO 0
#endif
#endif

#ifdef OPTO_KINGBRIGHTV2
#define PWM_MIN 922	//sin el pnp en el medio
#define PWM_MITAD 461
#define PWM_ARRANQUE2 1352	//16.5V salida no prende LED
#endif

#ifdef OPTO_KINGBRIGHTV22
#define PWM_MIN 1300	//sin el pnp en el medio
#define PWM_MITAD 450
#define PWM_ARRANQUE2 1300	//16.5V salida no prende LED
#endif

#ifdef OPTO_KINGBRIGHTV21
#define PWM_MIN 900	//sin el pnp en el medio
#define PWM_MITAD 450
#define PWM_ARRANQUE2 1352	//16.5V salida no prende LED
#endif

#ifdef OPTO_KINGBRIGHTV3
#define PWM_MIN 1005	//sin el pnp en el medio
#define PWM_MITAD 503
#define PWM_ARRANQUE2 1352	//16.5V salida no prende LED
#endif

#ifdef OPTO_KINGBRIGHTV4
#define PWM_MIN 1105	//sin el pnp en el medio
#define PWM_MITAD 553
#define PWM_ARRANQUE2 1352	//16.5V salida no prende LED
#endif

#ifdef OPTO_SHARP
#define PWM_MIN 850	//sin el pnp en el medio
#define PWM_MITAD 425
#define PWM_ARRANQUE2 1280	//16.5V salida no prende LED
#endif

//#define PWM_MIN_ARRANQUE_FAIL 1360	//sin el pnp en el medio
//#define PWM_MIN_ARRANQUE_FAIL 1350	//sin el pnp en el medio (cuando no tenia masa ni 100nF en opamp

//#define PWM_MIN_ARRANQUE_FAIL 1365	//con masa y opamp en frio ok en caliente titila
//#define PWM_MIN_ARRANQUE_FAIL 1355	//con masa y opamp en frio en caliente titila
#define PWM_MIN_ARRANQUE_FAIL 1345	//con masa y opamp en frio en caliente titila


//#define VALOR_MTIM 19	//tick = 32us OJO con 39 me termina dando entre 309Hz
#define VALOR_MTIM 5	//tick = 32us OJO con 39 me termina dando entre 309Hz
//#define VALOR_MTIM 30	//tickeq = 41.487us => 397Hz
//#define VALOR_MTIM 15	//tickeq = 20.74us => 800Hz             

//	FUNCIONES DEL MODULO                     
void RtiInit (void);
interrupt void VRti ();
void Wait_ms (unsigned short);
//interrupt void VTpm1();
//void Tpm1Init (void);
void TpmCh0Init(void);
void TpmCh1Init(void);
void Gen_40K_Enabled(void);
void Gen_40K_Disabled(void);
void Set_40K(unsigned short);
void MtimInit(void);
interrupt void VTpmOvf();
interrupt void isrVmtim();
void Set_R_Ton (unsigned char);
unsigned char Switch_On (void);
void Gen_Arranque_Enabled(void);
void Gen_Arranque_Disabled(void);
void Set_Arranque(unsigned short);


#endif
