;-----------------------------------------------------------------------------------------------------------
; cst.asm    Definition des constantes prgs
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
  ; v4.0 Séparation en type_APxx et reactivation anti-anti-rebond

  ; v3.1 FCT_IDENT: améliorations sensibles et reprises CAN opérationnel
  ;      Désactivation fonction anti-anti-rebond suite pb fenetre cuisine
  ; v3.0 Ajout fonction anti-anti-rebonds
        .include "conf.asm"                                                       ; Inclusion des constantes 
;----------------------------------------Déclarations des prototypes de fonctions --------------------------
        .globl _start                                                                        ; dans init.asm
        .globl TRAME_RECUE
        .globl RX_BUF
        .globl TX_BUF
        .globl INPUT_BUF
        .globl ENTRE_ANA_BUF
        .globl ID_IDENT
        .globl NBR_INT_CAN

        .globl _envoyer_rs232                                                               ; dans rs232.asm
        .globl _recu_rs232
        .globl _rs232_purge_trame

        .globl _clignotant_run                                                         ; Dans cligno_run.asm
        .globl _del_trame_on
        .globl _del_trame_off
        .globl _cdg_logiciel
        .globl _init_cdg_logiciel

        .globl _calcul_crc16                                                                ; dans crc16.asm

        .globl _start_of_conversion                                                           ; dans can.asm
        .globl _recu_can
	.globl _lance_start_of_conversion
        .globl CAN_PRET

        .globl _fct_entre_ana                                                       ; dans fct_entre_ana.asm

        .globl _fct_entre_tor                                                       ; dans fct_entre_tor.asm
        .globl _raz_sortie_tor
        .globl _polling_entre_tor

        .globl _fct_ident                                                               ; dans fct_ident.asm

        .globl _fct_sortie_tor                                                     ; dans fct_sortie_tor.asm

;---------------------------------------- Constantes communes à tous ---------------------------------------
	ID_MAITRE         = 0xFF
	VERSION_MAJOR     = 0x04
	VERSION_MINOR     = 0x00
	TAILLE_BUF        = 24
	TAILLE_ENTETE     = 6
	P4                = 0xC0
	P4.0              = 0xC0
	P4.1              = 0xC1
	P4.2              = 0xC2
	P4.3              = 0xC3
	P4.4              = 0xC4
	P4.5              = 0xC5
	P4.6              = 0xC6
	P4.7              = 0xC7

	P5                = 0xC4     ; P5 c'est pas D0 ->>>> a voir !!
	T3                = 0xFF
	ADCON             = 0xC5
	ADCH              = 0xC6
	ADCS              = 0xC8
	ADCI              = 0xC9
	ADC1              = 0xCC
	ADC0              = 0xCB
        IEN0              = 0xA8
	IndexTrame_taille = 3

	FCT_IDENT         = 0x01
	FCT_ENTRE_TOR     = 0x02
	FCT_ENTRE_ANA     = 0x03
	FCT_SORTIE_TOR    = 0x04
	DEBUG_PRENDRE_LIGNE = 0xF0
	DEBUG_LIBERE_LIGNE= 0xF1
	FCT_PING          = 0xFF

	BANK0             = 0x00
	BANK1             = 0x08
	BANK2             = 0x10
	BANK3             = 0x18
;-----------------------------------------------------------------------------------------------------------
