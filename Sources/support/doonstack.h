/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      doonstack.h
*
* @author    B01119
* 
* @version   1.0.3.0
* 
* @date      May-27-2009
* 
* @brief     C header file for DoOnStack.asm
*
*******************************************************************************/

#ifndef _DOONSTACK_H
#define _DOONSTACK_H

#ifdef __cplusplus
	extern "C" { /* our assembly functions have C calling convention */
#endif

// DoOnStack API
void FlashErase(const unsigned char * flash_destination);
void FlashProg(const unsigned char * flash_destination, unsigned char data);
unsigned char FlashProgBurst(const unsigned char * flash_destination, unsigned char * ram_source, unsigned char length);

// this definitions has been on this place only for backward compatibility
#define CopyRam2Flash (void)FlashProgBurst
#define CopyRam2FlashBurst CopyRam2Flash

#ifdef __cplusplus
	}
#endif
  
#endif /* _DOONSTACK_H */
