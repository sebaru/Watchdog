;-----------------------------------------------------------------------------------------------------------
; fct_entre_ana.asm    Fonction d'envoi des entrees ANA
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
        .module FctEntreANA
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _fct_entre_ana: Envoi de l'etat des entrées ANA
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_fct_entre_ana:
	mov R0,#TX_BUF
	mov R1,#ENTRE_ANA_BUF
	mov @R0, #ID_MAITRE         ; envoie au maitre
	inc R0
	mov @R0, #ID_ESCLAVE        ; source
	inc R0
	mov @R0, #FCT_ENTRE_ANA     ; fonction
	inc R0

	mov A,#NBR_ENTRE_ANA       ; R3 = cpt d'entree TOR
	mov R3,A
	jnz _fct_entre_ana_suite    ;

        mov A,#0x00
        mov @R0,A                   ; zero entre ANA

	lcall _envoyer_rs232        ; on envoie la trame
	ret
_fct_entre_ana_suite:
	mov R0,#TX_BUF
	mov R1,#ENTRE_ANA_BUF
	mov @R0, #ID_MAITRE         ; envoie au maitre
	inc R0
	mov @R0, #ID_ESCLAVE        ; source
	inc R0
	mov @R0, #FCT_ENTRE_ANA     ; fonction
	inc R0

	mov A,R3                    ; A = nbr entre ana
	dec A                       ; A = nbr_entre_ana - 1
	clr C
	rrc A                       ; A = (nbr_entre_ana - 1)/2
	clr C
	rrc A                       ; A = (nbr_entre_ana - 1)/4
	inc A                       ; A = (nbr_entre_ana - 1)/4 + 1
	add A,R3                    ; A vaut nbr_entre_ana + nbr_octet_suppl_10_bits
	mov R3,A                    ; R3 = Nombre total d'octet donnees
        mov @R0,A                   ; ajout dans la trame
        inc R0 
	
_fct_entre_ana_again:
	mov A,@R1
	mov @R0,A                   ; taille = nbr entre ana + 1 (10bits can)
	inc R0                      ; R0 pointe sur le debut donnees trame.
	inc R1
	djnz R3,_fct_entre_ana_again

	lcall _envoyer_rs232        ; on envoie la trame
        ret
;-----------------------------------------------------------------------------------------------------------

