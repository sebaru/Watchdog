;-----------------------------------------------------------------------------------------------------------
; can.asm    Gestion des conversion analogiques
; SÈbastien Lefevre                                                             sam 13 nov 2004 19:05:43 CET 
;-----------------------------------------------------------------------------------------------------------
        .module Can
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _start_of_conversion IRQ2 Timer 
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_start_of_conversion:
        push PSW                ; ranger le registre
        push ACC
	mov A,ADCON                 ; on recupere le registre de conf
	anl ADCON,#0xF8             ; selection de l'ea 0
	anl A, #0x07                ; respect des limites.
	inc A                       ; choix de l'ea suivante
	cjne A,#NBR_ENTRE_ANA,_start_of_conversion_normal
	mov A,#0x00
_start_of_conversion_normal:
	add A,#0x08                 ; ADCS = 1
        orl ADCON,A
        pop  ACC                    ;
        pop  PSW                    ; rÈactive la banque registres 0  
	ret
;-----------------------------------------------------------------------------------------------------------
; _clignotant_run   Complement de la diode de fonctionnement
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_lance_start_of_conversion:
	push PSW
	mov PSW,#BANK2
	djnz R3,_lance_start_of_conversion_fin
	mov R3,#0xFF
	;djnz R2,_lance_start_of_conversion_fin  pitete a virer ??
	;mov R2,#0x10
        mov CAN_PRET,#0x00
        lcall _start_of_conversion
_lance_start_of_conversion_fin:
	pop PSW
        ret

;-----------------------------------------------------------------------------------------------------------
; _recu_can: IRQ11 C.A.N
;-----------------------------------------------------------------------------------------------------------
_recu_can:
        push PSW                ; ranger le registre
        push ACC
        mov  A,NBR_INT_CAN
        inc  A
        mov  NBR_INT_CAN,A
        mov  PSW,#BANK1
        mov A,ADCON                 ; on recupere le registre de conf
	anl A, #0x07                ; respect des limites. A = ea selectionÈe
	mov R2,A                    ; sauvegarde pour utilisation future

        mov  R0,#ENTRE_ANA_BUF      ; debut buffer
        add A,R0
        mov R1,A                    ; R1 = octet correspondant buffer ea
        mov @R1,ADCH                ; sauvegarde dans buffer 8 bits

	mov A,R2                    ; NumÈro de l'entrÈe selectionnÈe
	clr C
	rrc A
	clr C
	rrc A                       ; Division par 4
	add A,R0                    ; Debut buffer
        add A,#NBR_ENTRE_ANA        ; + Nombre entre ana -> on pointe sur le bon octet 10 bits
        mov R1,A                    ; Buffer 10 bits

	mov A,R2
	anl A,#0x03
	jz _recu_can_0_rotate
        clr C                           ; Carry = 0 pour la soustraction
	subb A,#0x01
        jz _recu_can_1_rotate
        clr C                           ; Carry = 0 pour la soustraction
	subb A,#0x01
        jz _recu_can_2_rotate

	mov A,@R1
	anl A,#0xFC
	mov R3,A
	mov A,ADCON
	anl A,#0xC0
	clr C
        rrc A
        clr C
        rrc A
	clr C
        rrc A
        clr C
        rrc A
	clr C
        rrc A
        clr C
        rrc A
	orl A,R3
	mov @R1,A        
	ljmp _recu_can_rotate_end

_recu_can_2_rotate:
	mov A,@R1
	anl A,#0xF3
	mov R3,A
	mov A,ADCON
	anl A,#0xC0
	clr C
        rrc A
        clr C
        rrc A
	clr C
        rrc A
        clr C
        rrc A
	orl A,R3
	mov @R1,A        
	ljmp _recu_can_rotate_end

_recu_can_1_rotate:
	mov A,@R1
	anl A,#0xCF
	mov R3,A
	mov A,ADCON
	anl A,#0xC0
	clr C
        rrc A
        clr C
        rrc A
	orl A,R3
	mov @R1,A        
	ljmp _recu_can_rotate_end

_recu_can_0_rotate:
	mov A,@R1
	anl A,#0x3F
	mov R3,A
	mov A,ADCON
	anl A,#0xC0
	orl A,R3
	mov @R1,A        
_recu_can_rotate_end:
	mov CAN_PRET,#0x01
        pop  ACC                    ;
        pop  PSW                    ; r√©active la banque registres 0  
        ret
;-----------------------------------------------------------------------------------------------------------
