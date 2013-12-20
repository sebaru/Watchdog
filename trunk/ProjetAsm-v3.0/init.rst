ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; init.asm    Initialisation du microcontroleur
                              3 ; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5 	.module Init
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
                              8 ; internal ram data
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area RAMINTERNE_DIRECTE (ABS)        ; ram interne directe (0x0030, au dessus des registres)
   0030                      11 TRAME_RECUE:	.ds 1
   0031                      12 CAN_PRET:       .ds 1
   0032                      13 NBR_INT_CAN:    .ds 1
   0033                      14 ID_IDENT:       .ds 1
                             15 
                             16 	.area RAMINTERNE_INDIRECTE (ABS)        ; ram interne directe (0x0080, au dessus de la ram directe)
   0080                      17 RX_BUF:		.ds TAILLE_ENTETE+TAILLE_BUF
   009E                      18 TX_BUF:         .ds TAILLE_ENTETE+TAILLE_BUF  ; buffer d'émission
   00BC                      19 INPUT_BUF:      .ds NBR_ENTRE_TOR             ; buffer de travail entre TOR
   00D0                      20 ENTRE_ANA_BUF:	.ds 10
                             21 
   00DA                      22 PILE:           .ds 10 ; le fin de la zone est la pile.
                             23 
                             24 ;-----------------------------------------------------------------------------------------------------------
                             25 ; code
                             26 ;-----------------------------------------------------------------------------------------------------------
                             27 	.area CSEG	(CODE)
   007B                      28 _start:			                ; Le debut de la trame est en R1
   007B 75 81 DA             29 	mov SP,#PILE ; 0xE0
   007E 12 01 40             30 	lcall _init_registres           ; initialisation des registres
   0081 12 01 0B             31         lcall _init_rs232               ; initialisation de la rs232
   0084 12 01 3C             32         lcall _init_interruption        ; initialisation des interruptions
                    0008     33         .if #NBR_ENTRE_ANA - 0
   0087 12 01 44             34           lcall _init_can               ; initialisation du convertisseur
                             35         .endif
   008A 12 01 56             36         lcall _init_cdg_maitre          ; initialisation du chien de garde maitre
   008D 12 03 DC             37         lcall _init_cdg_logiciel        ; RAZ du chien de garde logiciel.
   0090 12 03 EB             38 	lcall _del_trame_off		; Diode reception trame offline
                             39 
   0093 12 02 DB             40         lcall _raz_sortie_tor           ; Penser à appeler la fonction de RAZ sortie...
                             41 
   0096                      42 _main_boucle:                           ; boucle principale du programme
   0096 12 03 B9             43 	lcall _clignotant_run           ; cligno voyant RUN
   0099 12 01 35             44 	lcall _top_cdg                  ; chien de garde materiel
   009C 12 03 CB             45 	lcall _cdg_logiciel             ; chien de garde logiciel
                             46 
   009F 12 04 29             47         lcall _polling_entre_tor        ; Polling d'evenements entre TOR
                             48 
                    0008     49         .if #NBR_ENTRE_ANA - 0
   00A2 E5 31                50 	  mov A,CAN_PRET
   00A4 60 03                51 	  jz  can_pas_pret
   00A6 12 03 27             52 	  lcall _lance_start_of_conversion
   00A9                      53 can_pas_pret:
                             54         .endif
                             55 ;------------------------------ Test d'arrivage d'une trame RS232 ------------------------------------------
   00A9 E5 30                56 	mov A,TRAME_RECUE
   00AB B4 01 E8             57 	cjne A,#0x01,_main_boucle
   00AE C2 8C                58 	clr  TR0
                             59 	;clr ES
                             60 
   00B0 78 80                61 	mov R0,#RX_BUF                      ; on se place au debut de la trame
   00B2 B6 01 48             62 	cjne @R0,#ID_ESCLAVE,_pas_pour_nous ; si c'est pas nous, on se barre
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 7.



                             63 
   00B5 12 01 E8             64 	lcall _calcul_crc16             ; on calcule le crc 16 associé.
   00B8 EC                   65 	mov A,R4
   00B9 C3                   66         clr C                           ; Carry = 0 pour la soustraction
   00BA 96                   67 	subb A,@R0                      ; A = crc16 trame - crc16 calculé
   00BB 70 40                68 	jnz _trame_traite               ; Comparaison poids fort CRC16
   00BD 08                   69 	inc R0                          ; champs CRC16, pf
   00BE ED                   70 	mov A,R5
   00BF C3                   71         clr C                           ; Carry = 0 pour la soustraction
   00C0 96                   72 	subb A,@R0                      ; A = crc16 trame - crc16 calculé
   00C1 70 3A                73 	jnz _trame_traite               ; Comparaison poids faible CRC16
                             74 
   00C3 78 80                75 	mov R0,#RX_BUF
   00C5 08                   76         inc R0                          ; nous sommes sur le champ source
   00C6 08                   77 	inc R0                          ; nous sommes sur le code fonction
                             78 
   00C7 B6 01 06             79 	cjne @R0,#FCT_IDENT,_pas_fct_ident
   00CA 12 02 13             80 	lcall _fct_ident                ; Appelle de la fonction d'identification
   00CD 02 00 FD             81 	ljmp _trame_traite
   00D0                      82 _pas_fct_ident:
                             83 
   00D0 B6 02 0C             84 	cjne @R0,#FCT_ENTRE_TOR,_pas_fct_entre_tor
   00D3 12 02 64             85 	lcall _fct_sortie_tor           ; on met à jour d'abord les sorties
   00D6 12 03 DC             86 	lcall _init_cdg_logiciel        ; RAZ Chien de garde sur reception etats SORTIES TOR
   00D9 12 04 E2             87 	lcall _fct_entre_tor            ; puis on envoie les entrées au maitre
   00DC 02 00 FD             88 	ljmp _trame_traite
   00DF                      89 _pas_fct_entre_tor:
                             90 
   00DF B6 03 06             91 	cjne @R0,#FCT_ENTRE_ANA,_pas_fct_entre_ana
   00E2 12 03 EE             92 	lcall _fct_entre_ana
   00E5 02 00 FD             93 	ljmp _trame_traite
   00E8                      94 _pas_fct_entre_ana:
                             95 
                             96 ;	cjne @R0,#FCT_SORTIE_TOR,_pas_fct_sortie_tor
                             97 ;	lcall _fct_sortie_tor
                             98 ;	ljmp _trame_traite
                             99 ;_pas_fct_sortie_tor:
                            100         ;lcall _fct_ident ; pour debug
                            101 
   00E8 B6 F0 06            102 	cjne @R0,#DEBUG_PRENDRE_LIGNE,_pas_debug_prendre_ligne
   00EB 12 03 E8            103 	lcall _del_trame_on
   00EE 02 00 FD            104 	ljmp _trame_traite
                            105 
   00F1                     106 _pas_debug_prendre_ligne:	
   00F1 B6 F1 06            107 	cjne @R0,#DEBUG_LIBERE_LIGNE,_pas_debug_libere_ligne
   00F4 12 03 EB            108 	lcall _del_trame_off
   00F7 02 00 FD            109 	ljmp _trame_traite
   00FA                     110 _pas_debug_libere_ligne:
                            111 
   00FA 02 00 FD            112         ljmp _trame_traite
   00FD                     113 _pas_pour_nous:
                            114 
   00FD                     115 _trame_traite:
   00FD 75 30 00            116 	mov TRAME_RECUE,#0x00           ; Acquit de la trame
   0100 75 8C 00            117         mov  TH0,#0x00
   0103 75 8A 01            118         mov  TL0,#0x01                  ; Reset Timer 0 Watchdog maitre
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 8.



   0106 D2 8C               119         setb TR0
                            120 	;set ES
   0108 02 00 96            121 	ljmp _main_boucle               ; rebouclage
                            122 
                            123 
                            124 ;-----------------------------------------------------------------------------------------------------------
                            125 ; Initialisation de la RS232
                            126 ;-----------------------------------------------------------------------------------------------------------
   010B                     127 _init_rs232:
   010B 75 30 00            128         mov TRAME_RECUE,#0x00
   010E 43 89 20            129         orl TMOD,#0x20          ; Timer 1 en mode 2
   0111 75 8D FD            130         mov TH1,#0xFD           ;70=75 bauds e8=1200bauds fd=9600bauds
   0114 85 8D 8B            131         mov TL1,TH1             ;70=75 bauds e8=1200bauds fd=9600bauds
   0117 43 87 80            132         orl PCON,#0x80          ; ca marche mieux sans 
   011A 75 98 5A            133         mov SCON,#0x5A          ; parametrage RS232
   011D D2 8E               134         setb TR1                ; activation timer
                            135 
   011F C0 D0               136         push PSW
   0121 75 D0 18            137 	mov PSW,#BANK3          ; Reception RS232
   0124 78 80               138 	mov R0,#RX_BUF
   0126 12 01 CE            139         lcall _rs232_purge_trame
   0129 78 80               140         mov R0,#RX_BUF          ; Debut de trame
   012B 7A 00               141         mov R2,#0x00            ; Nbr de caractere recu.
   012D 75 D0 10            142 	mov PSW,#BANK2   ; encore utilisé ???
   0130 78 9E               143         mov R0,#TX_BUF   ; idem
   0132 D0 D0               144         pop PSW
   0134 22                  145         ret
                            146 ;-----------------------------------------------------------------------------------------------------------
                            147 ; Chien de garde materiel
                            148 ;-----------------------------------------------------------------------------------------------------------
   0135                     149 _top_cdg:
   0135 43 87 10            150 	ORL PCON,#0x10          ; Reset Timer3 (overflow=reset controleur)
   0138 75 FF 00            151 	MOV T3,#0x00
   013B 22                  152 	RET	
                            153 ;-----------------------------------------------------------------------------------------------------------
                            154 ; initialisation des interruptions
                            155 ;-----------------------------------------------------------------------------------------------------------
   013C                     156 _init_interruption:
   013C 75 A8 D2            157         mov IEN0,#0xD2            ; valide les IRQs dont bit irq5 port série
   013F 22                  158 	ret
                            159 ;-----------------------------------------------------------------------------------------------------------
                            160 ; initialisation des registres
                            161 ;-----------------------------------------------------------------------------------------------------------
   0140                     162 _init_registres:
   0140 75 89 00            163 	mov TMOD,#0x00
   0143 22                  164 	ret
                            165 ;-----------------------------------------------------------------------------------------------------------
                            166 ; _init_can
                            167 ;-----------------------------------------------------------------------------------------------------------
   0144                     168 _init_can:
   0144 75 31 01            169 	mov  CAN_PRET,#0x01
   0147 75 32 00            170         mov  NBR_INT_CAN,#0x00
   014A C0 D0               171         push PSW
   014C 75 D0 10            172         mov  PSW,#BANK2
   014F 7A 01               173         mov  R2,#0x01
   0151 7B 01               174         mov  R3,#0x01
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 9.



   0153 D0 D0               175         pop  PSW
   0155 22                  176 	ret
                            177 ;-----------------------------------------------------------------------------------------------------------
                            178 ; _init_can
                            179 ;-----------------------------------------------------------------------------------------------------------
   0156                     180 _init_cdg_maitre:
   0156 43 89 01            181         orl TMOD,#0x01          ; Timer 0 en mode 1
   0159 75 8C 00            182         mov TH0,#0x00           ; trouver la valeur pour 1 seconde
   015C 85 8C 8A            183         mov TL0,TH0             ; recopie pour base
   015F D2 8C               184         setb TR0                ; timer 0 ON
   0161 22                  185 	ret
                            186 ;-----------------------------------------------------------------------------------------------------------
                            187 
                            188 
                            189 
                            190 
                            191 
                            192 
                            193 
                            194 
                            195 ;----------------------------------------
                            196 ; Attente 
                            197 ;----------------------------------------
   0162                     198 _attendre:
   0162 C2 AF               199         clr EA
   0164 7B FF               200 	mov R3,#0xFF
   0166                     201 _attenteR3:
   0166 7A FF               202 	mov R2,#0xFF
   0168                     203 _attenteR2:
   0168 DA FE               204 	djnz R2,_attenteR2
   016A DB FA               205 	djnz R3,_attenteR3
   016C D2 AF               206 	setb EA
   016E 22                  207 	ret
