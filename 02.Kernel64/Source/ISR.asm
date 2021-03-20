# file		ISR.asm
# date      2009/01/24
# author	kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief		ÀÎÅÍ·ŽÆ® Œ­ºñœº ·çÆŸ(ISR) °ü·ÃµÈ ŒÒœº ÆÄÀÏ

[BITS 64]           ; ÀÌÇÏÀÇ ÄÚµåŽÂ 64ºñÆ® ÄÚµå·Î Œ³Á€

SECTION .text       ; text ŒœŒÇ(ŒŒ±×žÕÆ®)À» Á€ÀÇ

; ¿ÜºÎ¿¡Œ­ Á€ÀÇµÈ ÇÔŒöžŠ Ÿµ Œö ÀÖµµ·Ï Œ±ŸðÇÔ(Import)
extern kCommonExceptionHandler, kCommonInterruptHandler, kKeyboardHandler
extern kTimerHandler, kDeviceNotAvailableHandler, kHDDHandler

; C ŸðŸî¿¡Œ­ È£ÃâÇÒ Œö ÀÖµµ·Ï ÀÌž§À» ³ëÃâÇÔ(Export)
; ¿¹¿Ü(Exception) Ã³ž®žŠ À§ÇÑ ISR
global kISRDivideError, kISRDebug, kISRNMI, kISRBreakPoint, kISROverflow
global kISRBoundRangeExceeded, kISRInvalidOpcode, kISRDeviceNotAvailable, kISRDoubleFault,
global kISRCoprocessorSegmentOverrun, kISRInvalidTSS, kISRSegmentNotPresent
global kISRStackSegmentFault, kISRGeneralProtection, kISRPageFault, kISR15
global kISRFPUError, kISRAlignmentCheck, kISRMachineCheck, kISRSIMDError, kISRETCException

; ÀÎÅÍ·ŽÆ®(Interrupt) Ã³ž®žŠ À§ÇÑ ISR
global kISRTimer, kISRKeyboard, kISRSlavePIC, kISRSerial2, kISRSerial1, kISRParallel2
global kISRFloppy, kISRParallel1, kISRRTC, kISRReserved, kISRNotUsed1, kISRNotUsed2
global kISRMouse, kISRCoprocessor, kISRHDD1, kISRHDD2, kISRETCInterrupt

; ÄÜÅØœºÆ®žŠ ÀúÀåÇÏ°í Œ¿·ºÅÍžŠ ±³ÃŒÇÏŽÂ žÅÅ©·Î
%macro KSAVECONTEXT 0       ; ÆÄ¶ó¹ÌÅÍžŠ ÀüŽÞ¹ÞÁö ŸÊŽÂ KSAVECONTEXT žÅÅ©·Î Á€ÀÇ
    ; RBP ·¹ÁöœºÅÍºÎÅÍ GS ŒŒ±×žÕÆ® Œ¿·ºÅÍ±îÁö žðµÎ œºÅÃ¿¡ »ðÀÔ
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds      ; DS ŒŒ±×žÕÆ® Œ¿·ºÅÍ¿Í ES ŒŒ±×žÕÆ® Œ¿·ºÅÍŽÂ œºÅÃ¿¡ Á÷Á¢
    push rax        ; »ðÀÔÇÒ Œö ŸøÀž¹Ç·Î, RAX ·¹ÁöœºÅÍ¿¡ ÀúÀåÇÑ ÈÄ œºÅÃ¿¡ »ðÀÔ
    mov ax, es
    push rax
    push fs
    push gs	

    ; ŒŒ±×žÕÆ® Œ¿·ºÅÍ ±³ÃŒ
    mov ax, 0x10    ; AX ·¹ÁöœºÅÍ¿¡ Ä¿³Î µ¥ÀÌÅÍ ŒŒ±×žÕÆ® µðœºÅ©ž³ÅÍ ÀúÀå
    mov ds, ax      ; DS ŒŒ±×žÕÆ® Œ¿·ºÅÍºÎÅÍ FS ŒŒ±×žÕÆ® Œ¿·ºÅÍ±îÁö žðµÎ
    mov es, ax      ; Ä¿³Î µ¥ÀÌÅÍ ŒŒ±×žÕÆ®·Î ±³ÃŒ
   	mov gs, ax
   	mov fs, ax
%endmacro           ; žÅÅ©·Î ³¡


