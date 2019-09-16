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
	
	call .GETRTC
	mov ax, -1
	sub bx, 80 * 2				; first line

.MESSAGEINIT:
    mov si, 0
    mov di, 0
	add bx, 80 * 2				; for nextline
	inc ax

	cmp ax, 3					; MAXLINE
	je .MESSAGEEND

.MESSAGELOOP:
	cmp ax, 1
	jge .NEXT1
    mov cl, byte [ si + MESSAGE1 ]
	jmp .PRINT

.NEXT1:
	cmp ax, 2
	jge .NEXT2
	mov cl, byte[ si + CLOCK_STRING ]
	jmp .PRINT

.NEXT2:
	mov cl, byte[ si + MESSAGE2 ]

.PRINT:
	cmp cl, 0
	je .MESSAGEINIT

    mov byte [ es: di  + bx ], cl
	inc si
    add di, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    jmp $

.BCDTOHEX:						; arg: ax, result: bx
	push cx
	mov cl, al

	shr al, 4					; 10's place
	mov bh, al
	
	and cl, 0x0F				; 1's place
	mov bl, cl

	pop cx
	ret

.BCDTOASCII:
	call .BCDTOHEX
	add bh, 48
	add bl, 48
	ret

.HEXCONVERT:				; arg: ax, result: bx
	call .BCDTOHEX
	push cx
	mov al, bh
	mov cl, 10
	mul cl
	add bl, al
	pop cx
	ret

.CYMD:
	call .BCDTOASCII
.CYMDREUSE:
	mov [ CLOCK_STRING + si ], bh
	inc si
	mov [ CLOCK_STRING + si ], bl
	ret

.GETRTC:
	pusha

	mov ah, 4h					; Select 'Read RTC Calendar'
	int 1Ah						; RTC sevices interrupt

    mov al, ch
    mov si, 20
    call .CYMD					; Get century

    mov al, cl                  ; Get year
    mov si, 22
    call .CYMD

    mov al, dh                  ; Get month
    mov si, 17
    call .CYMD

    mov al, dl                  ; Get date
    mov si, 14
    call .CYMD

	call .GETYOIL

	popa
	ret

.GETYOIL:						; cx <--- [ CC | YY ], dx <--- [ MM | DD ]
	push dx
	call .CALCCCYY
	pop dx

	call .CALCMM

	mov si, dx

	sub si, 2
	mov bh, byte[ YOIL + si ]
	mov bl, byte[ YOIL + si + 1 ]
	mov cl, byte[ YOIL + si + 2 ]
	
	mov si, 25
	call .CYMDREUSE
	inc si

	mov byte[ CLOCK_STRING + si ], cl

	ret

.CALCCCYY:
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

	;sub cx, 00					 (century)year - (19)00

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
	add ax, cx					; ex) 28(bx) + floor(119 mod(400)) / 100) = 29(bx)
	cmp ax, 0					; avoid 0 divide
	je .PASS
	mov dx, 0					; Sanitize dx
	mov bx, 7
	div bx
	mov ax, dx					; ex) ax = 29 mod 7
	mov bx, 2
	mul bx
	mov cx, ax					; ex) cx = (29 mod 7) * (366 mod 7) = 1 * 2

.PASS:
	mov ax, word[YEARGAP]
	mov dx, 0					; Sanitize dx
	mov bx, 7
	div bx						; ex) 90 mod(7) * 365 mod(7) = 6 * 1(const)
	add cx, dx					; cx = result

	ret

.CALCMM:						; dx: MM | DD, cl: calcCCYY result
	mov al, dh					; month(bcd)
	call .HEXCONVERT
	mov al, bl					; al: cmpMonth, bl: month
	mov bh, 1					; bh: isFEB flag set
	mov ah, 0					; ah: cmp31

	push dx						; save date

.MONTHLOOP:
	cmp al, 7
	jle .CMP31					; if(cmpMonth <= 7) means before July
	
	xor al, ah
	and al, 0x01
	je .MONTH30					; if((cmpMonth ^ cmp31) & 1)
	add cx, 31
	jmp .ENDPOINT

.CMP31:
	mov ah, 1
	jmp .MONTHLOOP

.MONTH30:						; else
	cmp bh, 1					; if(isFEB)
	je .FEBRUARY
	add cx, 30
	jmp .ENDPOINT

.FEBRUARY:
	mov bh, 0
	cmp byte[ YOONFLAG ], 1		; if(yoonFlag)
	je .IFYOON
	add cx, 28
	jmp .ENDPOINT

.IFYOON:
	add cx, 29
	jmp .ENDPOINT

.ENDPOINT:
	cmp bl, al
	jl .MONTHLOOP				; while(month >= cmpMonth)

.CALCDD:						; cx: monthResult, dl: DD
	pop ax						; pop dx, mov ax, dx(date(bcd))
	call .HEXCONVERT
	mov al, 0
	add al, bl					; day += monthResult

	mov ax, cx					; date + monthResult
	mov dx, 0					; Sanitize dx
	mov cx, 7
	div cx						; dx is index of yoil

	ret

MESSAGE1:		db 'MINT64 OS Boot Loader Start~!!', 0
CLOCK_STRING:	db 'Current Data: 00/00/0000 FFF', 0
MESSAGE2:		db 'OS Image Loading... Complete~!!', 0
YOIL:			db 'SUNMONTUEWEDTHUFRISAT', 0

YEARGAP:		dw 0
YOONFLAG:		db 0

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
