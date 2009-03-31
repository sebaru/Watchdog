ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; fct_entre_tor.asm    Fonction d'envoi des entrees TOR
                              3 ; S�bastien Lefevre                                                            dim 13 jun 2004 17:02:11 CEST
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5         .module FctEntreTOR
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 2.



                              6 	.include "cst.asm"
                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; cst.asm    Definition des constantes prgs
                              3 ; S�bastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
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



                              7 ;----------------------------------------D�clarations des prototypes de fonctions --------------------------
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
                             44 ;---------------------------------------- Constantes communes � tous ---------------------------------------
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



                              7 	.area CSEG	(CODE)
                              8 ;-----------------------------------------------------------------------------------------------------------
                              9 ; _polling_entre_tor: Surveille les etats entre TOR et les sauvegardes le cas echeant
                             10 ;-----------------------------------------------------------------------------------------------------------
   0429                      11 _polling_entre_tor:
   0429 78 BC                12         mov R0,#INPUT_BUF
                             13 	
                    0008     14 	.if #NBR_ENTRE_ANA - 0
                             15 	.else
                             16         ljmp _polling_entre_tor_0
                             17         .endif
                    0007     18 	.if #NBR_ENTRE_ANA - 1
                             19  	.else
                             20         ljmp _polling_entre_tor_1
                             21         .endif
                    0006     22 	.if #NBR_ENTRE_ANA - 2
                             23  	.else
                             24         ljmp _polling_entre_tor_2
                             25         .endif
                    0005     26 	.if #NBR_ENTRE_ANA - 3
                             27 	.else
                             28         ljmp _polling_entre_tor_3
                             29         .endif
                    0004     30 	.if #NBR_ENTRE_ANA - 4
                             31 	.else
                             32         ljmp _polling_entre_tor_4
                             33         .endif
                    0003     34 	.if #NBR_ENTRE_ANA - 5
                             35 	.else
                             36         ljmp _polling_entre_tor_5
                             37         .endif
                    0002     38 	.if #NBR_ENTRE_ANA - 6
                             39 	.else
                             40         ljmp _polling_entre_tor_6
                             41         .endif
                    0001     42 	.if #NBR_ENTRE_ANA - 7
                             43 	.else
                             44         ljmp _polling_entre_tor_7
                             45         .endif
                    0000     46 	.if #NBR_ENTRE_ANA - 8
                             47 	.else
   042B 02 04 7A             48         ljmp _polling_entre_tor_8
                             49         .endif
                    FFFFFFFF     50 	.if #NBR_ENTRE_ANA - 9
                             51 	.else
                             52         ljmp _polling_entre_tor_9
                             53         .endif
                    FFFFFFFE     54 	.if #NBR_ENTRE_ANA - 10
                             55 	.else
                             56         ljmp _polling_entre_tor_10
                             57         .endif
                    FFFFFFFD     58 	.if #NBR_ENTRE_ANA - 11
                             59 	.else
                             60         ljmp _polling_entre_tor_11
                             61         .endif
                    FFFFFFFC     62 	.if #NBR_ENTRE_ANA - 12
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 7.



                             63 	.else
                             64         ljmp _polling_entre_tor_12
                             65         .endif
                    FFFFFFFB     66 	.if #NBR_ENTRE_ANA - 13
                             67  	.else
                             68         ljmp _polling_entre_tor_13
                             69         .endif
                    FFFFFFFA     70 	.if #NBR_ENTRE_ANA - 14
                             71 	.else
                             72         ljmp _polling_entre_tor_14
                             73         .endif
                    FFFFFFF9     74 	.if #NBR_ENTRE_ANA - 15
                             75 	.else
                             76         ljmp _polling_entre_tor_15
                             77         .endif
                    FFFFFFF8     78 	.if #NBR_ENTRE_ANA - 16
                             79 	.else
                             80         ljmp _polling_entre_tor_16
                             81         .endif
                    FFFFFFF7     82 	.if #NBR_ENTRE_ANA - 17
                             83 	.else
                             84         ljmp _polling_entre_tor_17
                             85         .endif
                    FFFFFFF6     86 	.if #NBR_ENTRE_ANA - 18
                             87 	.else
                             88         ljmp _polling_entre_tor_18
                             89         .endif
                    FFFFFFF5     90 	.if #NBR_ENTRE_ANA - 19
                             91 	.else
                             92         ljmp _polling_entre_tor_19
                             93         .endif
                    FFFFFFF4     94 	.if #NBR_ENTRE_ANA - 20
                             95 	.else
                             96         ljmp _polling_entre_tor_20
                             97         .endif
                    FFFFFFF3     98 	.if #NBR_ENTRE_ANA - 21
                             99 	.else
                            100         ljmp _polling_entre_tor_21
                            101         .endif
                    FFFFFFF2    102 	.if #NBR_ENTRE_ANA - 22
                            103 	.else
                            104         ljmp _polling_entre_tor_22
                            105         .endif
                    FFFFFFF1    106 	.if #NBR_ENTRE_ANA - 23
                            107 	.else
                            108         ljmp _polling_entre_tor_23
                            109         .endif
                    FFFFFFF0    110 	.if #NBR_ENTRE_ANA - 24
                            111 	.else
                            112         ljmp _polling_entre_tor_24
                            113         .endif
                    FFFFFFEF    114 	.if #NBR_ENTRE_ANA - 25
                            115 	.else
                            116         ljmp _polling_entre_tor_25
                            117         .endif
                    FFFFFFEE    118 	.if #NBR_ENTRE_ANA - 26
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 8.



                            119 	.else
                            120         ljmp _polling_entre_tor_26
                            121         .endif
                    FFFFFFED    122 	.if #NBR_ENTRE_ANA - 27
                            123 	.else
                            124         ljmp _polling_entre_tor_27
                            125         .endif
                            126 
   042E                     127 _polling_entre_tor_0:
   042E E5 C4               128 	mov A,P5
   0430 54 01               129 	anl A,#0x01
   0432 C3                  130 	clr C
   0433 13                  131 	rrc A
   0434 91 CE               132         acall _polling_entre_tor_test
                    001B    133 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 1
                            134 	.else
                            135         ljmp _polling_entre_tor_fin
                            136         .endif
                            137 	
   0436                     138 _polling_entre_tor_1:
   0436 E5 C4               139 	mov A,P5
   0438 54 02               140 	anl A,#0x02
   043A C3                  141         clr C
   043B 13                  142         rrc A
   043C 13                  143         rrc A
   043D 91 CE               144         acall _polling_entre_tor_test
                    001A    145 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 2
                            146 	.else
                            147         ljmp _polling_entre_tor_fin
                            148         .endif
                            149 	
   043F                     150 _polling_entre_tor_2:
   043F E5 C4               151 	mov A,P5
   0441 54 04               152 	anl A,#0x04
   0443 C3                  153 	clr C
   0444 13                  154 	rrc A
   0445 13                  155 	rrc A
   0446 13                  156 	rrc A
   0447 91 CE               157         acall _polling_entre_tor_test
                    0019    158 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 3
                            159 	.else
                            160         ljmp _polling_entre_tor_fin
                            161         .endif
                            162 
   0449                     163 _polling_entre_tor_3:
   0449 E5 C4               164 	mov A,P5
   044B 54 08               165 	anl A,#0x08
   044D C3                  166 	clr C
   044E 13                  167 	rrc A
   044F 13                  168 	rrc A
   0450 13                  169 	rrc A
   0451 13                  170 	rrc A
   0452 91 CE               171         acall _polling_entre_tor_test
                    0018    172 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 4
                            173 	.else
                            174         ljmp _polling_entre_tor_fin
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 9.



                            175         .endif
                            176 
   0454                     177 _polling_entre_tor_4:
   0454 E5 C4               178 	mov A,P5
   0456 54 10               179 	anl A,#0x10
   0458 C3                  180 	clr C
   0459 33                  181 	rlc A
   045A 33                  182 	rlc A
   045B 33                  183 	rlc A
   045C 33                  184 	rlc A
   045D 91 CE               185         acall _polling_entre_tor_test
                    0017    186 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 5
                            187 	.else
                            188         ljmp _polling_entre_tor_fin
                            189         .endif
                            190 	
   045F                     191 _polling_entre_tor_5:
   045F E5 C4               192 	mov A,P5
   0461 54 20               193 	anl A,#0x20
   0463 C3                  194 	clr C
   0464 33                  195 	rlc A
   0465 33                  196 	rlc A
   0466 33                  197 	rlc A
   0467 91 CE               198         acall _polling_entre_tor_test
                    0016    199 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 6
                            200 	.else
                            201         ljmp _polling_entre_tor_fin
                            202         .endif
                            203 	
   0469                     204 _polling_entre_tor_6:
   0469 E5 C4               205 	mov A,P5
   046B 54 40               206 	anl A,#0x40
   046D C3                  207 	clr C
   046E 33                  208 	rlc A
   046F 33                  209 	rlc A
   0470 91 CE               210         acall _polling_entre_tor_test
                    0015    211 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 7
                            212 	.else
                            213         ljmp _polling_entre_tor_fin
                            214         .endif
                            215 	
   0472                     216 _polling_entre_tor_7:
   0472 E5 C4               217 	mov A,P5
   0474 54 80               218 	anl A,#0x80
   0476 C3                  219 	clr C
   0477 33                  220 	rlc A
   0478 91 CE               221         acall _polling_entre_tor_test
                    0014    222 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 8
                            223 	.else
                            224         ljmp _polling_entre_tor_fin
                            225         .endif
                            226 
   047A                     227 _polling_entre_tor_8:
   047A A2 90               228 	mov C,P1.0
   047C 91 CE               229         acall _polling_entre_tor_test
                    0013    230 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 9
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 10.



                            231 	.else
                            232         ljmp _polling_entre_tor_fin
                            233         .endif
                            234 	
   047E                     235 _polling_entre_tor_9:
   047E A2 91               236 	mov C,P1.1
   0480 91 CE               237         acall _polling_entre_tor_test
                    0012    238 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 10
                            239 	.else
                            240         ljmp _polling_entre_tor_fin
                            241         .endif
                            242 	
   0482                     243 _polling_entre_tor_10:
   0482 A2 92               244 	mov C,P1.2
   0484 91 CE               245         acall _polling_entre_tor_test
                    0011    246 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 11
                            247 	.else
                            248         ljmp _polling_entre_tor_fin
                            249         .endif
                            250 
   0486                     251 _polling_entre_tor_11:
   0486 A2 93               252 	mov C,P1.3
   0488 91 CE               253         acall _polling_entre_tor_test
                    0010    254 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 12
                            255 	.else
                            256         ljmp _polling_entre_tor_fin
                            257         .endif
                            258 	
   048A                     259 _polling_entre_tor_12:
   048A A2 94               260 	mov C,P1.4
   048C 91 CE               261         acall _polling_entre_tor_test
                    000F    262 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 13
                            263 	.else
                            264         ljmp _polling_entre_tor_fin
                            265         .endif
                            266 	
   048E                     267 _polling_entre_tor_13:
   048E A2 95               268 	mov C,P1.5
   0490 91 CE               269         acall _polling_entre_tor_test
                    000E    270 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 14
                            271 	.else
                            272         ljmp _polling_entre_tor_fin
                            273         .endif
                            274 	
   0492                     275 _polling_entre_tor_14:
   0492 A2 96               276 	mov C,P1.6
   0494 91 CE               277         acall _polling_entre_tor_test
                    000D    278 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 15
                            279 	.else
                            280         ljmp _polling_entre_tor_fin
                            281         .endif
                            282 	
   0496                     283 _polling_entre_tor_15:
   0496 A2 97               284 	mov C,P1.7
   0498 91 CE               285         acall _polling_entre_tor_test
                    000C    286 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 16
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 11.



                            287 	.else
                            288         ljmp _polling_entre_tor_fin
                            289         .endif
                            290 	
   049A                     291 _polling_entre_tor_16:
   049A A2 C0               292 	mov C,P4.0
   049C 91 CE               293         acall _polling_entre_tor_test
                    000B    294 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 17
                            295 	.else
                            296         ljmp _polling_entre_tor_fin
                            297         .endif
                            298 	
   049E                     299 _polling_entre_tor_17:
   049E A2 C1               300 	mov C,P4.1
   04A0 91 CE               301         acall _polling_entre_tor_test
                    000A    302 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 18
                            303 	.else
                            304         ljmp _polling_entre_tor_fin
                            305         .endif
                            306 	
   04A2                     307 _polling_entre_tor_18:
   04A2 A2 C2               308 	mov C,P4.2
   04A4 91 CE               309         acall _polling_entre_tor_test
                    0009    310 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 19
                            311 	.else
                            312         ljmp _polling_entre_tor_fin
                            313         .endif
                            314 	
   04A6                     315 _polling_entre_tor_19:
   04A6 A2 C3               316 	mov C,P4.3
   04A8 91 CE               317         acall _polling_entre_tor_test
                    0008    318 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 20
                            319 	.else
                            320         ljmp _polling_entre_tor_fin
                            321         .endif
                            322 	
   04AA                     323 _polling_entre_tor_20:
   04AA A2 C4               324 	mov C,P4.4
   04AC 91 CE               325         acall _polling_entre_tor_test
                    0007    326 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 21
                            327 	.else
                            328         ljmp _polling_entre_tor_fin
                            329         .endif
                            330 	
   04AE                     331 _polling_entre_tor_21:
   04AE A2 C5               332 	mov C,P4.5
   04B0 91 CE               333         acall _polling_entre_tor_test
                    0006    334 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 22
                            335 	.else
                            336         ljmp _polling_entre_tor_fin
                            337         .endif
                            338 	
   04B2                     339 _polling_entre_tor_22:
   04B2 A2 C6               340 	mov C,P4.6
   04B4 91 CE               341         acall _polling_entre_tor_test
                    0005    342 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 23
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 12.



                            343 	.else
                            344         ljmp _polling_entre_tor_fin
                            345         .endif
                            346 	
   04B6                     347 _polling_entre_tor_23:
   04B6 A2 C7               348 	mov C,P4.7
   04B8 91 CE               349         acall _polling_entre_tor_test
                    0004    350 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 24
                            351 	.else
                            352         ljmp _polling_entre_tor_fin
                            353         .endif
                            354 	
   04BA                     355 _polling_entre_tor_24:
   04BA A2 B2               356 	mov C,P3.2
   04BC 91 CE               357         acall _polling_entre_tor_test
                    0003    358 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 25
                            359 	.else
                            360         ljmp _polling_entre_tor_fin
                            361         .endif
                            362 
   04BE                     363 _polling_entre_tor_25:
   04BE A2 B3               364 	mov C,P3.3
   04C0 91 CE               365         acall _polling_entre_tor_test
                    0002    366 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 26
                            367 	.else
                            368         ljmp _polling_entre_tor_fin
                            369         .endif
                            370 
   04C2                     371 _polling_entre_tor_26:
   04C2 A2 B4               372 	mov C,P3.4
   04C4 91 CE               373         acall _polling_entre_tor_test
                    0001    374 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 27
                            375 	.else
                            376         ljmp _polling_entre_tor_fin
                            377         .endif
                            378 
   04C6                     379 _polling_entre_tor_27:
   04C6 A2 B5               380 	mov C,P3.5
   04C8 91 CE               381         acall _polling_entre_tor_test
                    0000    382 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR - 28
                            383 	.else
   04CA 02 04 CD            384         ljmp _polling_entre_tor_fin
                            385         .endif
                            386 
   04CD                     387 _polling_entre_tor_fin:
   04CD 22                  388         ret
                            389 ;-----------------------------------------------------------------------------------------------------------
                            390 ; _polling_entre_tor_test: Compare la valeur reelle d'un port avec celle sauvegard�e et met � jour le cas
                            391 ; �ch�ant                                                                                                   
                            392 ;-----------------------------------------------------------------------------------------------------------
   04CE                     393 _polling_entre_tor_test:
   04CE E6                  394         mov A,@R0
   04CF F9                  395         mov R1,A                             ; Sauvegarde
   04D0 54 02               396 	anl A,#0x02                          ; Bit 1: Lock� par le PC ou non ??
                            397 	;;; desactiv� pb fenetre cuisine 10/07/05 
                            398         ;;; jnz _polling_entre_tor_test_fin      ; Lock� ? -> Pas touche !
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 13.



                            399 
   04D2 74 00               400         mov A,#0x00                          ; C possede la valeur reelle du port � tester
   04D4 33                  401 	rlc A                                ; A recupere la valeur de C
   04D5 FA                  402         mov R2,A
                            403 
   04D6 E9                  404         mov A,R1                             ; recup de l'etat n-1
   04D7 54 01               405 	anl A,#0x01
   04D9 9A                  406 	subb A,R2
   04DA 60 04               407 	jz _polling_entre_tor_test_fin       ; rien n'a chang�, on se barre
                            408 	
   04DC EA                  409         mov A,R2
   04DD 44 02               410         orl A,#0x02
   04DF F6                  411         mov @R0,A
                            412 
   04E0                     413 _polling_entre_tor_test_fin:
   04E0 08                  414         inc R0
   04E1 22                  415         ret
                            416 
                            417 
                            418 ;-----------------------------------------------------------------------------------------------------------
                            419 ; _fct_entre_tor: Envoi de l'etat des entr�es TOR
                            420 ;-----------------------------------------------------------------------------------------------------------
   04E2                     421 _fct_entre_tor:
   04E2 78 9E               422 	mov R0,#TX_BUF
   04E4 76 FF               423 	mov @R0, #ID_MAITRE           ; envoie au maitre
   04E6 08                  424 	inc R0
   04E7 76 01               425 	mov @R0, #ID_ESCLAVE          ; source
   04E9 08                  426 	inc R0
   04EA 76 02               427 	mov @R0, #FCT_ENTRE_TOR       ; fonction
   04EC 08                  428 	inc R0
   04ED E8                  429         mov A,R0
   04EE FA                  430         mov R2,A                      ; R2 pointe sur le champ taille de trame
   04EF 08                  431 	inc R0                        ; R0 pointe sur le debut donnees trame.
   04F0 76 00               432 	mov @R0, #0x00                ; mise � zero
   04F2 74 14               433 	mov A,#NBR_ENTRE_TOR          ; R3 = cpt d'entree TOR
   04F4 FB                  434 	mov R3,A
   04F5 70 08               435 	jnz _fct_entre_tor_suite      ;
                            436 
   04F7 EA                  437         mov A,R2
   04F8 F9                  438         mov R1,A
   04F9 77 00               439         mov @R1,#0x00                 ; zero entre TOR
                            440 
   04FB 12 01 A5            441 	lcall _envoyer_rs232          ; on envoie la trame
   04FE 22                  442 	ret
   04FF                     443 _fct_entre_tor_suite:
                            444 
   04FF 7F 80               445 	mov R7,#0x80                  ; initialisation du poids
   0501 7D 00               446 	mov R5,#0x00                  ; Nombre d'octet donnees dans la trame
                            447 	
   0503 79 BC               448         mov R1,#INPUT_BUF
   0505                     449 _fct_entre_tor_suite2:                ; R3 possede le nombre d'entree TOR
   0505 E7                  450         mov A,@R1
   0506 54 FD               451         anl A,#0xFD                   ; mise � zero bit 1
   0508 F7                  452         mov @R1,A
   0509 54 01               453         anl A,#0x01                   ; on recupere le bit 0
   050B 13                  454         rrc A                         ; on le recupere dans le carry
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 14.



   050C B1 21               455 	acall _fct_entre_tor_push_bit
   050E 09                  456         inc R1
   050F DB F4               457         djnz R3,_fct_entre_tor_suite2
                            458 
   0511                     459 _fct_entre_tor_fin:
   0511 BF 80 03            460 	cjne R7,#0x80,_fct_entre_tor_octet_partiel
   0514 02 05 19            461 	ljmp _fct_entre_tor_taille	
   0517                     462 _fct_entre_tor_octet_partiel:
   0517 0D                  463 	inc R5                        ; l'octet est partiellement utilis�, il faut quand mem le compter
   0518 08                  464 	inc R0                        ; le crc est donc � l'octet suivant
   0519                     465 _fct_entre_tor_taille:
   0519 EA                  466         mov A,R2                      ; l'adresse du champs taille etait sauv�e dans R2
   051A F9                  467         mov R1,A
   051B ED                  468 	mov A,R5
   051C F7                  469 	mov @R1,A                     ; On precise la taille de la trame.
                            470 
   051D 12 01 A5            471 	lcall _envoyer_rs232          ; on envoie la trame
   0520 22                  472 	ret
                            473 
                            474 ;-----------------------------------------------------------------------------------------------------------
                            475 ; _fct_entre_tor_push_bit: push d'un bit dans la zone donnees de la trame
                            476 ;-----------------------------------------------------------------------------------------------------------
   0521                     477 _fct_entre_tor_push_bit:            ; push un bit (C) dans la trame donnees.
   0521 40 08               478 	jc  _fct_entre_tor_push_1
   0523 EF                  479 	mov A,R7                     ; R7 = poids du bit en cours
   0524 F4                  480 	cpl A
   0525 FE                  481 	mov R6,A                      ; R6 = compl�ment de R7
                            482 
   0526 E6                  483 	mov A,@R0                     ; recup�ration octet donnes trame
   0527 5E                  484 	anl A,R6                      ; mise � zero du bit de poids R7
   0528 F6                  485 	mov @R0,A
   0529 80 03               486 	sjmp _fct_entre_tor_push_bit_suite
   052B                     487 _fct_entre_tor_push_1:
   052B E6                  488 	mov A,@R0                     ; recup�ration octet donnes trame
   052C 4F                  489 	orl A,R7                      ; mise � un bit de poids R7
   052D F6                  490 	mov @R0,A
   052E                     491 _fct_entre_tor_push_bit_suite:
   052E EF                  492 	mov A,R7
   052F C3                  493 	clr C                         ; rotate du poids: << 1
   0530 13                  494 	rrc A
   0531 FF                  495 	mov R7,A                      ; decalage pour prochaine mise � un 
   0532 70 06               496 	jnz _fct_entre_tor_push_bit_fin ; si pas de depassement, fin
   0534 08                  497 	inc R0                        ; sinon octet trame suivant, reset R7
   0535 76 00               498 	mov @R0, #0x00                ; mise � zero
   0537 0D                  499 	inc R5                        ; un octet de plus consomm�.
   0538 7F 80               500 	mov R7,#0x80
   053A                     501 _fct_entre_tor_push_bit_fin:
   053A 22                  502 	ret
