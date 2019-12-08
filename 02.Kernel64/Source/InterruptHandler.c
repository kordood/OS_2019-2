/**
 *  file    InterruptHandler.c
 *  date    2009/01/24
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   인터럽트 및 예외 핸들러에 관련된 소스 파일
 */

#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"

/**
 *  공통으로 사용하는 예외 핸들러
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    
    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "                    Vector:                         " );
    kPrintStringXY( 27, 2, vcBuffer );
    kPrintStringXY( 0, 3, "====================================================" );

    while( 1 ) ;
}


void kPagefaultHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char ecBuffer[ 5 ] = { 0, };
    ecBuffer[ 0 ] = '0' + (qwErrorCode>>32) / 10;
    ecBuffer[ 1 ] = '0' + (qwErrorCode>>32) % 10;
    ecBuffer[ 2 ] = '0' + qwErrorCode / 10;
    ecBuffer[ 3 ] = '0' + qwErrorCode % 10;
	kPrintf("error code: %s\n", ecBuffer);

	DWORD entryIndex;
	QWORD CR2, CR3;

	__asm__ __volatile__(
		"mov %%cr2, %%rax \n\t"
		"mov %%rax, %0 \n\t"
		"mov %%cr3, %%rax \n\t"
		"mov %%rax, %1 \n\t"
	: "=m" (CR2), "=m" (CR3));			// CR2: fault addr(0x1ff000), CR3: P

	entryIndex = (CR2 >> 12);

	if(entryIndex > 511){
		kPrintStringXY( 32, 4, "Entry index bound error");
		while(1);
	}
	

	QWORD pt_index, pd_index, pdpt_index, pml4_index;
	QWORD *ptr;
	pt_index = (CR2 >> 12) & 0x1ff;
	pd_index = (CR2 >> 21) & 0x1ff;
	pdpt_index = (CR2 >> 30) & 0x1ff;
	pml4_index = (CR2 >> 39) & 0x1ff;

	ptr = (QWORD *)CR3 + pml4_index;
	
	if(((*ptr) & 0x01) ^ 0x01){
		*ptr = (*ptr) ^ 0x01;
	}

	ptr = (QWORD *)((*ptr)&0xFFFFFFF000) + pdpt_index;

	if(((*ptr) & 0x01) ^ 0x01){
		*ptr = (*ptr) ^ 0x01;
	}

	ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pd_index;

	if(((*ptr) & 0x01) ^ 0x01){
		*ptr = (*ptr) ^ 0x01;
	}

	kPrintf("idx: 0x%x\n", pd_index);
	if(pml4_index == 0 && pdpt_index == 0 && pd_index == 0){
	ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pt_index;

	if(((*ptr) & 0x01) ^ 0x01){
		*ptr = (*ptr) ^ 0x01;
	}
	QWORD var = *ptr;

	ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pt_index;
	}

	invlpg(&CR2);

	kPrintf("====================================================\n");
	kPrintf("                 Page Fault Occurs~!                \n" );
	kPrintf("                  Address: 0x%x                     \n", CR2);
	kPrintf("====================================================\n");
}


void kProtectionfaultHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char ecBuffer[ 5 ] = { 0, };
    ecBuffer[ 0 ] = '0' + (qwErrorCode>>32) / 10;
    ecBuffer[ 1 ] = '0' + (qwErrorCode>>32) % 10;
    ecBuffer[ 2 ] = '0' + qwErrorCode / 10;
    ecBuffer[ 3 ] = '0' + qwErrorCode % 10;
    kPrintf("error code: %s\n", ecBuffer);

    DWORD entryIndex;
    QWORD CR2, CR3;

    __asm__ __volatile__(
        "mov %%cr2, %%rax \n\t"
        "mov %%rax, %0 \n\t"
        "mov %%cr3, %%rax \n\t"
        "mov %%rax, %1 \n\t"
    : "=m" (CR2), "=m" (CR3));          // CR2: fault addr(0x1ff000), CR3: P

    entryIndex = (CR2 >> 12);

    if(entryIndex > 511){
        kPrintStringXY( 32, 4, "Entry index bound error");
        while(1);
    }

    QWORD pt_index, pd_index, pdpt_index, pml4_index;
    QWORD *ptr;
    pt_index = (CR2 >> 12) & 0x1ff;
    pd_index = (CR2 >> 21) & 0x1ff;
    pdpt_index = (CR2 >> 30) & 0x1ff;
    pml4_index = (CR2 >> 39) & 0x1ff;

    ptr = (QWORD *)CR3 + pml4_index;
    
    if(((*ptr) & 0x02) ^ 0x02){
        *ptr = (*ptr) ^ 0x02;
    }

    ptr = (QWORD *)((*ptr)&0xFFFFFFF000) + pdpt_index;

    if(((*ptr) & 0x02) ^ 0x02){
        *ptr = (*ptr) ^ 0x02;
    }

    ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pd_index;

    if(((*ptr) & 0x02) ^ 0x02){
        *ptr = (*ptr) ^ 0x02;
    }

    if(pml4_index == 0 && pdpt_index == 0 && pd_index == 0){
    ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pt_index;

    if(((*ptr) & 0x02) ^ 0x02){
        *ptr = (*ptr) ^ 0x02;
    }
    QWORD var = *ptr;

    ptr = (QWORD *)(*ptr&0xFFFFFFF000) + pt_index;
    }

    invlpg(&CR2);

    kPrintf("====================================================\n");
    kPrintf("              Protection Fault Occurs~!             \n" );
    kPrintf("                  Address: 0x%x                     \n", CR2);
    kPrintf("====================================================\n");
}

/**
 *  공통으로 사용하는 인터럽트 핸들러 
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  키보드 인터럽트의 핸들러
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  타이머 인터럽트의 핸들러
 */
void kTimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );

    // 타이머 발생 횟수를 증가
    g_qwTickCount++;

    // 태스크가 사용한 프로세서의 시간을 줄임
    kDecreaseProcessorTime();
    // 프로세서가 사용할 수 있는 시간을 다 썼다면 태스크 전환 수행
    if( kIsProcessorTimeExpired() == TRUE )
    {
        kScheduleInInterrupt();
    }
}

/**
 *  Device Not Available 예외의 핸들러
 */
void kDeviceNotAvailableHandler( int iVectorNumber )
{
    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;

    //=========================================================================
    // FPU 예외가 발생했음을 알리려고 메시지를 출력하는 부분
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    // 예외 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );    
    //=========================================================================
    
    // CR0 컨트롤 레지스터의 TS 비트를 0으로 설정
    kClearTS();

    // 이전에 FPU를 사용한 태스크가 있는지 확인하여, 있다면 FPU의 상태를 태스크에 저장
    qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    pstCurrentTask = kGetRunningTask();
    
    // 이전에 FPU를 사용한 것이 자신이면 아무것도 안 함
    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    {
        return ;
    }
    // FPU를 사용한 태스크가 있으면 FPU 상태를 저장
    else if( qwLastFPUTaskID != TASK_INVALIDID )
    {
        pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        {
            kSaveFPUContext( pstFPUTask->vqwFPUContext );
        }
    }
    
    // 현재 태스크가 FPU를 사용한 적이 있는 지 확인하여 FPU를 사용한 적이 없다면 
    // 초기화하고, 사용한적이 있다면 저장된 FPU 콘텍스트를 복원
    if( pstCurrentTask->bFPUUsed == FALSE )
    {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        kLoadFPUContext( pstCurrentTask->vqwFPUContext );
    }
    
    // FPU를 사용한 태스크 ID를 현재 태스크로 변경
    kSetLastFPUUsedTaskID( pstCurrentTask->stLink.qwID );
}

/**
 *  하드 디스크에서 발생하는 인터럽트의 핸들러
 */
void kHDDHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    // 왼쪽 위에 있는 메시지와 겹치지 않도록 (10, 0)에 출력
    kPrintStringXY( 10, 0, vcBuffer );
    //=========================================================================

    // 첫 번째 PATA 포트의 인터럽트 발생 여부를 TRUE로 설정
    kSetHDDInterruptFlag( TRUE, TRUE );
    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

static inline void invlpg(void* m)
{
	asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}
