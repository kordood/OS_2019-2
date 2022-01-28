# OS
19-2 SSU(SW) OS team project.

## 과제1 : 부트로더 작성하기 (4장 ~ 5장)
- 추가 구현 :
  - 현재 날짜를 출력 및 요일 출력

## 과제2 : 커널 초기화 (6장 ~ 10장)
- 추가 구현 : 
  - 사용 가능 메모리 크기 알아내기
  - 페이지 테이블 이용
    - Double Mapping
    - Finer-grained Page Table
    - Read-only page

## 과제3 : 쉘 구현하기 (11장 ~ 15장)   
- 추가 구현 :
  - Exception Handler 구현
    - page fault
    - protection fault
  - Shell 기능 확장
    - 명령어 history 기능
    - 명령어 자동 입력 기능

## 과제4 : 멀티태스킹 구현 및 스케쥴러 구현 (16장 ~ 21장)   
- 추가 구현 :
  - Proportional Share 스케쥴러 만들기
    - Lottery Scheduler
    - Stride Scheduler

## 과제5 : 동적 메모리 관리 및 파일 시스템 구현 (23장 ~ 27장)   
- 추가 구현 :
  - 파일 시스템 추가 구현
    - subdirectory 구현
    - 파일 세부정보 구현

## 추가 구현
- fork()
  -  호출한 Process의 메모리 공간, 레지스터, PC 값을 복사하여 새로운 Process 생성
- exec()
  - SSU Binary File을 메모리에 로딩하여, 프로세스를 새롭게 시작하도록 구현
- vi editor (hex editor)
  - Editor 내에 Hex 값 (0 ~ 9, a ~ f, A ~ F) 입력시 Disk에 Hex값 그대로 저장
---
(Eng)
# OS
19-2 SSU(SW) OS team project.

## Assignment #1 : Write Bootloader (Chapter 4 ~ 5)
- Extra Implementation :
  - Print out the date of today and the day of the week.

## Assignment #2 : Initialize Kernel (Chapter 6 ~ 10)
- Extra Implementation : 
  - Figure out a size of availible memory
  - Manage a page table includes below features
    - Double Mapping
    - Finer-grained Page Table
    - Read-only Page

## Assignment #3 : Implement Shell (Chapter 11 ~ 15)   
- Extra Implementation :
  - Implement exception handler includes below features
    - Page Fault
    - Protection Fault
  - Support additional features like below
    - Command History
    - Automatic Command Completion

## Assignment #4 : Implement Multitasking and Scheduler (Chapter 16 ~ 21)   
- Extra Implementation :
  - Support proportional share scheduler such as below
    - Lottery Scheduler
    - Stride Scheduler

## Assignment #5 : Manage dynamic memory and implement file system (Chapter 23 ~ 27)   
- Extra Implementation :
  - Support file system features like below
    - Subdirectory
    - Detail of a file

## Extra Implementation (free subject)
- fork()
  -  Generate a new process with copying memory spaces, registers and program counter of caller process.
- exec()
  - Start a process newly by loading SSU binary file on memory.
- vi editor (hex editor)
  - Write down, to disk, hex values (0 ~ 9, a ~ f, A ~ F) when a user type and save them on the editor.
