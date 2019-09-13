[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

START:
    mov ax, 0x07C0
    mov ds, ax
    mov ax, 0xB800
    mov es, ax

    mov si,    0
    
.SCREENCLEARLOOP:
    mov byte [ es: si ], 0
    mov byte [ es: si + 1 ], 0x0A

    add si, 2
    cmp si, 80 * 25 * 2

    jl .SCREENCLEARLOOP

    mov si, 0
    mov di, 0
	mov bx, 80 * 2				; for nextline
	call .GETRTC


.MESSAGELOOP:
    mov cl, byte [ si + MESSAGE1 ]
    cmp cl, 0
    ;je .MESSAGEEND				  unused
    je .MESSAGELINE

    mov byte [ es: di ], cl
    add si, 1
    add di, 2
	sub bx, 2					; for nexline

    jmp .MESSAGELOOP

.MESSAGELINE:
	mov si, 0					; Initialize si register

	jmp .MESSAGELOOP2

.MESSAGELOOP2:
	mov cl, byte [ si + CLOCK_STRING ]	
	cmp cl, 0

	je .MESSAGEEND

	mov byte [ es: di + bx], cl	; print to nextline(bx register knows the new line space!)
	add si, 1
	add di, 2

	jmp .MESSAGELOOP2

.MESSAGEEND:
    jmp $

.BCDTOHEX:						; arg: ax, result: bx
	push cx
	mov cl, al

	shr al, 4					; 10's place
	mov bh, al
	
	mov al, cl
	and al, 0x0F				; 1's place
	mov bl, al

	pop cx
	ret

.BCDTOASCII:
	call .BCDTOHEX
	add bh, 48
	add bl, 48
	ret

.HEXCONVERT:
	call .BCDTOHEX
	push cx
	mov al, bh
	mov cl, 10
	mul cl
	add bl, al
	pop cx
	ret

.GETRTC:
	pusha

	mov ah, 4h					; Select 'Read RTC Calendar'
	int 1Ah						; RTC sevices interrupt

	mov word[RTC], cx

	mov al, ch
	call .BCDTOASCII
	mov [CLOCK_STRING+20], bh	; Saving to string
	mov [CLOCK_STRING+21], bl

	mov al, cl					; Get year
	call .BCDTOASCII
	mov [CLOCK_STRING+22], bh
	mov [CLOCK_STRING+23], bl

	mov al, dh					; Get month
	call .BCDTOASCII
	mov [CLOCK_STRING+17], bh
	mov [CLOCK_STRING+18], bl

	mov al, dl					; Get date
	call .BCDTOASCII
	mov [CLOCK_STRING+14], bh
	mov [CLOCK_STRING+15], bl

	call .GETYOIL

	popa
	ret

.GETYOIL:
	mov cx, word[RTC]			; cx <--- [ YY | YY ]
	mov word[RTC], dx
	call .CALCYYYY
	mov ax, cx

	mov dx, word[RTC]			; dx <--- [ MM | DD ]
	push ax
	;call .CALCMM
	pop ax
	
	;add ax, result		mov bx, 7		div bx		dx = index

	pop dx
	pop cx
	ret

.CALCYYYY:
	mov al, ch					; century
	call .HEXCONVERT
	
	sub bl, 19					; century(year) - 19(00)

	mov ax, bx					; result 100's place
	mov bx, 100
	mul bx
	mov bx, ax					; 20()() - 19()() = 1()() => bx
	
	mov al, cl
	mov cx, bx					; result move to cx
	call .HEXCONVERT

	sub cx, 00					; (century)year - (19)00

	mov word[YEARGAP], cx

	call .YEARYOON

	ret

.YEARYOON:
	mov ax, cx
	mov bx, 4
	div bx						; ex) 119 / 4 = ax ... dx
	mov cx, ax					; ex) cx = 29

	mov ax, word[YEARGAP]
	mov bx, 100
	div bx
	sub cx, ax					; ex) 29 - floor(119 / 100) = 28(bx)

	mov dx, 0					; Sanitize dx
	mov bx, 4
	div bx						; ex) floor(119 mod(400) / 100)

	cmp ax, 1
	jne .YOIL1_PASS1
	
	push ax						; ex) ax = floor(119 mod(400)) / 100)
	mov ax, dx
	mov dx, 0					; Sanitize dx
	mov bx, 4					; ex) 19 / 4
	div bx
	pop ax

	cmp dx, 0
	jne .YOIL1_PASS1
	mov byte[YOONFLAG], 1		; save isYoon 1

.YOIL1_PASS1:
	add cx, ax					; ex) 28(bx) + floor(119 mod(400)) / 100) = 29(bx)
	mov ax, cx
	mov dx, 0					; Sanitize dx
	mov bx, 7
	cmp ax, 0
	je .PASS
	div bx
	mov cx, dx					; ex) cx = 29 mod 7

.PASS:
	mov ax, word[YEARGAP]
	mov dx, 0					; Sanitize dx
	mov bx, 7
	div bx						; ex) 90 mod(7) * 365 mod(7) = 6 * 1(const)
	add cx, dx					; cx = result

	ret

.CALCMM:
	mov al, dh					; month(bcd)
	mov ch, 1					; FEB flag set
	call .HEXCONVERT
	mov cx, 0
	push dx
	mov dx, 0

.MONTHLOOP:
	sub bl, 1					; month--
	cmp bl, 0
	je .CALCDD					; if(month == 0) jmp .CALCDD
	
	add cl, 1
	and cl, 0x01
	cmp cl, 1
	je .MONTH31

	cmp ch, 1
	jne .MONTH30
	mov ch, 0

	cmp byte[YOONFLAG], 1					; isYoon compare to 1
	je .IFYOON
	
	add dx, 28
	jmp .MONTHLOOP

.IFYOON:
	add dx, 29
	jmp .MONTHLOOP

.MONTH31:
	add dx, 31
	jmp .MONTHLOOP

.MONTH30:
	add dx, 30
	jmp .MONTHLOOP

.CALCDD:
	pop ax						; pop dx, mov ax, dx(date(bcd))
	call .HEXCONVERT

	mov ax, dx				; month(each date) + date
	add al, bl
	mov cx, 7
	div cx						; dx is index of yoil

	mov ax, dx
	mov dx, 0
	mov cx, 3
	mul cx

	ret
	

MESSAGE1:		db 'MINT64 OS Boot Loader Start~!!', 0
CLOCK_STRING:	db 'Current Data: 00/00/0000 FFF', 0
;YOIL:			db 'MONTUEWEDTHUFRISATSUN', 0

RTC: dw 0
YEARGAP:		dw 0
YOONFLAG:		db 0

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
