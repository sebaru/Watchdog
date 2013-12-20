ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; can.asm    Gestion des conversion analogiques
                              3 ; SÈbastien Lefevre                                                             sam 13 nov 2004 19:05:43 CET 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5         .module Can
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 2.



                              6 	.include "cst.asm"
                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; cst.asm    Definition des constantes prgs
                              3 ; SÈbastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
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



                              7 ;----------------------------------------DÈclarations des prototypes de fonctions --------------------------
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
                             44 ;---------------------------------------- Constantes communes ‡ tous ---------------------------------------
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
                              8 ; _start_of_conversion IRQ2 Timer 
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area CSEG	(CODE)
   030D                      11 _start_of_conversion:
   030D C0 D0                12         push PSW                ; ranger le registre
   030F C0 E0                13         push ACC
   0311 E5 C5                14 	mov A,ADCON                 ; on recupere le registre de conf
   0313 53 C5 F8             15 	anl ADCON,#0xF8             ; selection de l'ea 0
   0316 54 07                16 	anl A, #0x07                ; respect des limites.
   0318 04                   17 	inc A                       ; choix de l'ea suivante
   0319 B4 08 02             18 	cjne A,#NBR_ENTRE_ANA,_start_of_conversion_normal
   031C 74 00                19 	mov A,#0x00
   031E                      20 _start_of_conversion_normal:
   031E 24 08                21 	add A,#0x08                 ; ADCS = 1
   0320 42 C5                22         orl ADCON,A
   0322 D0 E0                23         pop  ACC                    ;
   0324 D0 D0                24         pop  PSW                    ; rÈactive la banque registres 0  
   0326 22                   25 	ret
                             26 ;-----------------------------------------------------------------------------------------------------------
                             27 ; _clignotant_run   Complement de la diode de fonctionnement
                             28 ;-----------------------------------------------------------------------------------------------------------
                             29 	.area CSEG	(CODE)
   0327                      30 _lance_start_of_conversion:
   0327 C0 D0                31 	push PSW
   0329 75 D0 10             32 	mov PSW,#BANK2
   032C DB 08                33 	djnz R3,_lance_start_of_conversion_fin
   032E 7B FF                34 	mov R3,#0xFF
                             35 	;djnz R2,_lance_start_of_conversion_fin  pitete a virer ??
                             36 	;mov R2,#0x10
   0330 75 31 00             37         mov CAN_PRET,#0x00
   0333 12 03 0D             38         lcall _start_of_conversion
   0336                      39 _lance_start_of_conversion_fin:
   0336 D0 D0                40 	pop PSW
   0338 22                   41         ret
                             42 
                             43 ;-----------------------------------------------------------------------------------------------------------
                             44 ; _recu_can: IRQ11 C.A.N
                             45 ;-----------------------------------------------------------------------------------------------------------
   0339                      46 _recu_can:
   0339 C0 D0                47         push PSW                ; ranger le registre
   033B C0 E0                48         push ACC
   033D E5 32                49         mov  A,NBR_INT_CAN
   033F 04                   50         inc  A
   0340 F5 32                51         mov  NBR_INT_CAN,A
   0342 75 D0 08             52         mov  PSW,#BANK1
   0345 E5 C5                53         mov A,ADCON                 ; on recupere le registre de conf
   0347 54 07                54 	anl A, #0x07                ; respect des limites. A = ea selectionÈe
   0349 FA                   55 	mov R2,A                    ; sauvegarde pour utilisation future
                             56 
   034A 78 D0                57         mov  R0,#ENTRE_ANA_BUF      ; debut buffer
   034C 28                   58         add A,R0
   034D F9                   59         mov R1,A                    ; R1 = octet correspondant buffer ea
   034E A7 C6                60         mov @R1,ADCH                ; sauvegarde dans buffer 8 bits
                             61 
   0350 EA                   62 	mov A,R2                    ; NumÈro de l'entrÈe selectionnÈe
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 7.



   0351 C3                   63 	clr C
   0352 13                   64 	rrc A
   0353 C3                   65 	clr C
   0354 13                   66 	rrc A                       ; Division par 4
   0355 28                   67 	add A,R0                    ; Debut buffer
   0356 24 08                68         add A,#NBR_ENTRE_ANA        ; + Nombre entre ana -> on pointe sur le bon octet 10 bits
   0358 F9                   69         mov R1,A                    ; Buffer 10 bits
                             70 
   0359 EA                   71 	mov A,R2
   035A 54 03                72 	anl A,#0x03
   035C 60 49                73 	jz _recu_can_0_rotate
   035E C3                   74         clr C                           ; Carry = 0 pour la soustraction
   035F 94 01                75 	subb A,#0x01
   0361 60 33                76         jz _recu_can_1_rotate
   0363 C3                   77         clr C                           ; Carry = 0 pour la soustraction
   0364 94 01                78 	subb A,#0x01
   0366 60 19                79         jz _recu_can_2_rotate
                             80 
   0368 E7                   81 	mov A,@R1
   0369 54 FC                82 	anl A,#0xFC
   036B FB                   83 	mov R3,A
   036C E5 C5                84 	mov A,ADCON
   036E 54 C0                85 	anl A,#0xC0
   0370 C3                   86 	clr C
   0371 13                   87         rrc A
   0372 C3                   88         clr C
   0373 13                   89         rrc A
   0374 C3                   90 	clr C
   0375 13                   91         rrc A
   0376 C3                   92         clr C
   0377 13                   93         rrc A
   0378 C3                   94 	clr C
   0379 13                   95         rrc A
   037A C3                   96         clr C
   037B 13                   97         rrc A
   037C 4B                   98 	orl A,R3
   037D F7                   99 	mov @R1,A        
   037E 02 03 B1            100 	ljmp _recu_can_rotate_end
                            101 
   0381                     102 _recu_can_2_rotate:
   0381 E7                  103 	mov A,@R1
   0382 54 F3               104 	anl A,#0xF3
   0384 FB                  105 	mov R3,A
   0385 E5 C5               106 	mov A,ADCON
   0387 54 C0               107 	anl A,#0xC0
   0389 C3                  108 	clr C
   038A 13                  109         rrc A
   038B C3                  110         clr C
   038C 13                  111         rrc A
   038D C3                  112 	clr C
   038E 13                  113         rrc A
   038F C3                  114         clr C
   0390 13                  115         rrc A
   0391 4B                  116 	orl A,R3
   0392 F7                  117 	mov @R1,A        
   0393 02 03 B1            118 	ljmp _recu_can_rotate_end
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 8.



                            119 
   0396                     120 _recu_can_1_rotate:
   0396 E7                  121 	mov A,@R1
   0397 54 CF               122 	anl A,#0xCF
   0399 FB                  123 	mov R3,A
   039A E5 C5               124 	mov A,ADCON
   039C 54 C0               125 	anl A,#0xC0
   039E C3                  126 	clr C
   039F 13                  127         rrc A
   03A0 C3                  128         clr C
   03A1 13                  129         rrc A
   03A2 4B                  130 	orl A,R3
   03A3 F7                  131 	mov @R1,A        
   03A4 02 03 B1            132 	ljmp _recu_can_rotate_end
                            133 
   03A7                     134 _recu_can_0_rotate:
   03A7 E7                  135 	mov A,@R1
   03A8 54 3F               136 	anl A,#0x3F
   03AA FB                  137 	mov R3,A
   03AB E5 C5               138 	mov A,ADCON
   03AD 54 C0               139 	anl A,#0xC0
   03AF 4B                  140 	orl A,R3
   03B0 F7                  141 	mov @R1,A        
   03B1                     142 _recu_can_rotate_end:
   03B1 75 31 01            143 	mov CAN_PRET,#0x01
   03B4 D0 E0               144         pop  ACC                    ;
   03B6 D0 D0               145         pop  PSW                    ; r√©active la banque registres 0  
   03B8 22                  146         ret
                            147 ;-----------------------------------------------------------------------------------------------------------
