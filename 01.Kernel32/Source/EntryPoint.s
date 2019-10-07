# file      EntryPoint.s
# date      2008/11/27
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     보호 모드 커널 엔트리 포인트에 관련된 소스 파일

[ORG 0x00]
[BITS 16]

SECTION .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
	mov ax, 0x1000  ; 0x10000

	mov ds, ax      ; DS 
	mov es, ax      ; ES 

<<<<<<< HEAD
    mov ds, ax      ; DS 
    mov es, ax      ; ES 
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; A20gate BIOS -> System control port
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, 0x2401
	int 0x15

	jc .A20GATEERROR
	jmp .A20GATESUCCESS

	.A20GATEERROR:
		In al, 0x92
		or al, 0x02
		and al, 0xFE
		out 0x92, al

	.A20GATESUCCESS:

    
    cli
    lgdt [ GDTR ]
=======
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Memory Available Size Print
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	pusha
	call MMAP
	call printUsableMem
>>>>>>> 8feb8cf7c46e32f280455106207dfd58e0d2b509

	mov ax, 0xB800
	mov es, ax

	mov ax, 3
	mov si, 160 
	mul si
	mov di, ax

    mov si, PRINTRAM

.MESSAGELOOP:
    mov cl, byte [ si ]

    cmp cl, 0
    je .MESSAGEEND

    mov byte [ es: di ], cl
    add si, 1
    add di, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
	popa
	jmp NEXT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MMAP:
mmap_ent equ 0x8000             ; the number of entries will be stored at 0x8000
do_e820:
	mov di, 0x8004          	; Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
	xor ebx, ebx				; ebx must be 0 to start
	xor bp, bp					; keep an entry count in bp
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes
	int 0x15
	jc short .failed			; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150		; Some BIOSes apparently trash this register?
	cmp eax, edx				; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx				; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	call getLengthLow
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820				; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes again
	int 0x15
	jc short .e820f				; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register
	call getLengthLow
.jmpin:
	jcxz .skipent				; skip any 0 length entries
	cmp cl, 20					; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]		; get lower uint32_t of memory region length
	or ecx, [es:di + 12]		; "or" it with upper uint32_t to test for zero
	jz .skipent					; if length uint64_t is 0, skip entry
	inc bp						; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx				; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [mmap_ent], bp			; store the entry count
	clc							; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.failed:
	stc							; "function unsupported" error exit
	ret

getLengthLow:
	mov eax, [ es:di + 8 ]
	add dword [ MEMSIZE ], eax
	ret

printUsableMem:					; now support < 100MB
	xor edx, edx
	mov eax, dword [ MEMSIZE ]
	mov ebx, 0x100000
	div ebx
	mov ecx, eax
	xor edx, edx
	mov ebx, 10
	div ebx
	add eax, 48
	add edx, 48
	mov byte [ PRINTRAM + 10], al
	mov byte [ PRINTRAM + 11], dl
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	End
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NEXT:

mov ax, 0x2401          ; A20
int 0x15                ; BIOS

jc .A20GATEERROR        ; A20
jmp .A20GATESUCCESS

.A20GATEERROR:
in al, 0x92
or al, 0x02
and al, 0xFE
out 0x92, al

.A20GATESUCCESS:

cli
lgdt [ GDTR ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Disable Paging, Disable Cache, Internal FPU, Disable Align Check, 
	; Enable ProtectedMode
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, 0x4000003B ; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
	mov cr0, eax        ; CR0

	jmp dword 0x18: ( PROTECTEDMODE - $$ + 0x10000 )

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	;
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]
PROTECTEDMODE:
	mov ax, 0x20
	mov ds, ax          ; DS
	mov es, ax          ; ES
	mov fs, ax          ; FS
	mov gs, ax          ; GS

	; 0x00000000~0x0000FFFF 64KB
	mov ss, ax          ; SS
	mov esp, 0xFFFE     ; ESP
	mov ebp, 0xFFFE     ; EBP

	push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )
	push 4
	push 0
	call PRINTMESSAGE
	add esp, 12

	jmp dword 0x18: 0x10200


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	;
PRINTMESSAGE:
push ebp
mov ebp, esp
push esi
push edi
push eax
push ecx
push edx

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; X
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Y
mov eax, dword [ ebp + 12 ]
mov esi, 160
mul esi
mov edi, eax

; X
mov eax, dword [ ebp + 8 ]
mov esi, 2
mul esi
add edi, eax

mov esi, dword [ ebp + 16 ]

.MESSAGELOOP:
mov cl, byte [ esi ]

cmp cl, 0
je .MESSAGEEND

mov byte [ edi + 0xB8000 ], cl

add esi, 1
add edi, 2

jmp .MESSAGELOOP

.MESSAGEEND:
pop edx
pop ecx
pop eax
pop edi
pop esi
pop ebp
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
align 8, db 0

; GDTR
dw 0x0000
; GDTR
GDTR:
	dw GDTEND - GDT - 1
dd ( GDT - $$ + 0x10000 )

; GDT
GDT:

NULLDescriptor:
	dw 0x0000
	dw 0x0000
	db 0x00
	db 0x00
	db 0x00
	db 0x00
	
IA_32eCODEDESCRIPTOR:
    dw 0xFFFF       ; Limit [15:0]
	dw 0x0000       ; Base [15:0]
	db 0x00         ; Base [23:16]
	db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
	db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
	db 0x00         ; Base [31:24] 

    IA_32eDATADESCRIPTOR:
	dw 0xFFFF       ; Limit [15:0]
	dw 0x0000       ; Base [15:0]
	db 0x00         ; Base [23:16]
	db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
	db 0xAF         ; G=1, D=0, L=1, Limit[19:16]
	db 0x00         ; Base [31:24]

CODEDESCRIPTOR:     
	dw 0xFFFF       ; Limit [15:0]
	dw 0x0000       ; Base [15:0]
	db 0x00         ; Base [23:16]
	db 0x9A         ; P=1, DPL=0, Code Segment, Execute/Read
	db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
	db 0x00         ; Base [31:24]  


DATADESCRIPTOR:
	dw 0xFFFF       ; Limit [15:0]
	dw 0x0000       ; Base [15:0]
	db 0x00         ; Base [23:16]
	db 0x92         ; P=1, DPL=0, Data Segment, Read/Write
	db 0xCF         ; G=1, D=1, L=0, Limit[19:16]
	db 0x00         ; Base [31:24]
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0
PRINTRAM: db 'RAM Size: XX MB', 0	; XX -> 10, 11

MEMSIZE: dw 0

times 512 - ( $ - $$ )  db  0x00    ; 512
