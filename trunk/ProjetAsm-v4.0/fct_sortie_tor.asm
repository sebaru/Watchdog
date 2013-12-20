;-----------------------------------------------------------------------------------------------------------
; fct_sortie_tor.asm    Gestion des sorties tor
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
        .module FctSortieTOR
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _fct_sortie_tor: Reception des sorties TOR
;-----------------------------------------------------------------------------------------------------------
	.area CSEG (CODE)
_fct_sortie_tor:
	mov A,#NBR_SORTIE_TOR       ; R3 = cpt de sortie TOR
	;mov R3,A
	jnz _fct_sortie_tor_suite   ; Si pas de sortie, on sort direct.
	ret
_fct_sortie_tor_suite:
	mov A,#RX_BUF
	add A,#0x04
	mov R0,A                    ; R0 pointe sur le premier champ de donnees.

	mov R7,#0x80                ; initialisation du poids
	mov R5,#0x00                ; Nombre d'octet donnees dans la trame

	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 8
	.else
        ljmp _fct_sortie_tor_8
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 9
	.else
        ljmp _fct_sortie_tor_9
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 10
	.else
        ljmp _fct_sortie_tor_10
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 11
	.else
        ljmp _fct_sortie_tor_11
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 12
	.else
        ljmp _fct_sortie_tor_12
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 13
 	.else
        ljmp _fct_sortie_tor_13
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 14
	.else
        ljmp _fct_sortie_tor_14
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 15
	.else
        ljmp _fct_sortie_tor_15
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 16
	.else
        ljmp _fct_sortie_tor_16
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 17
	.else
        ljmp _fct_sortie_tor_17
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 18
	.else
        ljmp _fct_sortie_tor_18
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 19
	.else
        ljmp _fct_sortie_tor_19
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 20
	.else
        ljmp _fct_sortie_tor_20
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 21
	.else
        ljmp _fct_sortie_tor_21
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 22
	.else
        ljmp _fct_sortie_tor_22
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 23
	.else
        ljmp _fct_sortie_tor_23
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 24
	.else
        ljmp _fct_sortie_tor_24
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 25
	.else
        ljmp _fct_sortie_tor_25
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 26
	.else
        ljmp _fct_sortie_tor_26
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 27
	.else
        ljmp _fct_sortie_tor_27
        .endif
	ret

_fct_sortie_tor_8:
	acall _fct_sortie_tor_pop_bit
	mov P1.0,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 9
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_9:
	acall _fct_sortie_tor_pop_bit
	mov P1.1,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 10
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_10:
	acall _fct_sortie_tor_pop_bit
	mov P1.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 11
	.else
        ljmp _fct_sortie_tor_fin
        .endif

_fct_sortie_tor_11:
	acall _fct_sortie_tor_pop_bit
	mov P1.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 12
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_12:
	acall _fct_sortie_tor_pop_bit
	mov P1.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 13
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_13:
	acall _fct_sortie_tor_pop_bit
	mov P1.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 14
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_14:
	acall _fct_sortie_tor_pop_bit
	mov P1.6,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 15
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_15:
	acall _fct_sortie_tor_pop_bit
	mov P1.7,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 16
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_16:
	acall _fct_sortie_tor_pop_bit
	mov P4.0,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 17
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_17:
	acall _fct_sortie_tor_pop_bit
	mov P4.1,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 18
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_18:
	acall _fct_sortie_tor_pop_bit
	mov P4.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 19
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_19:
	acall _fct_sortie_tor_pop_bit
	mov P4.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 20
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_20:
	acall _fct_sortie_tor_pop_bit
	mov P4.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 21
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_21:
	acall _fct_sortie_tor_pop_bit
	mov P4.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 22
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_22:
	acall _fct_sortie_tor_pop_bit
	mov P4.6,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 23
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_23:
	acall _fct_sortie_tor_pop_bit
	mov P4.7,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 24
	.else
        ljmp _fct_sortie_tor_fin
        .endif
	
_fct_sortie_tor_24:
	acall _fct_sortie_tor_pop_bit
	mov P3.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 25
	.else
        ljmp _fct_sortie_tor_fin
        .endif

_fct_sortie_tor_25:
	acall _fct_sortie_tor_pop_bit
	mov P3.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 26
	.else
        ljmp _fct_sortie_tor_fin
        .endif

_fct_sortie_tor_26:
	acall _fct_sortie_tor_pop_bit
	mov P3.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 27
	.else
        ljmp _fct_sortie_tor_fin
        .endif

