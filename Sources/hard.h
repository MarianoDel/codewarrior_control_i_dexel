#ifndef _HARD_H_
#define _HARD_H_

#include "derivative.h"

//MODOS DEL PROGRAMA
//#define POR_INTERRUPCION
//#define OPEN_LOOP
//#define CONSTANT
#define DRIVER40W 			//cambios 16-06-2016 led mayor tension
//#define DRIVER60W 
//#define DRIVER100W
//#define DRIVER100W_V2		//se usa en conjunto con DRIVER100W
//#define DRIVER100W_V14		//NO USAR EN conjunto con DRIVER100W y DRIVER100W_V2 
#define GEN_4K

//DISTINTOS OPTOS
#define OPTO_KINGBRIGHT
//#define OPTO_KINGBRIGHTV2		//extraniamente solo en una placa
//#define OPTO_KINGBRIGHTV21		//extraniamente solo en una placa
//#define OPTO_KINGBRIGHTV22	//es el mas grabado
//#define OPTO_KINGBRIGHTV3
//#define OPTO_KINGBRIGHTV4
//#define OPTO_SHARP				//todas las kingbright de 60W pasadas a 100W tuvimos que ponerle este (SHARP)

//Conexiones de las patas del micro

//canal P con npn en el medio
#define MOSFET (PTBD_PTBD5 == 1)
#define MOSFET_OFF PTBD_PTBD5 = 0
#define MOSFET_ON PTBD_PTBD5 = 1

//canal TR ENA SALIDA con npn
#define OUT (PTBD_PTBD5 == 0)
#define OUT_OFF PTBD_PTBD5 = 1
#define OUT_ON PTBD_PTBD5 = 0

//transistor para CTROL
#define TR_CTRL (PTAD_PTAD0 == 1)
#define TR_CTRL_OFF PTAD_PTAD0 = 0
#define TR_CTRL_ON PTAD_PTAD0 = 1


//sin PNP en el medio
//#define MOSFET (PTAD_PTAD0 == 0)
//#define MOSFET_OFF PTAD_PTAD0 = 1
//#define MOSFET_ON PTAD_PTAD0 = 0

#define INDICACION (PTBD_PTBD0 == 1)
#define INDICACION_ON PTBD_PTBD0 = 1
#define INDICACION_OFF PTBD_PTBD0 = 0

#define CAMBIO_1A10 (PTBD_PTBD1 == 1)
#define CAMBIO_ON_1A10 PTBD_PTBD1 = 1
#define CAMBIO_OFF_1A10 PTBD_PTBD1 = 0

#define CAMBIO_ISENSE (PTBD_PTBD2 == 1)
#define CAMBIO_ON_ISENSE PTBD_PTBD2 = 1
#define CAMBIO_OFF_ISENSE PTBD_PTBD2 = 0

#define ARRANQUE_RESET 			0
#define ARRANQUE_01 			1
#define ARRANQUE_02 			2
#define ARRANQUE_03 			3
#define TOMO_MUESTRA_1A10 		4
#define TOMO_MUESTRA_ISENSE 	5
#define VEO_MODIFICACIONES 		6
#define HAGO_CAMBIOS 			7
#define ARRANQUE_FAIL_01		8
#define V1A10_FAIL_01			9
#define ARRANQUE_04				10

#ifdef DRIVER100W_V14
#define MAX_1A10	240
//#define MAX_1A10	254
#endif

#ifdef DRIVER100W
#define MAX_1A10	240
//#define MAX_1A10	254
#endif

#ifdef DRIVER60W
//#define MAX_1A10	185
#define MAX_1A10	240	//MAX_1A10 lo mantengo pero hago una cuenta para darle margen
//#define MAX_1A10	255	//lo dejo ir hasta 250 porque me conviene la cuenta
#endif

#ifdef DRIVER40W
#define MAX_1A10	240
//#define MAX_1A10	254
#endif

#endif

