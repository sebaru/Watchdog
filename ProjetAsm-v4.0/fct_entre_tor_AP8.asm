;-----------------------------------------------------------------------------------------------------------
; fct_entre_tor.asm    Fonction d'envoi des entrees TOR
; Sébastien Lefevre                                                            dim 13 jun 2004 17:02:11 CEST
;-----------------------------------------------------------------------------------------------------------
        .module FctEntreTOR
	.include "cst.asm"
	.area CSEG	(CODE)
;-----------------------------------------------------------------------------------------------------------
; _polling_entre_tor: Surveille les etats entre TOR et les sauvegardes le cas echeant
;-----------------------------------------------------------------------------------------------------------
.if #TYPE_AP - 8
.else

_polling_entre_tor:
        mov R0,#INPUT_BUF
	

_polling_entre_tor_0:
	mov A,P5
	anl A,#0x01
	clr C
	rrc A
        acall _polling_entre_tor_test
	
_polling_entre_tor_1:
	mov A,P5
	anl A,#0x02
        clr C
        rrc A
        rrc A
        acall _polling_entre_tor_test
	
_polling_entre_tor_2:
	mov A,P5
	anl A,#0x04
	clr C
	rrc A
	rrc A
	rrc A
        acall _polling_entre_tor_test

_polling_entre_tor_3:
	mov A,P5
	anl A,#0x08
	clr C
	rrc A
	rrc A
	rrc A
	rrc A
        acall _polling_entre_tor_test

_polling_entre_tor_4:
	mov A,P5
	anl A,#0x10
	clr C
	rlc A
	rlc A
	rlc A
	rlc A
        acall _polling_entre_tor_test
	
_polling_entre_tor_5:
	mov A,P5
	anl A,#0x20
	clr C
	rlc A
	rlc A
	rlc A
        acall _polling_entre_tor_test
	
_polling_entre_tor_6:
	mov A,P5
	anl A,#0x40
	clr C
	rlc A
	rlc A
        acall _polling_entre_tor_test
	
_polling_entre_tor_7:
	mov A,P5
	anl A,#0x80
	clr C
	rlc A
        acall _polling_entre_tor_test

_polling_entre_tor_8:
	mov C,P1.7
        acall _polling_entre_tor_test
	
_polling_entre_tor_9:
	mov C,P1.6
        acall _polling_entre_tor_test
	
_polling_entre_tor_10:
	mov C,P1.5
        acall _polling_entre_tor_test

_polling_entre_tor_11:
	mov C,P1.4
        acall _polling_entre_tor_test
	
_polling_entre_tor_12:
	mov C,P1.3
        acall _polling_entre_tor_test
	
_polling_entre_tor_13:
	mov C,P1.2
        acall _polling_entre_tor_test
	
_polling_entre_tor_14:
	mov C,P1.1
        acall _polling_entre_tor_test
	
_polling_entre_tor_15:
	mov C,P1.0
        acall _polling_entre_tor_test
	
_polling_entre_tor_24:
	mov C,P3.2
        acall _polling_entre_tor_test

_polling_entre_tor_25:
	mov C,P3.3
        acall _polling_entre_tor_test

_polling_entre_tor_26:
	mov C,P3.4
        acall _polling_entre_tor_test

_polling_entre_tor_27:
	mov C,P3.5

_polling_entre_tor_fin:
        ret
;-----------------------------------------------------------------------------------------------------------
; _polling_entre_tor_test: Compare la valeur reelle d'un port avec celle sauvegardée et met à jour le cas
; échéant                                                                                                   
;-----------------------------------------------------------------------------------------------------------
_polling_entre_tor_test:
        mov A,@R0
        mov R1,A                             ; Sauvegarde
	anl A,#0x02                          ; Bit 1: Locké par le PC ou non ??
	;;; desactivé pb fenetre cuisine 10/07/05 
        ;;; activé 11/11/2006
        jnz _polling_entre_tor_test_fin      ; Locké ? -> Pas touche !

        mov A,#0x00                          ; C possede la valeur reelle du port à tester
	rlc A                                ; A recupere la valeur de C
        mov R2,A

        mov A,R1                             ; recup de l'etat n-1
	anl A,#0x01
	subb A,R2
	jz _polling_entre_tor_test_fin       ; rien n'a changé, on se barre
	
        mov A,R2
        orl A,#0x02
        mov @R0,A

