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

## 추가 기능
- fork()
  -  호출한 Process의 메모리 공간, 레지스터, PC 값을 복사하여 새로운 Process 생성
- exec()
  - SSU Binary File을 메모리에 로딩하여, 프로세스를 새롭게 시작하도록 구현
- vi editor (hex editor)
  - Editor 내에 Hex 값 (0 ~ 9, a ~ f, A ~ F) 입력시 Disk에 Hex값 그대로 저장
