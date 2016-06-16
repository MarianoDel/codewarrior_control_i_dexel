#include "timer.h"
#include "derivative.h"
#include "hard.h"
#include "hidef.h"
//#include "analog.h"


//---- VARIABLES EXTERNAS ----//
extern volatile unsigned char timer_overflow;
extern volatile unsigned int timer_standby;
extern volatile unsigned int timer_1_10;
extern volatile unsigned int timer_grabado;
#ifdef POR_INTERRUPCION
extern short pwm;
#endif

//---- VARIABLES GLOBALES ----//
volatile unsigned int rti;
volatile unsigned char r_ton = 0;
volatile unsigned char r_timer = 0;
volatile unsigned char t_switch = 0;
#ifdef POR_INTERRUPCION
volatile unsigned short last_t_pwm = 0;
volatile unsigned char t_pwm = 0;
#endif

interrupt void VRti() {
    SRTISC = SRTISC | 0x40;  //escribo 1 en RTIACK  
    
    if (rti)
      rti--;
    
    if (timer_grabado)
      timer_grabado--;
    
}

void RtiInit (void) { 
  SRTISC = 0x11;  //clk interno 1KHz, ena interrupt, 8msegs
  rti = 0;
}

void Wait_ms (unsigned short tiempo)
{
  //solo multiplos de 8msegs
  tiempo >>= 3;
  rti = tiempo;
  while (rti)
    ;
}

// TPM necesito una interrupcion cada 4KHz y ademas el PWM para el led verde
//
// TPMSC  CPWMS = 0
//        CLKSB:A = 0 1
//        PS2:1:0 = 0 0 0
//
// TPMMODH TPMMODL valor para 4 khz
//
// TPMC0SC  CH0IE = 1
//          MS0B:A = 1 0
//          ELS0B:A = 1 0
//
// TPMC0VH TPMC0VL duty cycle
//
// utilizo la misma interrupcion del channel 0 en PWM y no la de overflow

// EDGE ALIGNED SIN INT
void TpmCh0Init(void) {
#ifdef GEN_4K	
  TPMMOD = VALOR_TPMMOD_EDGE;
#else
	TPMMOD = 952;
#endif
  TPMC0V = VALOR_TPMC0V_EDGE;
  TPMC0SC = TPM_40K_OFF;  //sin int edge aligned ch0
  TPMSC = 0x08;   //sin int ovf y bus clk sin divisor
//  TPMSC = 0x0A;   //sin int ovf y bus clk divido 4
}

// EDGE ALIGNED SIN INT
void TpmCh1Init(void) {
  //TPMMOD = VALOR_TPMMOD_EDGE;	//se utiliza el del TpmCh0
  TPMC1V = VALOR_TPMC1V_EDGE;
  TPMC1SC = TPM_40K_OFF;  //sin int edge aligned ch0 uso el mismo que TpmCh0
  //se utiliza el TPMSC del TpmCh0
  //TPMSC = 0x08;   //sin int ovf y bus clk sin divisor
//  TPMSC = 0x0A;   //sin int ovf y bus clk divido 4
}
void Gen_40K_Enabled(void)
{
	//pata del port al CH0
	TPMC0SC = TPM_40K_ON;  //sin int edge aligned ch0
	//ojo arranco desde PWM_MIN
	TPMC0V = PWM_MIN;
}

void Gen_40K_Disabled(void)
{
	TR_CTRL_OFF;
	//revierto pata CH0 a port
	TPMC0SC  = TPM_40K_OFF;
	TR_CTRL_OFF;
}

void Set_40K(unsigned short nuevo)
{
	//los valores se setean como 1 - d
//	if (nuevo < MAX_D)
//		TPMC0V = MAX_D;
//	else if (nuevo > MIN_D)
//		TPMC0V = MIN_D;
//	else
		TPMC0V = nuevo;
}

void Gen_Arranque_Enabled(void)
{
	//pata del port al CH0
	TPMC1SC = TPM_40K_ON;  //sin int edge aligned ch0
	//ojo arranco desde 0
	TPMC1V = 0;
}

void Gen_Arranque_Disabled(void)
{
#ifdef DRIVER100W_V14
	OUT_OFF;	//por las dudas
	//revierto pata CH0 a port
	TPMC1SC  = TPM_40K_OFF;
	//apago port
	OUT_OFF;	
#else
	MOSFET_OFF;	//por las dudas
	//revierto pata CH0 a port
	TPMC1SC  = TPM_40K_OFF;
	//apago port
	MOSFET_OFF;
#endif
}