; ÄÜÅØœºÆ®žŠ º¹¿øÇÏŽÂ žÅÅ©·Î
%macro KLOADCONTEXT 0   ; ÆÄ¶ó¹ÌÅÍžŠ ÀüŽÞ¹ÞÁö ŸÊŽÂ KSAVECONTEXT žÅÅ©·Î Á€ÀÇ
    ; GS ŒŒ±×žÕÆ® Œ¿·ºÅÍºÎÅÍ RBP ·¹ÁöœºÅÍ±îÁö žðµÎ œºÅÃ¿¡Œ­ ²š³» º¹¿ø
    pop gs
    pop fs
    pop rax
    mov es, ax      ; ES ŒŒ±×žÕÆ® Œ¿·ºÅÍ¿Í DS ŒŒ±×žÕÆ® Œ¿·ºÅÍŽÂ œºÅÃ¿¡Œ­ Á÷Á¢
    pop rax         ; ²š³» º¹¿øÇÒ Œö ŸøÀž¹Ç·Î, RAX ·¹ÁöœºÅÍ¿¡ ÀúÀåÇÑ µÚ¿¡ º¹¿ø
    mov ds, ax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro       ; žÅÅ©·Î ³¡

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	¿¹¿Ü ÇÚµé·¯
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #0, Divide Error ISR
kISRDivideError:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 0
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #1, Debug ISR
kISRDebug:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 1
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #2, NMI ISR
kISRNMI:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 2
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #3, BreakPoint ISR
kISRBreakPoint:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 3
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #4, Overflow ISR
kISROverflow:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 4
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #5, Bound Range Exceeded ISR
kISRBoundRangeExceeded:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 5
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #6, Invalid Opcode ISR
kISRInvalidOpcode:
    pop rax
    jmp $
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 6
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #7, Device Not Available ISR
kISRDeviceNotAvailable:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 7
    call kDeviceNotAvailableHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #8, Double Fault ISR
kISRDoubleFault:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 8
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #9, Coprocessor Segment Overrun ISR
kISRCoprocessorSegmentOverrun:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 9
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #10, Invalid TSS ISR
kISRInvalidTSS:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 10
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #11, Segment Not Present ISR
kISRSegmentNotPresent:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 11
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #12, Stack Segment Fault ISR
kISRStackSegmentFault:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 12
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #13, General Protection ISR
kISRGeneralProtection:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 13
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #14, Page Fault ISR
kISRPageFault:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£¿Í ¿¡·¯ ÄÚµåžŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 14
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #15, Reserved ISR
kISR15:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 15
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #16, FPU Error ISR
kISRFPUError:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 16
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #17, Alignment Check ISR
kISRAlignmentCheck:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 17
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    add rsp, 8      ; ¿¡·¯ ÄÚµåžŠ œºÅÃ¿¡Œ­ ÁŠ°Å
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #18, Machine Check ISR
kISRMachineCheck:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 18
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #19, SIMD Floating Point Exception ISR
kISRSIMDError:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 19
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #20~#31, Reserved ISR
kISRETCException:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ¿¹¿Ü ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 20
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	ÀÎÅÍ·ŽÆ® ÇÚµé·¯
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #32, ÅžÀÌžÓ ISR
kISRTimer:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 32    
    call kTimerHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #33, Å°ºžµå ISR
kISRKeyboard:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 33
    call kKeyboardHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø

; #34, œœ·¹ÀÌºê PIC ISR
kISRSlavePIC:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 34
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #35, œÃž®Ÿó Æ÷Æ® 2 ISR
kISRSerial2:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 35
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #36, œÃž®Ÿó Æ÷Æ® 1 ISR
kISRSerial1:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 36
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #37, ÆÐ·¯·Œ Æ÷Æ® 2 ISR
kISRParallel2:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 37
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #38, ÇÃ·ÎÇÇ µðœºÅ© ÄÁÆ®·Ñ·¯ ISR
kISRFloppy:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 38
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #39, ÆÐ·¯·Œ Æ÷Æ® 1 ISR
kISRParallel1:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 39
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #40, RTC ISR
kISRRTC:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 40
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #41, ¿¹ŸàµÈ ÀÎÅÍ·ŽÆ®ÀÇ ISR
kISRReserved:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 41
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #42, »ç¿ë ŸÈÇÔ
kISRNotUsed1:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 42
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #43, »ç¿ë ŸÈÇÔ
kISRNotUsed2:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 43
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #44, ž¶¿ìœº ISR
kISRMouse:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 44
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #45, ÄÚÇÁ·ÎŒŒŒ­ ISR
kISRCoprocessor:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 45
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #46, ÇÏµå µðœºÅ© 1 ISR
kISRHDD1:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 46
    call kHDDHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
    
; #47, ÇÏµå µðœºÅ© 2 ISR
kISRHDD2:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 47
    call kHDDHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
      
; #48 ÀÌ¿ÜÀÇ žðµç ÀÎÅÍ·ŽÆ®¿¡ ŽëÇÑ ISR              
kISRETCInterrupt:
    KSAVECONTEXT    ; ÄÜÅØœºÆ®žŠ ÀúÀåÇÑ µÚ Œ¿·ºÅÍžŠ Ä¿³Î µ¥ÀÌÅÍ µðœºÅ©ž³ÅÍ·Î ±³ÃŒ
    
    ; ÇÚµé·¯¿¡ ÀÎÅÍ·ŽÆ® ¹øÈ£žŠ »ðÀÔÇÏ°í ÇÚµé·¯ È£Ãâ
    mov rdi, 48
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; ÄÜÅØœºÆ®žŠ º¹¿ø
    iretq           ; ÀÎÅÍ·ŽÆ® Ã³ž®žŠ ¿Ï·áÇÏ°í ÀÌÀü¿¡ ŒöÇàÇÏŽø ÄÚµå·Î º¹¿ø
