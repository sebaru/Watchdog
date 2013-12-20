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

.if #TYPE_AP - 8
.else

_fct_sortie_tor:
_fct_sortie_tor_suite:
	mov A,#RX_BUF
	add A,#0x04
	mov R0,A                    ; R0 pointe sur le premier champ de donnees.

	mov R7,#0x80                ; initialisation du poids
	mov R5,#0x00                ; Nombre d'octet donnees dans la trame
	
_fct_sortie_tor_16:
	acall _fct_sortie_tor_pop_bit
	mov P4.0,C
	
_fct_sortie_tor_17:
	acall _fct_sortie_tor_pop_bit
	mov P4.1,C
	
_fct_sortie_tor_18:
	acall _fct_sortie_tor_pop_bit
	mov P4.2,C
	
_fct_sortie_tor_19:
	acall _fct_sortie_tor_pop_bit
	mov P4.3,C
	
_fct_sortie_tor_20:
	acall _fct_sortie_tor_pop_bit
	mov P4.4,C
	
_fct_sortie_tor_21:
	acall _fct_sortie_tor_pop_bit
	mov P4.5,C
	
_fct_sortie_tor_22:
	acall _fct_sortie_tor_pop_bit
	mov P4.6,C
	
_fct_sortie_tor_23:
	acall _fct_sortie_tor_pop_bit
	mov P4.7,C
	
_fct_sortie_tor_fin:
	ret

.endif

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
 _raz_sortie_tor_suite:
        setb C

_raz_sortie_tor_16:
	mov P4.0,C
	
_raz_sortie_tor_17:
	mov P4.1,C
	
_raz_sortie_tor_18:
	mov P4.2,C
	
_raz_sortie_tor_19:
	mov P4.3,C
	
_raz_sortie_tor_20:
	mov P4.4,C
	
_raz_sortie_tor_21:
	mov P4.5,C
	
_raz_sortie_tor_22:
	mov P4.6,C
	
_raz_sortie_tor_23:
	mov P4.7,C
	
_raz_sortie_tor_fin:
	ret
;-----------------------------------------------------------------------------------------------------------