void Set_Arranque(unsigned short nuevo)
{
	//los valores se setean como 1 - d
//	if (nuevo < MAX_D)
//		TPMC0V = MAX_D;
//	else if (nuevo > MIN_D)
//		TPMC0V = MIN_D;
//	else
		TPMC1V = nuevo;
}


interrupt void VTpmOvf() {

	if (TPMSC_TOF)
    TPMSC_TOF = 0;   
}

// FUNCIONES DEL MTIM
void MtimInit(void)
{
	//MTIMCLK = 0x08;	//busclk / 256
	MTIMCLK = 0x03;	//busclk / 8
	//MTIMMOD = VALOR_MTIM;	//cada tick 32us int cada 160u
	MTIMMOD = 100;	//cada tick 1us int cada 100u
	MTIMSC = 0x40; //int
}

interrupt void isrVmtim() {
	if (MTIMSC_TOF)
		MTIMSC_TOF = 0;
	
	if (timer_standby)
		timer_standby--;

	if (timer_1_10)
		timer_1_10--;

#ifdef POR_INTERRUPCION
	//pwm es el error en la medicion deberia ser positivo o negativo
	//Seteo del pwm por interrupcion	cada 500us
	if (!t_pwm)
	{
		t_pwm = 5;
		/*
		if (last_t_pwm < pwm)
			last_t_pwm++;
		else if (last_t_pwm > pwm)
			last_t_pwm--;
		else
			last_t_pwm = pwm;
		*/
		/*
		//TODO: poner limites al movimiento de last_t_pwm
		if (pwm != 0)
		{
			if (pwm > 0)
				last_t_pwm++;
			else
				last_t_pwm--;
			//last_t_pwm debe variar entre un minimo y un maximo
			if (last_t_pwm > 0)
			{
				if (last_t_pwm > LAST_MAX)
				{
					Set_40K(PWM_MIN - LAST_MAX);					
				}
				else
					Set_40K(PWM_MIN - last_t_pwm);
			}				
			else
				Set_40K(PWM_MIN);
		}
		//sino debe ser igual a 0 y no hago nada
		 */
		//TODO: poner limites al movimiento de last_t_pwm
		if (pwm != last_t_pwm)
		{
			if (pwm > last_t_pwm)
				last_t_pwm++;
			else
				last_t_pwm--;
			//last_t_pwm debe variar entre un minimo y un maximo
			Set_40K(PWM_MIN - last_t_pwm);
		}
	}
	else
		t_pwm--;
		
	
//	toggle_400 = 1;
#endif

}

/*
interrupt void VTpmOvf() {

  if (TPMSC_TOF)
    TPMSC_TOF = 0;
  
    if (timer_standby)
      timer_standby--;
    
    if (timer_1_10)
      timer_1_10--;
    
   timer_overflow = 1;
   
   if (r_timer < R_RELOAD)    //rutina R
   {
      if (r_timer >= r_ton)
        MOSFET_R_OFF;
      
      r_timer++;
   }
   else
   {
     if (r_ton)   
        MOSFET_R_ON;
      r_timer = 0;
   }
   
   if (SWITCH)
   {
    if (t_switch < T_SWITCH_MAX)
      t_switch++;
   }
   else
   {
    if (t_switch)
      t_switch--;
   }

}

void Set_R_Ton (unsigned char valor) {
  DisableInterrupts;
  r_ton = valor;
  EnableInterrupts;
}

unsigned char Switch_On (void) {
  unsigned char a;
  DisableInterrupts;
  if (t_switch > T_SWITCH)
    a = 1;
  else
    a = 0;
  EnableInterrupts;
  return a;
}
*/

/*
// TPM Para funcionar como Overflow
//fout = 15,552MHz -> fbus = 7,776MHz
void Tpm1Init (void) {                //cada tick tiene 128,6 ns
  TPM1SC = 0x00;  //disable tpm
  TPM1MOD = 8100;
  TPM1SC = 0x48;  //ena interrupt, busclk, div x 1
  
  rti = 0;
}

interrupt void VTpm1() {
  if (TPM1SC_TOF)
    TPM1SC_TOF = 0;
    
    if (rti)
      rti--;
}

*/
