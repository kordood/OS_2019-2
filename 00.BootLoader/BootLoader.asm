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

.GETRTC:
	pusha

	mov ah, 4h					; Select 'Read RTC Calendar'
	int 1Ah						; RTC sevices interrupt

	mov al, ch					; Get century
	shr al, 4					; Take 10's place
	add al, 48					; bit to ascii
	mov [CLOCK_STRING+20], al	; Saving to string

	mov al, ch
	and al, 0x0F				; Take 1's place
	add al, 48
	mov [CLOCK_STRING+21], al

	mov al, cl					; Get year
	shr al, 4
	add al, 48
	mov [CLOCK_STRING+22], al

	mov al, cl
	and al, 0x0F
	add al, 48
	mov [CLOCK_STRING+23], al

	mov al, dh					; Get month
	shr al, 4
	add al, 48
	mov [CLOCK_STRING+17], al

	mov al, dh
	and al, 0x0F
	add al, 48
	mov [CLOCK_STRING+18], al

	mov al, dl					; Get date
	shr al, 4
	add al, 48
	mov [CLOCK_STRING+14], al

	mov al, dl
	and al, 0x0F
	add al, 48
	mov [CLOCK_STRING+15], al

	call .GETYOIL

	popa
	ret

.GETYOIL:
	mov ax, cx
	mov bx, dx
	push cx
	push dx

	mov cx, ax					; cx <--- [ YY | YY ]
	call .CALCYYYY
	mov ax, cx

	mov cx, bx					; cx <--- [ MM | DD ]
	call .CALCMMDD

	pop dx
	pop cx
	ret

.CALCYYYY:
	push bx						; save MMDD
	mov dh, 19
	mov dl, 00					; dx <--- [ 19 | 00 ]

	mov al, ch
	and al, 0x0F				; 1's place
	mov bl, al

	mov al, ch					; century
	shr al, 4

	mov bh, 10					; 10's place
	mul bh

	add bl, al
	sub bl, dh					; century(year) - 19(00)

	mov ax, bx					; result 100's place
	mov bx, 100
	mul bx
	mov bx, ax
	
	mov al, cl
	and al, 0x0F				; year
	add bl, al

	mov al, cl
	shr al, 4
	mov ch, 10
	mul ch
	add bl, al
	sub bl, dl					; bx contains result of yyyy - 1900		ex) [ 119 ]

	mov cx, bx					; result

	call .YOIL1

	pop bx
	ret

.YOIL1:
	mov ax, cx
	mov dx, 4
	div dx						; ex) 119 / 4 = ax ... dx
	mov bx, ax

	mov ax, cx
	mov dx, 100
	div dx
	sub bx, ax					; ex) 29 - floor(119 / 100) = 28(bx)

	mov ax, cx
	mov dx, 400
	div dx
	mov ax, dx
	mov dx, 100
	div dx
	add bx, ax					; ex) 28(bx) + floor(119 mod(400)) / 100) = 29(bx)

	sub cx, bx					; ex) cx: 119 - 29, bx: 29

	mov ax, cx
	mov dx, 7
	div dx
	mov ax, dx
	mov dl, 1
	mul dl 						; ex) 90 mod(7) * 365 mod(7)
	mov cx, ax

	mov ax, bx
	mov dx, 7
	div dx
	mov ax, dx
	mov dl, 2
	mul dl						; ex) 29 mod(7) * 366 mod(7)

	add ax, cx
	mov dx, 7
	div dx
	mov cx, dx					; cx mod(7)




.CALCMMDD:
	push ax
	mov dh, 01
	mov dl, 01					; dx <--- [ 01 | 01 ]

	pop ax
	ret


MESSAGE1:    db 'MINT64 OS Boot Loader Start~!!', 0
CLOCK_STRING: db 'Current Data: 00/00/0000 FFF', 0

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
