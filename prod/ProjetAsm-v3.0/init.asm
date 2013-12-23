;-----------------------------------------------------------------------------------------------------------
; init.asm    Initialisation du microcontroleur
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
	.module Init
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; internal ram data
;-----------------------------------------------------------------------------------------------------------
	.area RAMINTERNE_DIRECTE (ABS)        ; ram interne directe (0x0030, au dessus des registres)
TRAME_RECUE:	.ds 1
CAN_PRET:       .ds 1
NBR_INT_CAN:    .ds 1
ID_IDENT:       .ds 1

	.area RAMINTERNE_INDIRECTE (ABS)        ; ram interne directe (0x0080, au dessus de la ram directe)
RX_BUF:		.ds TAILLE_ENTETE+TAILLE_BUF
TX_BUF:         .ds TAILLE_ENTETE+TAILLE_BUF  ; buffer d'émission
INPUT_BUF:      .ds NBR_ENTRE_TOR             ; buffer de travail entre TOR
ENTRE_ANA_BUF:	.ds 10

PILE:           .ds 10 ; le fin de la zone est la pile.

;-----------------------------------------------------------------------------------------------------------
; code
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_start:			                ; Le debut de la trame est en R1
	mov SP,#PILE ; 0xE0
	lcall _init_registres           ; initialisation des registres
        lcall _init_rs232               ; initialisation de la rs232
        lcall _init_interruption        ; initialisation des interruptions
        .if #NBR_ENTRE_ANA - 0
          lcall _init_can               ; initialisation du convertisseur
        .endif
        lcall _init_cdg_maitre          ; initialisation du chien de garde maitre
        lcall _init_cdg_logiciel        ; RAZ du chien de garde logiciel.
	lcall _del_trame_off		; Diode reception trame offline

        lcall _raz_sortie_tor           ; Penser à appeler la fonction de RAZ sortie...

_main_boucle:                           ; boucle principale du programme
	lcall _clignotant_run           ; cligno voyant RUN
	lcall _top_cdg                  ; chien de garde materiel
	lcall _cdg_logiciel             ; chien de garde logiciel

        lcall _polling_entre_tor        ; Polling d'evenements entre TOR

        .if #NBR_ENTRE_ANA - 0
	  mov A,CAN_PRET
	  jz  can_pas_pret
	  lcall _lance_start_of_conversion
can_pas_pret:
        .endif
;------------------------------ Test d'arrivage d'une trame RS232 ------------------------------------------
	mov A,TRAME_RECUE
	cjne A,#0x01,_main_boucle
	clr  TR0
	;clr ES

	mov R0,#RX_BUF                      ; on se place au debut de la trame
	cjne @R0,#ID_ESCLAVE,_pas_pour_nous ; si c'est pas nous, on se barre

	lcall _calcul_crc16             ; on calcule le crc 16 associé.
	mov A,R4
        clr C                           ; Carry = 0 pour la soustraction
	subb A,@R0                      ; A = crc16 trame - crc16 calculé
	jnz _trame_traite               ; Comparaison poids fort CRC16
	inc R0                          ; champs CRC16, pf
	mov A,R5
        clr C                           ; Carry = 0 pour la soustraction
	subb A,@R0                      ; A = crc16 trame - crc16 calculé
	jnz _trame_traite               ; Comparaison poids faible CRC16

	mov R0,#RX_BUF
        inc R0                          ; nous sommes sur le champ source
	inc R0                          ; nous sommes sur le code fonction

	cjne @R0,#FCT_IDENT,_pas_fct_ident
	lcall _fct_ident                ; Appelle de la fonction d'identification
	ljmp _trame_traite
_pas_fct_ident:

	cjne @R0,#FCT_ENTRE_TOR,_pas_fct_entre_tor
	lcall _fct_sortie_tor           ; on met à jour d'abord les sorties
	lcall _init_cdg_logiciel        ; RAZ Chien de garde sur reception etats SORTIES TOR
	lcall _fct_entre_tor            ; puis on envoie les entrées au maitre
	ljmp _trame_traite
