;-----------------------------------------------------------------------------------------------------------
; fct_ident.asm    Fonction d'identification
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
        .module FctIDENT
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _fct_ident: envoie d'une trame d'identification
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_fct_ident:
	mov R0,#TX_BUF
        mov @R0, #ID_MAITRE         ; envoie au maitre
        inc R0
	mov @R0, #ID_ESCLAVE        ; source
        inc R0
	mov @R0, #FCT_IDENT         ; fonction ident
        inc R0
	mov @R0, #0x0D              ; taille

        inc R0
        push ACC
        mov A,ID_IDENT
        inc A
        mov ID_IDENT,A
        mov @R0,A
        pop ACC

        inc R0
	mov @R0, #VERSION_MAJOR     ; majeur
        inc R0
	mov @R0, #VERSION_MINOR     ; mineur
        inc R0
	mov @R0, #NBR_ENTRE_ANA     ; Nombre d'entrée ANA gérées
        inc R0
	mov @R0, #NBR_ENTRE_TOR     ; Nombre d'entrée TOR gérées
        inc R0
	mov @R0, #NBR_ENTRE_CHOC    ; dont entrées choc
        inc R0
	mov @R0, #NBR_SORTIE_TOR    ; Nombre de sorties TOR gérées

        inc R0
        mov A,TH0
	mov @R0, A                  ; Watchdog Maitre Timer 0
        inc R0
        mov A,TL0
	mov @R0, A                  ; Watchdog Maitre Timer 0

        inc R0
	mov @R0, T3                ; Watchdog Materiel Timer 3

        inc R0
        mov A,NBR_INT_CAN
	mov @R0, A                 ; Nombre d'interruption CAN

        inc  R0
        push PSW
        mov  PSW,#BANK2
        mov  A,R2
        pop  PSW
        mov  @R0,A

        inc  R0
        push PSW
        mov  PSW,#BANK2
        mov  A,R3
        pop  PSW
        mov  @R0,A

	lcall _envoyer_rs232        ; on envoie la trame
	ret
;-----------------------------------------------------------------------------------------------------------
