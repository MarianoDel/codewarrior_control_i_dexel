;******************************************************************************
;
; Freescale Semiconductor Inc.
; (c) Copyright 2004-2006 Freescale Semiconductor, Inc.
; (c) Copyright 2001-2004 Motorola, Inc.
; ALL RIGHTS RESERVED.
;
;**************************************************************************//*!
;
;   $File Name: doonstack.inc$
;
;   $Last Modified By: B01119$
;
;   $Version: 1.0.1.0$
;
;   $Date: Apr-4-2007$ 
;
;   @brief     DoOnStack.inc header file (target dependent)
;
;******************************************************************************
;
; 
;
;*****************************************************************************/


;****  Flash Module (FLASH)  ****************************************************************
;*
FCDIV:      equ   $1820       ;Flash clock divider register
; bit position masks
mDIVLD:     equ   %10000000   ;clock divider loaded
mPRDIV8:    equ   %01000000   ;enable prescale by 8
mDIV5:      equ   %00100000   ;flash clock divider bits (high)
mDIV4:      equ   %00010000   ; "
mDIV3:      equ   %00001000   ; "
mDIV2:      equ   %00000100   ; "
mDIV1:      equ   %00000010   ; "
mDIV0:      equ   %00000001   ;flash clock divider bits (low)

FOPT:       equ   $1821       ;Flash options register
; bit position masks
mKEYEN:     equ   %10000000   ;enable backdoor key to security
mFNORED     equ   %01000000   ;Vector redirection enable
mSEC01:     equ   %00000010   ;security state code (high)
mSEC00:     equ   %00000001   ;security state code (low)

FCNFG:      equ   $1823       ;Flash configuration register
; bit position masks
mKEYACC:    equ   %00100000   ;enable security key writing

FPROT:      equ   $1824       ;Flash protection register
; bit position masks
mFPOPEN:    equ   %10000000   ;open unprotected flash for program/erase
mFPDIS:     equ   %01000000   ;flash protection disable
mFPS2:      equ   %00100000   ;flash protect size select (high)
mFPS1:      equ   %00010000   ;flash protect size select
mFPS0:      equ   %00001000   ;flash protect size select (low)

FSTAT:      equ   $1825       ;Flash status register
; bit position masks
mFCBEF:     equ   %10000000   ;flash command buffer empty flag
mFCCF:      equ   %01000000   ;flash command complete flag
mFPVIOL:    equ   %00100000   ;flash protection violation
mFACCERR:   equ   %00010000   ;flash access error
mFBLANK:    equ   %00000100   ;flash verified as all blank (erased =$ff) flag

FCMD:       equ   $1826       ;Flash command register
; bit position masks
mFCMD7:     equ   %10000000   ;Flash command (high)
mFCMD6:     equ   %01000000   ; "
mFCMD5:     equ   %00100000   ; "
mFCMD4:     equ   %00010000   ; "
mFCMD3:     equ   %00001000   ; "
mFCMD2:     equ   %00000100   ; "
mFCMD1:     equ   %00000010   ; "
mFCMD0:     equ   %00000001   ;Flash command (low)
; command codes
mBlank:     equ   $05         ;Blank Check command
mByteProg:  equ   $20         ;Byte Program command
mBurstProg: equ   $25         ;Burst Program command
mPageErase: equ   $40         ;Page Erase command
mMassErase: equ   $41         ;Mass Erase command

;****  Flash non-volatile register images  **************************************************
;*
NVBACKKEY:  equ   $FFB0       ;8-byte backdoor comparison key
; comparison key in $FFB0 through $FFB7

; following 2 registers transfered from flash to working regs at reset

NVPROT:     equ   $FFBD       ;NV flash protection byte
; NVPROT transfers to FPROT on reset

NVICGTRIM:  equ   $FFBE       ;NV ICG Trim Setting
; ICG trim value measured during factory test. User software optionally
; copies to ICGTRM during initialization.

NVOPT:      equ   $FFBF       ;NV flash options byte
; NVFEOPT transfers to FOPT on reset

;****  END  *********************************************************************************