_pas_fct_entre_tor:

	cjne @R0,#FCT_ENTRE_ANA,_pas_fct_entre_ana
	lcall _fct_entre_ana
	ljmp _trame_traite
_pas_fct_entre_ana:

;	cjne @R0,#FCT_SORTIE_TOR,_pas_fct_sortie_tor
;	lcall _fct_sortie_tor
;	ljmp _trame_traite
;_pas_fct_sortie_tor:
        ;lcall _fct_ident ; pour debug

	cjne @R0,#DEBUG_PRENDRE_LIGNE,_pas_debug_prendre_ligne
	lcall _del_trame_on
	ljmp _trame_traite

_pas_debug_prendre_ligne:	
	cjne @R0,#DEBUG_LIBERE_LIGNE,_pas_debug_libere_ligne
	lcall _del_trame_off
	ljmp _trame_traite
_pas_debug_libere_ligne:

        ljmp _trame_traite
_pas_pour_nous:

_trame_traite:
	mov TRAME_RECUE,#0x00           ; Acquit de la trame
        mov  TH0,#0x00
        mov  TL0,#0x01                  ; Reset Timer 0 Watchdog maitre
        setb TR0
	;set ES
	ljmp _main_boucle               ; rebouclage


;-----------------------------------------------------------------------------------------------------------
; Initialisation de la RS232
;-----------------------------------------------------------------------------------------------------------
_init_rs232:
        mov TRAME_RECUE,#0x00
        orl TMOD,#0x20          ; Timer 1 en mode 2
        mov TH1,#0xFD           ;70=75 bauds e8=1200bauds fd=9600bauds
        mov TL1,TH1             ;70=75 bauds e8=1200bauds fd=9600bauds
        orl PCON,#0x80          ; ca marche mieux sans 
        mov SCON,#0x5A          ; parametrage RS232
        setb TR1                ; activation timer

        push PSW
	mov PSW,#BANK3          ; Reception RS232
	mov R0,#RX_BUF
        lcall _rs232_purge_trame
        mov R0,#RX_BUF          ; Debut de trame
        mov R2,#0x00            ; Nbr de caractere recu.
	mov PSW,#BANK2   ; encore utilisé ???
        mov R0,#TX_BUF   ; idem
        pop PSW
        ret
;-----------------------------------------------------------------------------------------------------------
; Chien de garde materiel
;-----------------------------------------------------------------------------------------------------------
_top_cdg:
	ORL PCON,#0x10          ; Reset Timer3 (overflow=reset controleur)
	MOV T3,#0x00
	RET	
;-----------------------------------------------------------------------------------------------------------
; initialisation des interruptions
;-----------------------------------------------------------------------------------------------------------
_init_interruption:
        mov IEN0,#0xD2            ; valide les IRQs dont bit irq5 port série
	ret
;-----------------------------------------------------------------------------------------------------------
; initialisation des registres
;-----------------------------------------------------------------------------------------------------------
_init_registres:
	mov TMOD,#0x00
	ret
;-----------------------------------------------------------------------------------------------------------
; _init_can
;-----------------------------------------------------------------------------------------------------------
_init_can:
	mov  CAN_PRET,#0x01
        mov  NBR_INT_CAN,#0x00
        push PSW
        mov  PSW,#BANK2
        mov  R2,#0x01
        mov  R3,#0x01
        pop  PSW
	ret
;-----------------------------------------------------------------------------------------------------------
; _init_can
;-----------------------------------------------------------------------------------------------------------
_init_cdg_maitre:
        orl TMOD,#0x01          ; Timer 0 en mode 1
        mov TH0,#0x00           ; trouver la valeur pour 1 seconde
        mov TL0,TH0             ; recopie pour base
        setb TR0                ; timer 0 ON
	ret
;-----------------------------------------------------------------------------------------------------------








;----------------------------------------
; Attente 
;----------------------------------------
_attendre:
        clr EA
	mov R3,#0xFF
_attenteR3:
	mov R2,#0xFF
_attenteR2:
	djnz R2,_attenteR2
	djnz R3,_attenteR3
	setb EA
	ret
