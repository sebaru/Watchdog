ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; cligno_run.asm    Gestion des DELs RUN et TRAME
                              3 ; SÃ©bastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5 	.module Clignotant_RUN
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
                              8 ; _clignotant_run   Complement de la diode de fonctionnement
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area CSEG	(CODE)
   03B9                      11 _clignotant_run:
   03B9 C0 D0                12 	push PSW
   03BB 75 D0 10             13 	mov PSW,#BANK2
   03BE DF 08                14 	djnz R7,_clignotant_run_fin
   03C0 7F 0F                15 	mov R7,#0x0F
   03C2 DE 04                16 	djnz R6,_clignotant_run_fin
   03C4 7E 10                17 	mov R6,#0x10
   03C6 B2 B7                18 	cpl P3.7
   03C8                      19 _clignotant_run_fin:
   03C8 D0 D0                20 	pop PSW
   03CA 22                   21 	ret
                             22 ;-----------------------------------------------------------------------------------------------------------
                             23 ; _cdg_logiciel  Stoppe les sorties si le maitre ne repond pas assez souvent
                             24 ;-----------------------------------------------------------------------------------------------------------
                             25 	.area CSEG	(CODE)
   03CB                      26 _cdg_logiciel:
   03CB C0 D0                27 	push PSW
   03CD 75 D0 10             28 	mov PSW,#BANK2
   03D0 DD 07                29 	djnz R5,_cdg_logiciel_fin
   03D2 7D FF                30 	mov R5,#0xFF
   03D4 DC 03                31 	djnz R4,_cdg_logiciel_fin
   03D6 12 02 DB             32         lcall _raz_sortie_tor
   03D9                      33 _cdg_logiciel_fin:
   03D9 D0 D0                34 	pop PSW
   03DB 22                   35 	ret
                             36 
   03DC                      37 _init_cdg_logiciel:
   03DC C0 D0                38 	push PSW
   03DE 75 D0 10             39 	mov PSW,#BANK2
   03E1 7D FF                40 	mov R5,#0xFF
   03E3 7C FF                41 	mov R4,#0xFF
   03E5 D0 D0                42 	pop PSW
   03E7 22                   43 	ret
                             44 
                             45 ;-----------------------------------------------------------------------------------------------------------
                             46 ; _del_trame_xx  Allumage ou Extinction de la diode de presence trame
                             47 ;-----------------------------------------------------------------------------------------------------------
   03E8                      48 _del_trame_on:
   03E8 D2 B6                49 	setb P3.6
   03EA 22                   50 	ret
   03EB                      51 _del_trame_off:
   03EB C2 B6                52 	clr P3.6
   03ED 22                   53 	ret
                             54 
                             55 ;-----------------------------------------------------------------------------------------------------------
