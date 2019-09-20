# file      BootLoader.asm
# date      16/09/2019
# author    kordood
#           Copyright(c)2019 All rights reserved by kordood
# brief     MINT64 Kernel 32-bits

[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1020:START

SECTORCOUNT:        dw  0x0000
TOTALSECTORCOUNT    equ 1023

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SECTION: CODE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, cs
    mov ds, ax
    mov ax, 0xB800

    mov es, ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Generate code each sector
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    %assign i   0
    %rep TOTALSECTORCOUNT
        %assign i   i + 1
    
        mov ax, 2

        mul word [ SECTORCOUNT ]
        mov si, ax

        mov byte [ es: si + ( 240 * 2 ) ], '0' + ( i % 10 )
        add word [ SECTORCOUNT ], 1

        %if i == TOTALSECTORCOUNT
            jmp $
        %else
            jmp ( 0x1020 + i * 0x20 ): 0x0000
        %endif
        
        times ( 512 - ( $ - $$ ) % 512 )    db 0x00

    %endrep
