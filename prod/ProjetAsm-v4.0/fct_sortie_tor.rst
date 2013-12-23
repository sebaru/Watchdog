ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 1.



                              1 ;-----------------------------------------------------------------------------------------------------------
                              2 ; fct_sortie_tor.asm    Gestion des sorties tor
                              3 ; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
                              4 ;-----------------------------------------------------------------------------------------------------------
                              5         .module FctSortieTOR
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
                              8 ; _fct_sortie_tor: Reception des sorties TOR
                              9 ;-----------------------------------------------------------------------------------------------------------
                             10 	.area CSEG (CODE)
   0264                      11 _fct_sortie_tor:
   0264 74 00                12 	mov A,#NBR_SORTIE_TOR       ; R3 = cpt de sortie TOR
                             13 	;mov R3,A
   0266 70 01                14 	jnz _fct_sortie_tor_suite   ; Si pas de sortie, on sort direct.
   0268 22                   15 	ret
   0269                      16 _fct_sortie_tor_suite:
   0269 74 80                17 	mov A,#RX_BUF
   026B 24 04                18 	add A,#0x04
   026D F8                   19 	mov R0,A                    ; R0 pointe sur le premier champ de donnees.
                             20 
   026E 7F 80                21 	mov R7,#0x80                ; initialisation du poids
   0270 7D 00                22 	mov R5,#0x00                ; Nombre d'octet donnees dans la trame
                             23 
                    0014     24 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 8
                             25 	.else
                             26         ljmp _fct_sortie_tor_8
                             27         .endif
                    0013     28 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 9
                             29 	.else
                             30         ljmp _fct_sortie_tor_9
                             31         .endif
                    0012     32 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 10
                             33 	.else
                             34         ljmp _fct_sortie_tor_10
                             35         .endif
                    0011     36 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 11
                             37 	.else
                             38         ljmp _fct_sortie_tor_11
                             39         .endif
                    0010     40 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 12
                             41 	.else
                             42         ljmp _fct_sortie_tor_12
                             43         .endif
                    000F     44 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 13
                             45  	.else
                             46         ljmp _fct_sortie_tor_13
                             47         .endif
                    000E     48 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 14
                             49 	.else
                             50         ljmp _fct_sortie_tor_14
                             51         .endif
                    000D     52 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 15
                             53 	.else
                             54         ljmp _fct_sortie_tor_15
                             55         .endif
                    000C     56 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 16
                             57 	.else
                             58         ljmp _fct_sortie_tor_16
                             59         .endif
                    000B     60 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 17
                             61 	.else
                             62         ljmp _fct_sortie_tor_17
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 7.



                             63         .endif
                    000A     64 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 18
                             65 	.else
                             66         ljmp _fct_sortie_tor_18
                             67         .endif
                    0009     68 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 19
                             69 	.else
                             70         ljmp _fct_sortie_tor_19
                             71         .endif
                    0008     72 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 20
                             73 	.else
                             74         ljmp _fct_sortie_tor_20
                             75         .endif
                    0007     76 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 21
                             77 	.else
                             78         ljmp _fct_sortie_tor_21
                             79         .endif
                    0006     80 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 22
                             81 	.else
                             82         ljmp _fct_sortie_tor_22
                             83         .endif
                    0005     84 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 23
                             85 	.else
                             86         ljmp _fct_sortie_tor_23
                             87         .endif
                    0004     88 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 24
                             89 	.else
                             90         ljmp _fct_sortie_tor_24
                             91         .endif
                    0003     92 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 25
                             93 	.else
                             94         ljmp _fct_sortie_tor_25
                             95         .endif
                    0002     96 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 26
                             97 	.else
                             98         ljmp _fct_sortie_tor_26
                             99         .endif
                    0001    100 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 27
                            101 	.else
                            102         ljmp _fct_sortie_tor_27
                            103         .endif
   0272 22                  104 	ret
                            105 
   0273                     106 _fct_sortie_tor_8:
   0273 51 C7               107 	acall _fct_sortie_tor_pop_bit
   0275 92 90               108 	mov P1.0,C
                    0013    109 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 9
                            110 	.else
                            111         ljmp _fct_sortie_tor_fin
                            112         .endif
                            113 	
   0277                     114 _fct_sortie_tor_9:
   0277 51 C7               115 	acall _fct_sortie_tor_pop_bit
   0279 92 91               116 	mov P1.1,C
                    0012    117 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 10
                            118 	.else
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 8.



                            119         ljmp _fct_sortie_tor_fin
                            120         .endif
                            121 	
   027B                     122 _fct_sortie_tor_10:
   027B 51 C7               123 	acall _fct_sortie_tor_pop_bit
   027D 92 92               124 	mov P1.2,C
                    0011    125 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 11
                            126 	.else
                            127         ljmp _fct_sortie_tor_fin
                            128         .endif
                            129 
   027F                     130 _fct_sortie_tor_11:
   027F 51 C7               131 	acall _fct_sortie_tor_pop_bit
   0281 92 93               132 	mov P1.3,C
                    0010    133 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 12
                            134 	.else
                            135         ljmp _fct_sortie_tor_fin
                            136         .endif
                            137 	
   0283                     138 _fct_sortie_tor_12:
   0283 51 C7               139 	acall _fct_sortie_tor_pop_bit
   0285 92 94               140 	mov P1.4,C
                    000F    141 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 13
                            142 	.else
                            143         ljmp _fct_sortie_tor_fin
                            144         .endif
                            145 	
   0287                     146 _fct_sortie_tor_13:
   0287 51 C7               147 	acall _fct_sortie_tor_pop_bit
   0289 92 95               148 	mov P1.5,C
                    000E    149 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 14
                            150 	.else
                            151         ljmp _fct_sortie_tor_fin
                            152         .endif
                            153 	
   028B                     154 _fct_sortie_tor_14:
   028B 51 C7               155 	acall _fct_sortie_tor_pop_bit
   028D 92 96               156 	mov P1.6,C
                    000D    157 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 15
                            158 	.else
                            159         ljmp _fct_sortie_tor_fin
                            160         .endif
                            161 	
   028F                     162 _fct_sortie_tor_15:
   028F 51 C7               163 	acall _fct_sortie_tor_pop_bit
   0291 92 97               164 	mov P1.7,C
                    000C    165 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 16
                            166 	.else
                            167         ljmp _fct_sortie_tor_fin
                            168         .endif
                            169 	
   0293                     170 _fct_sortie_tor_16:
   0293 51 C7               171 	acall _fct_sortie_tor_pop_bit
   0295 92 C0               172 	mov P4.0,C
                    000B    173 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 17
                            174 	.else
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 9.



                            175         ljmp _fct_sortie_tor_fin
                            176         .endif
                            177 	
   0297                     178 _fct_sortie_tor_17:
   0297 51 C7               179 	acall _fct_sortie_tor_pop_bit
   0299 92 C1               180 	mov P4.1,C
                    000A    181 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 18
                            182 	.else
                            183         ljmp _fct_sortie_tor_fin
                            184         .endif
                            185 	
   029B                     186 _fct_sortie_tor_18:
   029B 51 C7               187 	acall _fct_sortie_tor_pop_bit
   029D 92 C2               188 	mov P4.2,C
                    0009    189 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 19
                            190 	.else
                            191         ljmp _fct_sortie_tor_fin
                            192         .endif
                            193 	
   029F                     194 _fct_sortie_tor_19:
   029F 51 C7               195 	acall _fct_sortie_tor_pop_bit
   02A1 92 C3               196 	mov P4.3,C
                    0008    197 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 20
                            198 	.else
                            199         ljmp _fct_sortie_tor_fin
                            200         .endif
                            201 	
   02A3                     202 _fct_sortie_tor_20:
   02A3 51 C7               203 	acall _fct_sortie_tor_pop_bit
   02A5 92 C4               204 	mov P4.4,C
                    0007    205 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 21
                            206 	.else
                            207         ljmp _fct_sortie_tor_fin
                            208         .endif
                            209 	
   02A7                     210 _fct_sortie_tor_21:
   02A7 51 C7               211 	acall _fct_sortie_tor_pop_bit
   02A9 92 C5               212 	mov P4.5,C
                    0006    213 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 22
                            214 	.else
                            215         ljmp _fct_sortie_tor_fin
                            216         .endif
                            217 	
   02AB                     218 _fct_sortie_tor_22:
   02AB 51 C7               219 	acall _fct_sortie_tor_pop_bit
   02AD 92 C6               220 	mov P4.6,C
                    0005    221 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 23
                            222 	.else
                            223         ljmp _fct_sortie_tor_fin
                            224         .endif
                            225 	
   02AF                     226 _fct_sortie_tor_23:
   02AF 51 C7               227 	acall _fct_sortie_tor_pop_bit
   02B1 92 C7               228 	mov P4.7,C
                    0004    229 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 24
                            230 	.else
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 10.



                            231         ljmp _fct_sortie_tor_fin
                            232         .endif
                            233 	
   02B3                     234 _fct_sortie_tor_24:
   02B3 51 C7               235 	acall _fct_sortie_tor_pop_bit
   02B5 92 B2               236 	mov P3.2,C
                    0003    237 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 25
                            238 	.else
                            239         ljmp _fct_sortie_tor_fin
                            240         .endif
                            241 
   02B7                     242 _fct_sortie_tor_25:
   02B7 51 C7               243 	acall _fct_sortie_tor_pop_bit
   02B9 92 B3               244 	mov P3.3,C
                    0002    245 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 26
                            246 	.else
                            247         ljmp _fct_sortie_tor_fin
                            248         .endif
                            249 
   02BB                     250 _fct_sortie_tor_26:
   02BB 51 C7               251 	acall _fct_sortie_tor_pop_bit
   02BD 92 B4               252 	mov P3.4,C
                    0001    253 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 27
                            254 	.else
                            255         ljmp _fct_sortie_tor_fin
                            256         .endif
                            257 
   02BF                     258 _fct_sortie_tor_27:
   02BF 51 C7               259 	acall _fct_sortie_tor_pop_bit
   02C1 92 B5               260 	mov P3.5,C
                    0000    261 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 28
                            262 	.else
   02C3 02 02 C6            263         ljmp _fct_sortie_tor_fin
                            264         .endif
                            265 
   02C6                     266 _fct_sortie_tor_fin:
   02C6 22                  267 	ret
                            268 
                            269 ;-----------------------------------------------------------------------------------------------------------
                            270 ; _fct_sortie_tor_pop_bit: pop d'un bit dans la zone donnees de la trame
                            271 ;-----------------------------------------------------------------------------------------------------------
   02C7                     272 _fct_sortie_tor_pop_bit:            ; push un bit (C) dans la trame donnees.
   02C7 E6                  273 	mov A,@R0                     ; recupération octet donnes trame
   02C8 5F                  274 	anl A,R7                      ; mise à un bit de poids R7
   02C9 FA                  275 	mov R2,A                      ; sauvegarde.
                            276 
   02CA EF                  277 	mov A,R7                      ; rotate du poids.
   02CB C3                  278 	clr C                         ; rotate du poids: >> 1
   02CC 13                  279 	rrc A
   02CD FF                  280 	mov R7,A                      ; decalage pour prochaine mise à un 
   02CE 70 03               281 	jnz _fct_sortie_tor_pop_bit_fin ; si pas de depassement, fin
   02D0 08                  282 	inc R0                        ; sinon octet trame suivant, reset R7
   02D1 7F 80               283 	mov R7,#0x80
   02D3                     284 _fct_sortie_tor_pop_bit_fin:
                            285 
   02D3 EA                  286 	mov A,R2
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 11.



   02D4 60 03               287 	jz _fct_sortie_tor_pop_bit_0  ; demande de mise à zero ?
   02D6 C3                  288 	clr C                         ; MAU sortie -> MAZ patte controleur
   02D7 41 DA               289 	ajmp _fct_sortie_tor_pop_bit_suite
   02D9                     290 _fct_sortie_tor_pop_bit_0:
   02D9 D3                  291 	setb C                        ; MAZ sortie -> MAU patte controleur             
   02DA                     292 _fct_sortie_tor_pop_bit_suite:
   02DA 22                  293 	ret
                            294 	
                            295 ;---------------------------------------------------------------------------------------
                            296 ; _raz_sortie_tor
                            297 ;---------------------------------------------------------------------------------------
                            298 
   02DB                     299  _raz_sortie_tor:
   02DB 74 00               300  	mov A,#NBR_SORTIE_TOR
                            301 	;mov R3,A
   02DD 70 01               302 	jnz _raz_sortie_tor_suite
   02DF 22                  303 	ret
   02E0                     304  _raz_sortie_tor_suite:
   02E0 D3                  305         setb C
                    0014    306 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 8
                            307 	.else
                            308         ljmp _raz_sortie_tor_8
                            309         .endif
                    0013    310 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 9
                            311 	.else
                            312         ljmp _raz_sortie_tor_9
                            313         .endif
                    0012    314 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 10
                            315 	.else
                            316         ljmp _raz_sortie_tor_10
                            317         .endif
                    0011    318 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 11
                            319 	.else
                            320         ljmp _raz_sortie_tor_11
                            321         .endif
                    0010    322 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 12
                            323 	.else
                            324         ljmp _raz_sortie_tor_12
                            325         .endif
                    000F    326 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 13
                            327  	.else
                            328         ljmp _raz_sortie_tor_13
                            329         .endif
                    000E    330 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 14
                            331 	.else
                            332         ljmp _raz_sortie_tor_14
                            333         .endif
                    000D    334 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 15
                            335 	.else
                            336         ljmp _raz_sortie_tor_15
                            337         .endif
                    000C    338 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 16
                            339 	.else
                            340         ljmp _raz_sortie_tor_16
                            341         .endif
                    000B    342 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 17
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 12.



                            343 	.else
                            344         ljmp _raz_sortie_tor_17
                            345         .endif
                    000A    346 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 18
                            347 	.else
                            348         ljmp _raz_sortie_tor_18
                            349         .endif
                    0009    350 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 19
                            351 	.else
                            352         ljmp _raz_sortie_tor_19
                            353         .endif
                    0008    354 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 20
                            355 	.else
                            356         ljmp _raz_sortie_tor_20
                            357         .endif
                    0007    358 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 21
                            359 	.else
                            360         ljmp _raz_sortie_tor_21
                            361         .endif
                    0006    362 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 22
                            363 	.else
                            364         ljmp _raz_sortie_tor_22
                            365         .endif
                    0005    366 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 23
                            367 	.else
                            368         ljmp _raz_sortie_tor_23
                            369         .endif
                    0004    370 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 24
                            371 	.else
                            372         ljmp _raz_sortie_tor_24
                            373         .endif
                    0003    374 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 25
                            375 	.else
                            376         ljmp _raz_sortie_tor_25
                            377         .endif
                    0002    378 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 26
                            379 	.else
                            380         ljmp _raz_sortie_tor_26
                            381         .endif
                    0001    382 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC - 27
                            383 	.else
                            384         ljmp _raz_sortie_tor_27
                            385         .endif
                            386 
   02E1                     387 _raz_sortie_tor_8:
   02E1 92 90               388 	mov P1.0,C
                    0013    389 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 9
                            390 	.else
                            391         ljmp _raz_sortie_tor_fin
                            392         .endif
                            393 	
   02E3                     394 _raz_sortie_tor_9:
   02E3 92 91               395 	mov P1.1,C
                    0012    396 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 10
                            397 	.else
                            398         ljmp _raz_sortie_tor_fin
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 13.



                            399         .endif
                            400 	
   02E5                     401 _raz_sortie_tor_10:
   02E5 92 92               402 	mov P1.2,C
                    0011    403 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 11
                            404 	.else
                            405         ljmp _raz_sortie_tor_fin
                            406         .endif
                            407 
   02E7                     408 _raz_sortie_tor_11:
   02E7 92 93               409 	mov P1.3,C
                    0010    410 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 12
                            411 	.else
                            412         ljmp _raz_sortie_tor_fin
                            413         .endif
                            414 	
   02E9                     415 _raz_sortie_tor_12:
   02E9 92 94               416 	mov P1.4,C
                    000F    417 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 13
                            418 	.else
                            419         ljmp _raz_sortie_tor_fin
                            420         .endif
                            421 	
   02EB                     422 _raz_sortie_tor_13:
   02EB 92 95               423 	mov P1.5,C
                    000E    424 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 14
                            425 	.else
                            426         ljmp _raz_sortie_tor_fin
                            427         .endif
                            428 	
   02ED                     429 _raz_sortie_tor_14:
   02ED 92 96               430 	mov P1.6,C
                    000D    431 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 15
                            432 	.else
                            433         ljmp _raz_sortie_tor_fin
                            434         .endif
                            435 	
   02EF                     436 _raz_sortie_tor_15:
   02EF 92 97               437 	mov P1.7,C
                    000C    438 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 16
                            439 	.else
                            440         ljmp _raz_sortie_tor_fin
                            441         .endif
                            442 	
   02F1                     443 _raz_sortie_tor_16:
   02F1 92 C0               444 	mov P4.0,C
                    000B    445 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 17
                            446 	.else
                            447         ljmp _raz_sortie_tor_fin
                            448         .endif
                            449 	
   02F3                     450 _raz_sortie_tor_17:
   02F3 92 C1               451 	mov P4.1,C
                    000A    452 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 18
                            453 	.else
                            454         ljmp _raz_sortie_tor_fin
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 14.



                            455         .endif
                            456 	
   02F5                     457 _raz_sortie_tor_18:
   02F5 92 C2               458 	mov P4.2,C
                    0009    459 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 19
                            460 	.else
                            461         ljmp _raz_sortie_tor_fin
                            462         .endif
                            463 	
   02F7                     464 _raz_sortie_tor_19:
   02F7 92 C3               465 	mov P4.3,C
                    0008    466 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 20
                            467 	.else
                            468         ljmp _raz_sortie_tor_fin
                            469         .endif
                            470 	
   02F9                     471 _raz_sortie_tor_20:
   02F9 92 C4               472 	mov P4.4,C
                    0007    473 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 21
                            474 	.else
                            475         ljmp _raz_sortie_tor_fin
                            476         .endif
                            477 	
   02FB                     478 _raz_sortie_tor_21:
   02FB 92 C5               479 	mov P4.5,C
                    0006    480 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 22
                            481 	.else
                            482         ljmp _raz_sortie_tor_fin
                            483         .endif
                            484 	
   02FD                     485 _raz_sortie_tor_22:
   02FD 92 C6               486 	mov P4.6,C
                    0005    487 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 23
                            488 	.else
                            489         ljmp _raz_sortie_tor_fin
                            490         .endif
                            491 	
   02FF                     492 _raz_sortie_tor_23:
   02FF 92 C7               493 	mov P4.7,C
                    0004    494 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 24
                            495 	.else
                            496         ljmp _raz_sortie_tor_fin
                            497         .endif
                            498 	
   0301                     499 _raz_sortie_tor_24:
   0301 92 B2               500 	mov P3.2,C
                    0003    501 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 25
                            502 	.else
                            503         ljmp _raz_sortie_tor_fin
                            504         .endif
                            505 
   0303                     506 _raz_sortie_tor_25:
   0303 92 B3               507 	mov P3.3,C
                    0002    508 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 26
                            509 	.else
                            510         ljmp _raz_sortie_tor_fin
ASxxxx Assembler V01.70 + NoICE + SDCC mods + Flat24 Feb-1999  (Intel 8051), page 15.



                            511         .endif
                            512 
   0305                     513 _raz_sortie_tor_26:
   0305 92 B4               514 	mov P3.4,C
                    0001    515 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 27
                            516 	.else
                            517         ljmp _raz_sortie_tor_fin
                            518         .endif
                            519 
   0307                     520 _raz_sortie_tor_27:
   0307 92 B5               521 	mov P3.5,C
                    0000    522 	.if #NBR_ENTRE_ANA+#NBR_ENTRE_TOR+#NBR_ENTRE_CHOC+#NBR_SORTIE_TOR - 28
                            523 	.else
   0309 02 03 0C            524         ljmp _raz_sortie_tor_fin
                            525         .endif
                            526 
   030C                     527 _raz_sortie_tor_fin:
   030C 22                  528 	ret
                            529 ;-----------------------------------------------------------------------------------------------------------
                            530 
