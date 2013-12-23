;-----------------------------------------------------------------------------------------------------------
; cligno_run.asm    Gestion des DELs RUN et TRAME
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
	.module Clignotant_RUN
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _clignotant_run   Complement de la diode de fonctionnement
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_clignotant_run:
	push PSW
	mov PSW,#BANK2
	djnz R7,_clignotant_run_fin
	mov R7,#0x0F
	djnz R6,_clignotant_run_fin
	mov R6,#0x10
	cpl P3.7
_clignotant_run_fin:
	pop PSW
	ret
;-----------------------------------------------------------------------------------------------------------
; _cdg_logiciel  Stoppe les sorties si le maitre ne repond pas assez souvent
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_cdg_logiciel:
	push PSW
	mov PSW,#BANK2
	djnz R5,_cdg_logiciel_fin
	mov R5,#0xFF
	djnz R4,_cdg_logiciel_fin
        lcall _raz_sortie_tor
_cdg_logiciel_fin:
	pop PSW
	ret

_init_cdg_logiciel:
	push PSW
	mov PSW,#BANK2
	mov R5,#0xFF
	mov R4,#0xFF
	pop PSW
	ret

;-----------------------------------------------------------------------------------------------------------
; _del_trame_xx  Allumage ou Extinction de la diode de presence trame
;-----------------------------------------------------------------------------------------------------------
_del_trame_on:
	setb P3.6
	ret
_del_trame_off:
	clr P3.6
	ret

;-----------------------------------------------------------------------------------------------------------
