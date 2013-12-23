;-----------------------------------------------------------------------------------------------------------
; crc16.asm    Calcul du crc16
; Sébastien Lefevre                                                            mar 08 jun 2004 12:49:36 CEST 
;-----------------------------------------------------------------------------------------------------------
	.module  Crc16
        .include "cst.asm"
;-----------------------------------------------------------------------------------------------------------
; Calcul du CRC16 de la trame adressée par R0
;-----------------------------------------------------------------------------------------------------------
	.area CSEG	(CODE)
_calcul_crc16:
	mov	R4,#0xFF            ; poids fort crc16
	mov	R5,#0xFF            ; poids faible crc16

        mov     A,R0                ; début de trame
	add     A,#IndexTrame_taille; zone TAILLE
	mov     R1,A
	mov     A,@R1               ; recuperation de la taille à processer
	add     A,#TAILLE_ENTETE    ; + taille entete
        clr C                       ; Carry = 0 pour la soustraction
	subb    A,#0x02             ; on ne calcule pas le CRC du CRC !!
	mov     R3,A                ; R3 = compteur d'octet

_boucle_octet:
                               	; debut de boucle
	mov	A,R5		; poids faible crc 16
	xrl	A,@R0		; xor octet en cours de codage
	mov	R5,A

	mov	R2,#0x08        ; boucle bit
_boucle_bit:
	mov	A,R4		; poids fort crc
	clr	C		; Carry = 0
	rrc	A		; rotate right
	mov	R4,A		; save
	mov	A,R5		; au tour du poids faible
	rrc	A		; rotate
	mov	R5,A		; save
	jnc	_boucle_bit_fin

	mov 	A,R4		; Xor si retenue = 1
	xrl	A,#0xA0
	mov	R4,A
	mov	A,R5
	xrl	A,#0x01
	mov	R5,A
_boucle_bit_fin:
        djnz    R2,_boucle_bit

	inc	R0       	; on avance dans la trame
	djnz    R3,_boucle_octet
        ret
;-----------------------------------------------------------------------------------------------------------

