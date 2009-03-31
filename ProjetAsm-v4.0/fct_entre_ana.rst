ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; fct_entre_ana.asm    Fonction d'envoi des entrees ANA
                              3 ; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5         .module FctEntreANA
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 2.



                              6 	.include "cst.asm"
                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; cst.asm    Definition des constantes prgs
                              3 ; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5 
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 3.



                              6         .include "conf.asm"                                                       ; Inclusion des constantes 
                    0001      1 ID_ESCLAVE = 1
                    0008      2 NBR_ENTRE_ANA = 8
                    0014      3 NBR_ENTRE_TOR = 20
                    0000      4 NBR_ENTRE_CHOC = 0
                    0000      5 NBR_SORTIE_TOR = 0
                    0000      6 NBR_SORTIE_ANA = 0
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 4.



                              7 ;----------------------------------------Déclarations des prototypes de fonctions --------------------------
                              8         .globl _start                                                                        ; dans init.asm
                              9         .globl TRAME_RECUE
                             10         .globl RX_BUF
                             11         .globl TX_BUF
                             12         .globl INPUT_BUF
                             13         .globl ENTRE_ANA_BUF
                             14         .globl ID_IDENT
                             15         .globl NBR_INT_CAN
                             16 
                             17         .globl _envoyer_rs232                                                               ; dans rs232.asm
                             18         .globl _recu_rs232
                             19         .globl _rs232_purge_trame
                             20 
                             21         .globl _clignotant_run                                                         ; Dans cligno_run.asm
                             22         .globl _del_trame_on
                             23         .globl _del_trame_off
                             24         .globl _cdg_logiciel
                             25         .globl _init_cdg_logiciel
                             26 
                             27         .globl _calcul_crc16                                                                ; dans crc16.asm
                             28 
                             29         .globl _start_of_conversion                                                           ; dans can.asm
                             30         .globl _recu_can
                             31 	.globl _lance_start_of_conversion
                             32         .globl CAN_PRET
                             33 
                             34         .globl _fct_entre_ana                                                       ; dans fct_entre_ana.asm
                             35 
                             36         .globl _fct_entre_tor                                                       ; dans fct_entre_tor.asm
                             37         .globl _raz_sortie_tor
                             38         .globl _polling_entre_tor
                             39 
                             40         .globl _fct_ident                                                               ; dans fct_ident.asm
                             41 
                             42         .globl _fct_sortie_tor                                                     ; dans fct_sortie_tor.asm
                             43 
                             44 ;---------------------------------------- Constantes communes à tous ---------------------------------------
                    00FF     45 	ID_MAITRE         = 0xFF
                    0003     46 	VERSION_MAJOR     = 0x03
                    0001     47 	VERSION_MINOR     = 0x01
                    0018     48 	TAILLE_BUF        = 24
                    0006     49 	TAILLE_ENTETE     = 6
                    00C0     50 	P4                = 0xC0
                    00C0     51 	P4.0              = 0xC0
                    00C1     52 	P4.1              = 0xC1
                    00C2     53 	P4.2              = 0xC2
                    00C3     54 	P4.3              = 0xC3
                    00C4     55 	P4.4              = 0xC4
                    00C5     56 	P4.5              = 0xC5
                    00C6     57 	P4.6              = 0xC6
                    00C7     58 	P4.7              = 0xC7
                             59 
                    00C4     60 	P5                = 0xC4     ; P5 c'est pas D0 ->>>> a voir !!
                    00FF     61 	T3                = 0xFF
                    00C5     62 	ADCON             = 0xC5
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 5.



                    00C6     63 	ADCH              = 0xC6
                    00C8     64 	ADCS              = 0xC8
                    00C9     65 	ADCI              = 0xC9
                    00CC     66 	ADC1              = 0xCC
                    00CB     67 	ADC0              = 0xCB
                    00A8     68         IEN0              = 0xA8
                    0003     69 	IndexTrame_taille = 3
                             70 
                    0001     71 	FCT_IDENT         = 0x01
                    0002     72 	FCT_ENTRE_TOR     = 0x02
                    0003     73 	FCT_ENTRE_ANA     = 0x03
                    0004     74 	FCT_SORTIE_TOR    = 0x04
                    00F0     75 	DEBUG_PRENDRE_LIGNE = 0xF0
                    00F1     76 	DEBUG_LIBERE_LIGNE= 0xF1
                    00FF     77 	FCT_PING          = 0xFF
                             78 
                    0000     79 	BANK0             = 0x00
                    0008     80 	BANK1             = 0x08
                    0010     81 	BANK2             = 0x10
                    0018     82 	BANK3             = 0x18
                             83 ;-----------------------------------------------------------------------------------------------------------
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 6.



                              7 ;-----------------------------------------------------------------------------------------------------------
                              8 ; _fct_entre_ana: Envoi de l'etat des entrées ANA
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area CSEG	(CODE)
   03EE                      11 _fct_entre_ana:
   03EE 78 9E                12 	mov R0,#TX_BUF
   03F0 79 D0                13 	mov R1,#ENTRE_ANA_BUF
   03F2 76 FF                14 	mov @R0, #ID_MAITRE         ; envoie au maitre
   03F4 08                   15 	inc R0
   03F5 76 01                16 	mov @R0, #ID_ESCLAVE        ; source
   03F7 08                   17 	inc R0
   03F8 76 03                18 	mov @R0, #FCT_ENTRE_ANA     ; fonction
   03FA 08                   19 	inc R0
                             20 
   03FB 74 08                21 	mov A,#NBR_ENTRE_ANA       ; R3 = cpt d'entree TOR
   03FD FB                   22 	mov R3,A
   03FE 70 07                23 	jnz _fct_entre_ana_suite    ;
                             24 
   0400 74 00                25         mov A,#0x00
   0402 F6                   26         mov @R0,A                   ; zero entre ANA
                             27 
   0403 12 01 A5             28 	lcall _envoyer_rs232        ; on envoie la trame
   0406 22                   29 	ret
   0407                      30 _fct_entre_ana_suite:
   0407 78 9E                31 	mov R0,#TX_BUF
   0409 79 D0                32 	mov R1,#ENTRE_ANA_BUF
   040B 76 FF                33 	mov @R0, #ID_MAITRE         ; envoie au maitre
   040D 08                   34 	inc R0
   040E 76 01                35 	mov @R0, #ID_ESCLAVE        ; source
   0410 08                   36 	inc R0
   0411 76 03                37 	mov @R0, #FCT_ENTRE_ANA     ; fonction
   0413 08                   38 	inc R0
                             39 
   0414 EB                   40 	mov A,R3                    ; A = nbr entre ana
   0415 14                   41 	dec A                       ; A = nbr_entre_ana - 1
   0416 C3                   42 	clr C
   0417 13                   43 	rrc A                       ; A = (nbr_entre_ana - 1)/2
   0418 C3                   44 	clr C
   0419 13                   45 	rrc A                       ; A = (nbr_entre_ana - 1)/4
   041A 04                   46 	inc A                       ; A = (nbr_entre_ana - 1)/4 + 1
   041B 2B                   47 	add A,R3                    ; A vaut nbr_entre_ana + nbr_octet_suppl_10_bits
   041C FB                   48 	mov R3,A                    ; R3 = Nombre total d'octet donnees
   041D F6                   49         mov @R0,A                   ; ajout dans la trame
   041E 08                   50         inc R0 
                             51 	
   041F                      52 _fct_entre_ana_again:
   041F E7                   53 	mov A,@R1
   0420 F6                   54 	mov @R0,A                   ; taille = nbr entre ana + 1 (10bits can)
   0421 08                   55 	inc R0                      ; R0 pointe sur le debut donnees trame.
   0422 09                   56 	inc R1
   0423 DB FA                57 	djnz R3,_fct_entre_ana_again
                             58 
   0425 12 01 A5             59 	lcall _envoyer_rs232        ; on envoie la trame
   0428 22                   60         ret
                             61 ;-----------------------------------------------------------------------------------------------------------
                             62 