_fct_sortie_tor_27:
	acall _fct_sortie_tor_pop_bit
	mov P3.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 28
	.else
        ljmp _fct_sortie_tor_fin
        .endif

_fct_sortie_tor_fin:
	ret

;-----------------------------------------------------------------------------------------------------------
; _fct_sortie_tor_pop_bit: pop d'un bit dans la zone donnees de la trame
;-----------------------------------------------------------------------------------------------------------
_fct_sortie_tor_pop_bit:            ; push un bit (C) dans la trame donnees.
	mov A,@R0                     ; recupération octet donnes trame
	anl A,R7                      ; mise à un bit de poids R7
	mov R2,A                      ; sauvegarde.

	mov A,R7                      ; rotate du poids.
	clr C                         ; rotate du poids: >> 1
	rrc A
	mov R7,A                      ; decalage pour prochaine mise à un 
	jnz _fct_sortie_tor_pop_bit_fin ; si pas de depassement, fin
	inc R0                        ; sinon octet trame suivant, reset R7
	mov R7,#0x80
_fct_sortie_tor_pop_bit_fin:

	mov A,R2
	jz _fct_sortie_tor_pop_bit_0  ; demande de mise à zero ?
	clr C                         ; MAU sortie -> MAZ patte controleur
	ajmp _fct_sortie_tor_pop_bit_suite
_fct_sortie_tor_pop_bit_0:
	setb C                        ; MAZ sortie -> MAU patte controleur             
_fct_sortie_tor_pop_bit_suite:
	ret
	
;---------------------------------------------------------------------------------------
; _raz_sortie_tor
;---------------------------------------------------------------------------------------

 _raz_sortie_tor:
 	mov A,#NBR_SORTIE_TOR
	;mov R3,A
	jnz _raz_sortie_tor_suite
	ret
 _raz_sortie_tor_suite:
        setb C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 8
	.else
        ljmp _raz_sortie_tor_8
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 9
	.else
        ljmp _raz_sortie_tor_9
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 10
	.else
        ljmp _raz_sortie_tor_10
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 11
	.else
        ljmp _raz_sortie_tor_11
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 12
	.else
        ljmp _raz_sortie_tor_12
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 13
 	.else
        ljmp _raz_sortie_tor_13
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 14
	.else
        ljmp _raz_sortie_tor_14
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 15
	.else
        ljmp _raz_sortie_tor_15
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 16
	.else
        ljmp _raz_sortie_tor_16
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 17
	.else
        ljmp _raz_sortie_tor_17
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 18
	.else
        ljmp _raz_sortie_tor_18
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 19
	.else
        ljmp _raz_sortie_tor_19
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 20
	.else
        ljmp _raz_sortie_tor_20
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 21
	.else
        ljmp _raz_sortie_tor_21
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 22
	.else
        ljmp _raz_sortie_tor_22
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 23
	.else
        ljmp _raz_sortie_tor_23
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 24
	.else
        ljmp _raz_sortie_tor_24
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 25
	.else
        ljmp _raz_sortie_tor_25
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 26
	.else
        ljmp _raz_sortie_tor_26
        .endif
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 27
	.else
        ljmp _raz_sortie_tor_27
        .endif

_raz_sortie_tor_8:
	mov P1.0,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 9
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_9:
	mov P1.1,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 10
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_10:
	mov P1.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 11
	.else
        ljmp _raz_sortie_tor_fin
        .endif

_raz_sortie_tor_11:
	mov P1.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 12
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_12:
	mov P1.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 13
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_13:
	mov P1.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 14
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_14:
	mov P1.6,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 15
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_15:
	mov P1.7,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 16
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_16:
	mov P4.0,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 17
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_17:
	mov P4.1,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 18
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_18:
	mov P4.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 19
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_19:
	mov P4.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 20
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_20:
	mov P4.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 21
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_21:
	mov P4.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 22
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_22:
	mov P4.6,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 23
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_23:
	mov P4.7,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 24
	.else
        ljmp _raz_sortie_tor_fin
        .endif
	
_raz_sortie_tor_24:
	mov P3.2,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 25
	.else
        ljmp _raz_sortie_tor_fin
        .endif

_raz_sortie_tor_25:
	mov P3.3,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 26
	.else
        ljmp _raz_sortie_tor_fin
        .endif

_raz_sortie_tor_26:
	mov P3.4,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 27
	.else
        ljmp _raz_sortie_tor_fin
        .endif

_raz_sortie_tor_27:
	mov P3.5,C
	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 28
	.else
        ljmp _raz_sortie_tor_fin
        .endif

_raz_sortie_tor_fin:
	ret
;-----------------------------------------------------------------------------------------------------------

