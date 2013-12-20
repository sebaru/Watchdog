;-----------------------------------------------------------------------------------------------------------
; rs232.asm    Envoi et reception des trames rs232/485
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
        .module  Rs232
	.include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; _recu_rs232: appelé par interruption
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_recu_rs232:
        push PSW                ; ranger le registre
        push ACC
	mov A,TRAME_RECUE       ; trame precedente traitée ??
	jnz  _recu_rs232_fin    ; si non, fin
        mov  TH0,#0x00
        mov  TL0,#0x01                  ; Reset Timer 0 Watchdog maitre
        mov  PSW,#BANK3          ; activer la banque de registres 3
        clr  RI
        mov  @R0,SBUF               ; si RI=1 placer le caractÃ©re
        inc  R0                     ; mise à jour du pointeur
        inc  R2
        mov  A,#RX_BUF              ; debut du tampon
	add  A,#IndexTrame_taille   ; zone TAILLE
	mov  R1,A                   ; copie dans le registre
	mov  A,@R1                  ; recupération de la taille à lire
	cjne A,#TAILLE_BUF,_recu_rs232_diff
_recu_rs232_continue:               ; on arrive la si taille <= TAILLE_BUF
        add  A,#TAILLE_ENTETE       ; additionne la taille entete
        clr  C
        subb A,R2
        jnz  _recu_rs232_fin        ; compare: si egalité, alors on a tout lu
        mov  TRAME_RECUE, #0x01     ; on valide la trame
_recu_rs232_reset_trame:        
        mov R0,#RX_BUF              ; on reset les registres pour la prochaine trame
        mov R2,#0x0                 ; Nombre de caracteres Ã  recevoir
_recu_rs232_fin:
        pop  ACC                    ;
        pop  PSW                    ; réactive la banque registres 0  
        ret
_recu_rs232_diff:
        jc   _recu_rs232_continue   ; si taille <= TAILLE_BUF, on continue
        sjmp _recu_rs232_reset_trame; sinon, taille > TAILLE_BUF -> drop trame
;-----------------------------------------------------------------------------------------------------------
; Envoi de la trame TXBUF sur la liaison RS232
;-----------------------------------------------------------------------------------------------------------
_envoyer_rs232:
        lcall _del_trame_on             ; prend la main sur la ligne RS485
	mov R0,#TX_BUF              ; on pointe au debut de la trame
	lcall _calcul_crc16         ; on calcul le CRC16
	mov A,R4
	mov @R0,A                   ; recopie poids fort CRC16
	inc R0
	mov A,R5
	mov @R0,A                   ; recopie poids faible CRC16

	mov  A,#TX_BUF
	add  A,#IndexTrame_taille
	mov  R1,A
	mov  A,@R1                  ; recuperation de la taille à envoyer
	add  A,#TAILLE_ENTETE
	mov  R2,A                   ; R3 = nombre total d'octets a envoyer
	mov R0,#TX_BUF              ; debut du buffer
_envoi_encore:
	jnb  TI,_envoi_encore       ; attente que la ligne se libere
	clr  TI                     ; oki, on valide l'envoi
        mov  SBUF,@R0               ; on balance
	inc  R0                     ; caractere suivant
	djnz R2,_envoi_encore       ; saut si pas fini
_dernier_bit:
	jnb  TI,_dernier_bit        ; attente que le dernier bit soit envoyé
	lcall _del_trame_off
	ret
;-----------------------------------------------------------------------------------------------------------
; Purge de la trame de reception
;-----------------------------------------------------------------------------------------------------------
_rs232_purge_trame:
        push PSW                    ; ranger le registre
        push ACC
        mov  PSW,#BANK3             ; activer la banque de registres 3
	mov A,#TAILLE_ENTETE
	add A,#TAILLE_BUF
	mov R2,A
_rs232_purge_trame_encore:
        mov @R0,#0x00
        inc R0
        djnz R2, _rs232_purge_trame_encore	
        mov  R0,#RX_BUF             ; on reset les registres pour la prochaine trame
        mov  R2,#0x0                ; Nombre de caracteres à recevoir
        pop  ACC
        pop  PSW                    ; réactive la banque registres 0  
        ret
;-----------------------------------------------------------------------------------------------------------

