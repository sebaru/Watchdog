ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; fct_ident.asm    Fonction d'identification
                              3 ; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5         .module FctIDENT
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
                              8 ; _fct_ident: envoie d'une trame d'identification
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area CSEG	(CODE)
   0213                      11 _fct_ident:
   0213 78 9E                12 	mov R0,#TX_BUF
   0215 76 FF                13         mov @R0, #ID_MAITRE         ; envoie au maitre
   0217 08                   14         inc R0
   0218 76 01                15 	mov @R0, #ID_ESCLAVE        ; source
   021A 08                   16         inc R0
   021B 76 01                17 	mov @R0, #FCT_IDENT         ; fonction ident
   021D 08                   18         inc R0
   021E 76 0D                19 	mov @R0, #0x0D              ; taille
                             20 
   0220 08                   21         inc R0
   0221 C0 E0                22         push ACC
   0223 E5 33                23         mov A,ID_IDENT
   0225 04                   24         inc A
   0226 F5 33                25         mov ID_IDENT,A
   0228 F6                   26         mov @R0,A
   0229 D0 E0                27         pop ACC
                             28 
   022B 08                   29         inc R0
   022C 76 03                30 	mov @R0, #VERSION_MAJOR     ; majeur
   022E 08                   31         inc R0
   022F 76 01                32 	mov @R0, #VERSION_MINOR     ; mineur
   0231 08                   33         inc R0
   0232 76 08                34 	mov @R0, #NBR_ENTRE_ANA     ; Nombre d'entrée ANA gérées
   0234 08                   35         inc R0
   0235 76 14                36 	mov @R0, #NBR_ENTRE_TOR     ; Nombre d'entrée TOR gérées
   0237 08                   37         inc R0
   0238 76 00                38 	mov @R0, #NBR_ENTRE_CHOC    ; dont entrées choc
   023A 08                   39         inc R0
   023B 76 00                40 	mov @R0, #NBR_SORTIE_TOR    ; Nombre de sorties TOR gérées
                             41 
   023D 08                   42         inc R0
   023E E5 8C                43         mov A,TH0
   0240 F6                   44 	mov @R0, A                  ; Watchdog Maitre Timer 0
   0241 08                   45         inc R0
   0242 E5 8A                46         mov A,TL0
   0244 F6                   47 	mov @R0, A                  ; Watchdog Maitre Timer 0
                             48 
   0245 08                   49         inc R0
   0246 A6 FF                50 	mov @R0, T3                ; Watchdog Materiel Timer 3
                             51 
   0248 08                   52         inc R0
   0249 E5 32                53         mov A,NBR_INT_CAN
   024B F6                   54 	mov @R0, A                 ; Nombre d'interruption CAN
                             55 
   024C 08                   56         inc  R0
   024D C0 D0                57         push PSW
   024F 75 D0 10             58         mov  PSW,#BANK2
   0252 EA                   59         mov  A,R2
   0253 D0 D0                60         pop  PSW
   0255 F6                   61         mov  @R0,A
                             62 
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 7.



   0256 08                   63         inc  R0
   0257 C0 D0                64         push PSW
   0259 75 D0 10             65         mov  PSW,#BANK2
   025C EB                   66         mov  A,R3
   025D D0 D0                67         pop  PSW
   025F F6                   68         mov  @R0,A
                             69 
   0260 12 01 A5             70 	lcall _envoyer_rs232        ; on envoie la trame
   0263 22                   71 	ret
                             72 ;-----------------------------------------------------------------------------------------------------------