_polling_entre_tor_test_fin:
        inc R0
        ret

.endif

;-----------------------------------------------------------------------------------------------------------
; _fct_entre_tor: Envoi de l'etat des entrées TOR
;-----------------------------------------------------------------------------------------------------------
_fct_entre_tor:
	mov R0,#TX_BUF
	mov @R0, #ID_MAITRE           ; envoie au maitre
	inc R0
	mov @R0, #ID_ESCLAVE          ; source
	inc R0
	mov @R0, #FCT_ENTRE_TOR       ; fonction
	inc R0
        mov A,R0
        mov R2,A                      ; R2 pointe sur le champ taille de trame
	inc R0                        ; R0 pointe sur le debut donnees trame.
	mov @R0, #0x00                ; mise à zero
	mov A,#NBR_ENTRE_TOR          ; R3 = cpt d'entree TOR
	mov R3,A
	jnz _fct_entre_tor_suite      ;

        mov A,R2
        mov R1,A
        mov @R1,#0x00                 ; zero entre TOR

	lcall _envoyer_rs232          ; on envoie la trame
	ret
_fct_entre_tor_suite:

	mov R7,#0x80                  ; initialisation du poids
	mov R5,#0x00                  ; Nombre d'octet donnees dans la trame
	
        mov R1,#INPUT_BUF
_fct_entre_tor_suite2:                ; R3 possede le nombre d'entree TOR
        mov A,@R1
        anl A,#0xFD                   ; mise à zero bit 1
        mov @R1,A
        anl A,#0x01                   ; on recupere le bit 0
        rrc A                         ; on le recupere dans le carry
	acall _fct_entre_tor_push_bit
        inc R1
        djnz R3,_fct_entre_tor_suite2

_fct_entre_tor_fin:
	cjne R7,#0x80,_fct_entre_tor_octet_partiel
	ljmp _fct_entre_tor_taille	
_fct_entre_tor_octet_partiel:
	inc R5                        ; l'octet est partiellement utilisé, il faut quand mem le compter
	inc R0                        ; le crc est donc à l'octet suivant
_fct_entre_tor_taille:
        mov A,R2                      ; l'adresse du champs taille etait sauvée dans R2
        mov R1,A
	mov A,R5
	mov @R1,A                     ; On precise la taille de la trame.

	lcall _envoyer_rs232          ; on envoie la trame
	ret

;-----------------------------------------------------------------------------------------------------------
; _fct_entre_tor_push_bit: push d'un bit dans la zone donnees de la trame
;-----------------------------------------------------------------------------------------------------------
_fct_entre_tor_push_bit:            ; push un bit (C) dans la trame donnees.
	jc  _fct_entre_tor_push_1
	mov A,R7                     ; R7 = poids du bit en cours
	cpl A
	mov R6,A                      ; R6 = complément de R7

	mov A,@R0                     ; recupération octet donnes trame
	anl A,R6                      ; mise à zero du bit de poids R7
	mov @R0,A
	sjmp _fct_entre_tor_push_bit_suite
_fct_entre_tor_push_1:
	mov A,@R0                     ; recupération octet donnes trame
	orl A,R7                      ; mise à un bit de poids R7
	mov @R0,A
_fct_entre_tor_push_bit_suite:
	mov A,R7
	clr C                         ; rotate du poids: << 1
	rrc A
	mov R7,A                      ; decalage pour prochaine mise à un 
	jnz _fct_entre_tor_push_bit_fin ; si pas de depassement, fin
	inc R0                        ; sinon octet trame suivant, reset R7
	mov @R0, #0x00                ; mise à zero
	inc R5                        ; un octet de plus consommé.
	mov R7,#0x80
_fct_entre_tor_push_bit_fin:
	ret
