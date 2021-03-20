[ORG 0x00]          ; 코드의 시작 어드레스를 0x00으로 설정
[BITS 64]           ; 이하의 코드는 16비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

push rbp
mov rbp, rsp

mov rsi, rdi
mov rcx, 0
mov rax, 0xb80A0
mov rdi, rax

MESSAGELOOP:               ; 메시지를 출력하는 루프
    mov cl, byte [ rsi ]     ; rsi 레지스터가 가리키는 문자열 위치에서 한 문자를 
                            ; rcx 레지스터에 복사
                            ; rcx 레지스터는 rcx 레지스터의 하위 1바이트를 의미
                            ; 문자열은 1바이트면 충분하므로 rcx 레지스터의 하위 1바이트만 사용
    
    cmp ecx, 0               ; 복사된 문자와 0을 비교
    je .MESSAGEEND          ; 복사한 문자의 값이 0이면 문자열이 종료되었음을
                            ; 의미하므로 .MESSAGEEND로 이동하여 문자 출력 종료

    mov byte [ rdi ], cl ; 0이 아니라면 비디오 메모리 어드레스 0xB800:rdi에 문자를 출력
    
    add rsi, 1               ; rsi 레지스터에 1을 더하여 다음 문자열로 이동
    add rdi, 2               ; rdi 레지스터에 2를 더하여 비디오 메모리의 다음 문자 위치로 이동
                            ; 비디오 메모리는 (문자, 속성)의 쌍으로 구성되므로 문자만 출력하려면
                            ; 2를 더해야 함

    jmp MESSAGELOOP        ; 메시지 출력 루프로 이동하여 다음 문자를 출력
	
.MESSAGEEND:

    mov rsp, rbp
	pop rbp
    ret
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 부트 로더 시작 메시지
SECTION .data
MESSAGE1:    db 'Execve Successfully Started!!', 0 ; 출력할 메시지 정의
       