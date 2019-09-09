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

	popa
	ret

MESSAGE1:    db 'MINT64 OS Boot Loader Start~!!', 0
CLOCK_STRING: db 'Current Data: 00/00/0000 FFF', 0

times 510 - ( $ - $$ )    db    0x00

db 0x55
db 0xAA
