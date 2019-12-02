/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔 셸에 관련된 소스 파일
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "DoubleLinkedList.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"

// 커맨드 테이블 정의
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
        { "help", "Show Help", kHelp },
        { "cls", "Clear Screen", kCls },
        { "totalram", "Show Total RAM Size", kShowTotalRAMSize },
        { "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
        { "shutdown", "Shutdown And Reboot OS", kShutdown },
        { "settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", 
                kSetTimer },
        { "wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT },
        { "rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter },
        { "cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed },
        { "date", "Show Date And Time", kShowDateAndTime },
        { "createtask", "Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask },
        { "forktask", "Fork Task", kForkTest },
        { "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
                kChangeTaskPriority },
        { "tasklist", "Show Task List", kShowTaskList },
        { "killtask", "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)", kKillTask },
        { "cpuload", "Show Processor Load", kCPULoad },
        { "testmutex", "Test Mutex Function", kTestMutex },
        { "testthread", "Test Thread And Process Function", kTestThread },
        { "showmatrix", "Show Matrix Screen", kShowMatrix },
        { "testpie", "Test PIE Calculation", kTestPIE },               
        { "dynamicmeminfo", "Show Dyanmic Memory Information", kShowDyanmicMemoryInformation },
        { "testseqalloc", "Test Sequential Allocation & Free", kTestSequentialAllocation },
        { "testranalloc", "Test Random Allocation & Free", kTestRandomAllocation },
        { "hddinfo", "Show HDD Information", kShowHDDInformation },
        { "readsector", "Read HDD Sector, ex)readsector 0(LBA) 10(count)", 
                kReadSector },
        { "writesector", "Write HDD Sector, ex)writesector 0(LBA) 10(count)", 
                kWriteSector },
        { "mounthdd", "Mount HDD", kMountHDD },
        { "formathdd", "Format HDD", kFormatHDD },
        { "filesysteminfo", "Show File System Information", kShowFileSystemInformation },
        { "createfile", "Create File, ex)createfile a.txt", kCreateFileInRootDirectory },
        { "deletefile", "Delete File, ex)deletefile a.txt", kDeleteFileInRootDirectory },
        { "dir", "Show Directory", kShowRootDirectory },
        { "writefile", "Write Data To File, ex) writefile a.txt", kWriteDataToFile },
        { "readfile", "Read Data From File, ex) readfile a.txt", kReadDataFromFile },
        { "testfileio", "Test File I/O Function", kTestFileIO },
        { "testperformance", "Test File Read/WritePerformance", kTestPerformance },
        { "flush", "Flush File System Cache", kFlushCache },
//	{ "strcat", "String Cat(dummy)", kDummy },
//	{ "stra", "String (dummy)", kDummy },
//	{ "strcbat", "String (dummy)", kDummy },
//	{ "strcact", "String (dummy)", kDummy },
//	{ "strcats", "String (dummy)", kDummy },
//	{ "strcp", "String Copy(dummy)", kDummy },
//	{ "rm", "Remove File(dummy)", kDummy },
//	{ "rmdir", "Remove Directory(dummy)", kDummy },
//	{ "pwd", "Present Working Directory(dummy)", kDummy },
    { "pagefault", "Cause page fault", kPagefault },
    { "protfault", "Cause protection fault", kProtectionfault },
};                                     

int tabflag = 0;
//=============================================================================
//  실제 셸을 구성하는 코드
//==============================================================================
/**
 *  셸의 메인 루프
 */
void kStartConsoleShell( void )
{
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

	// 명령문 10개 저장용 list 생성
	DoubleLinkedList spdll;
	DoubleLinkedList* spDLL = CreateLinkedList(&spdll);
	int iCNT = 0;
	SNode node[10];
	char data[10][CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	char* tmpData;
	int UPnDOWN = 0;
	int DownOn = 0;
	int UpOn = 0;

    // 프롬프트 출력
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );

	/**
	 * Performance evaluation func
	 * kExecuteCommand("createtask 2 3");
	 * kExecuteCommand("createtask 3");
	 */
    
    while( 1 )
    {
		if(iCommandBufferIndex == 0){
			DownOn = 0;
			UPnDOWN = 0;
		}
        // 키가 수신될 때까지 대기
        bKey = kGetCh();
        // Backspace 키 처리
        if( bKey == KEY_BACKSPACE )
        {
			tabflag = 0;
            if( iCommandBufferIndex > 0 )
            {
                // 현재 커서 위치를 얻어서 한 문자 앞으로 이동한 다음 공백을 출력하고 
				// 커맨드 버퍼에서 마지막 문자 삭제
                kGetCursor( &iCursorX, &iCursorY );
                kPrintStringXY( iCursorX - 1, iCursorY, " " );
                kSetCursor( iCursorX - 1, iCursorY );
				vcCommandBuffer[ --iCommandBufferIndex ] = '\0';

				if(iCommandBufferIndex == 0){
					UPnDOWN = 0;
					DownOn = 0;
					UpOn = 0;
					spDLL->m_spIterator = spDLL->m_spTail;		
				}
            }
        }
        // 엔터 키 처리
        else if( bKey == KEY_ENTER )
        {
			tabflag = 0;
			kPrintf( "\n" );
			DownOn = 0;
			UPnDOWN = 0;
			UpOn = 0;
            
            if( iCommandBufferIndex > 0 )
            {
				kMemCpy(&data[iCNT%10], vcCommandBuffer, iCommandBufferIndex+1);
                // 커맨드 버퍼에 있는 명령을 실행
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                kExecuteCommand( vcCommandBuffer );
            }
            
			if(iCommandBufferIndex > 0){
				//명령문을 DoubleLinkedList에 삽입
				if(iCNT >= 10){
					DownOn = 0;
					Pop_Front(spDLL);
				}
				if(iCNT == 0){
					Push_Empty(spDLL, &data[iCNT%10], &node[iCNT%10]);
				}
				else{
					Push_Back(spDLL, &data[iCNT%10], &node[iCNT%10]);
				}
				++iCNT;
			}

            // 프롬프트 출력 및 커맨드 버퍼 초기화
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
        }
        // 시프트 키, CAPS Lock, NUM Lock, Scroll Lock은 무시
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
			tabflag = 0;
        }
	    	// 위로 가면 과거 명령어 출력
		else if((bKey == KEY_UP) || (bKey == KEY_DOWN)){
			if(bKey == KEY_UP){
				if(UPnDOWN > 0) DownOn = 1;

				if((UPnDOWN == 0)&&(UpOn == 0)){
					if(spDLL->m_spIterator!= NULL) tmpData = IteratePrevList(spDLL, UPnDOWN);	
					if(spDLL->m_spIterator!=spDLL->m_spHead) {
						UPnDOWN++;
					}
					if((spDLL->m_spIterator == spDLL->m_spHead)){
					}
				}else if ((UPnDOWN == 1)&&(UpOn == 0)){
					tmpData = IteratePrevList(spDLL, UPnDOWN);	
					UpOn = 1;
				}else if(UpOn == 1){
					if(spDLL->m_spIterator!=spDLL->m_spHead) UPnDOWN++;

					if((spDLL->m_spIterator == spDLL->m_spHead)){
					}
					
					else tmpData = IteratePrevList(spDLL, UPnDOWN);
				}

				while(iCommandBufferIndex!=0){
					kGetCursor( &iCursorX, &iCursorY );
					kPrintStringXY( iCursorX - 1, iCursorY, " " );
					kSetCursor( iCursorX - 1, iCursorY );
				vcCommandBuffer[--iCommandBufferIndex] = '\0';
				}

				int ind = 0;
				while(tmpData[ind]!=NULL){
					vcCommandBuffer[iCommandBufferIndex] = tmpData[ind];
					kPrintf("%c", tmpData[ind]);
					iCommandBufferIndex++;
					ind++;
				}
			}

			if(bKey == KEY_DOWN){
				UPnDOWN--;
				if(UPnDOWN<0) {
					UPnDOWN = 0;
					DownOn = 0;

				}
				if(DownOn == 1){
				if(iCommandBufferIndex > 0)
					tmpData = IterateNextList(spDLL,UPnDOWN);	
				else tmpData = '\0';	
				}
				while(iCommandBufferIndex!=0){
					kGetCursor( &iCursorX, &iCursorY );
					kPrintStringXY( iCursorX - 1, iCursorY, " " );
					kSetCursor( iCursorX - 1, iCursorY );
				vcCommandBuffer[--iCommandBufferIndex] = '\0';
				}
				int ind = 0;
				while(tmpData[ind]!=NULL){
					vcCommandBuffer[iCommandBufferIndex++] = tmpData[ind];
					kPrintf("%c", tmpData[ind]);
					ind++;
				}
			}
		}

		else
		{
			// ***********TAB*****************//
			if( bKey == KEY_TAB )
			{
				kExecuteTab(vcCommandBuffer, &iCommandBufferIndex, tabflag);
				tabflag = 1;
				continue;
			}

			// 버퍼에 공간이 남아있을 때만 가능
			if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
			{
				tabflag = 0;
				vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
				kPrintf( "%c", bKey );
			}
		}
	}
}


void kExecuteTab( char* pcCommandBuffer, int* iCommandBufferIndex, int tabflag )
{
	int iCount;					//전체 커맨드 갯수
	int iCommandLength;			//커맨드 하나의 길이
	char subString[100];		//커맨드substring

	int pcCommandBufferLength;	// 버퍼에 들어온 길이

	int i;	
	int j;
	int cnt = 0;
	int cmdIndex[100];
	char* pCommand;


	iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

	pcCommandBufferLength = kStrLen(pcCommandBuffer);


	for( i = 0 ; i < iCount ; i++ )
	{
		//buffer에 들어온 길이만큼 substring 해주기
		pCommand = gs_vstCommandTable[i].pcCommand;

		for(j = 0; j < pcCommandBufferLength; j++){
			subString[j] = pCommand[j];
		}
		// 커맨드의 내용이 일치하는지 검사
		if(( kMemCmp( subString, pcCommandBuffer, pcCommandBufferLength ) == 0 ) )	//null문자 확인해보기
		{
			cnt++;
			cmdIndex[i] = 1;
		}
		else
		{
			cmdIndex[i] = 0;
		}

	}

	if(cnt == 0)
	{
		return;
	}

	else if(cnt == 1)
	{
		//int i;
		for(i = 0; i < iCount; i++)
		{
			if(cmdIndex[i] == 1)	//몇번째 명령어인지 확인
			{
				break;
			}
		}

		pCommand = gs_vstCommandTable[i].pcCommand;
		iCommandLength = kStrLen(pCommand);

		//int j;
		for(j = pcCommandBufferLength; j < iCommandLength; j++)
		{
			if(*iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
			{
				pcCommandBuffer[ (int)(*iCommandBufferIndex)++ ] = pCommand[j];
				kPrintf( "%c", pCommand[j] );
			}
		}
	}
	//cnt > 1
	else{

		char cmpChar = '\0';
		int miss = 0;
		int pCommandIndex = pcCommandBufferLength;

		for(int idx = 0; idx < 10;idx++){
			cmpChar = '\0';
			for(i = 0; i < iCount; i++){

				pCommand = gs_vstCommandTable[i].pcCommand;

				if(cmdIndex[i] == 1){	//일치하는게 있을때

					if(cmpChar == '\0')	{
						cmpChar	= pCommand[pCommandIndex];
					}

					else{
						if(cmpChar == pCommand[pCommandIndex]){
						}
						else{
							miss = 1;
							break;
						}
					}

				}
			}
			if(!miss){
				pcCommandBuffer[ (int)(*iCommandBufferIndex)++ ] = cmpChar;
				kPrintf( "%c", cmpChar);
				miss = 0;
			}
			pCommandIndex++;
		}

		int tabcnt = 0;
		int line = 1;

		if(tabflag == 1){
			kPrintf("\n");
			for(i = 0;i < iCount; i++){
				if(cmdIndex[i] == 1){
					kPrintf("%s\t",gs_vstCommandTable[i].pcCommand);
					tabcnt++;

					if(cnt - (line*3) > 0 && ((tabcnt%3) == 0)){
						kPrintf("\n");
						line++;
					}				
				}
			}
			
			kPrintf("\n");
			kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE ); 
 			kPrintf("%s", pcCommandBuffer);

		}
	}
}

/*
 *  커맨드 버퍼에 있는 커맨드를 비교하여 해당 커맨드를 처리하는 함수를 수행
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // 공백으로 구분된 커맨드를 추출
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // 커맨드 테이블을 검사해서 동일한 이름의 커맨드가 있는지 확인
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // 커맨드의 길이와 내용이 완전히 일치하는지 검사
        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
            break;
        }
    }

    // 리스트에서 찾을 수 없다면 에러 출력
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
    }
}

/**
 *  파라미터 자료구조를 초기화
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  공백으로 구분된 파라미터의 내용과 길이를 반환
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // 더 이상 파라미터가 없으면 나감
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // 버퍼의 길이만큼 이동하면서 공백을 검색
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // 파라미터를 복사하고 길이를 반환
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // 파라미터의 위치 업데이트
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  커맨드를 처리하는 코드
//==============================================================================
/**
 *  셸 도움말을 출력
 */
static void kHelp( const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

    // 가장 긴 커맨드의 길이를 계산
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // 도움말 출력
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

/**
 *  화면을 지움 
 */
static void kCls( const char* pcParameterBuffer )
{
    // 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지운 후, 라인 1로 커서 이동
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  총 메모리 크기를 출력
 */
static void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}


/**
 *  문자열로 된 숫자를 숫자로 변환하여 화면에 출력
 */
static void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    // 파라미터 초기화
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {
        // 다음 파라미터를 구함, 파라미터의 길이가 0이면 파라미터가 없는 것이므로
	// 종료
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        // 파라미터에 대한 정보를 출력하고 16진수인지 10진수인지 판단하여 변환한 후
	// 결과를 printf로 출력
        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        // 0x로 시작하면 16진수, 그외는 10진수로 판단
        if( kMemCmp( vcParameter, "0x", 2 ) == 0 )
        {
            lValue = kAToI( vcParameter + 2, 16 );
            kPrintf( "HEX Value = %q\n", lValue );
        }
        else
        {
            lValue = kAToI( vcParameter, 10 );
            kPrintf( "Decimal Value = %d\n", lValue );
        }
        
        iCount++;
    }
}

/**
 *  PC를 재시작(Reboot)
 */
static void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // 키보드 컨트롤러를 통해 PC를 재시작
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

/**
 *  PIT 컨트롤러의 카운터 0 설정
 */
static void kSetTimer( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    // 파라미터 초기화
    kInitializeParameter( &stList, pcParameterBuffer );
    
    // milisecond 추출
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
        return ;
    }
    lValue = kAToI( vcParameter, 10 );

    // Periodic 추출
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
        return ;
    }    
    bPeriodic = kAToI( vcParameter, 10 );
    
    kInitializePIT( MSTOCOUNT( lValue ), bPeriodic );
    kPrintf( "Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic );
}

/**
 *  PIT 컨트롤러를 직접 사용하여 ms 동안 대기  
 */
static void kWaitUsingPIT( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;
    
    // 파라미터 초기화
    kInitializeParameter( &stList, pcParameterBuffer );
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)wait 100(ms)\n" );
        return ;
    }
    
    lMillisecond = kAToI( pcParameterBuffer, 10 );
    kPrintf( "%d ms Sleep Start...\n", lMillisecond );
    
    /// 인터럽트를 비활성화하고 PIT 컨트롤러를 통해 직접 시간을 측정
    kDisableInterrupt();
    for( i = 0 ; i < lMillisecond / 30 ; i++ )
    {
        kWaitUsingDirectPIT( MSTOCOUNT( 30 ) );
    }
    kWaitUsingDirectPIT( MSTOCOUNT( lMillisecond % 30 ) );   
    kEnableInterrupt();
    kPrintf( "%d ms Sleep Complete\n", lMillisecond );
    
    // 타이머 복원
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );
}

/**
 *  타임 스탬프 카운터를 읽음  
 */
static void kReadTimeStampCounter( const char* pcParameterBuffer )
{
    QWORD qwTSC;
    
    qwTSC = kReadTSC();
    kPrintf( "Time Stamp Counter = %q\n", qwTSC );
}

/**
 *  프로세서의 속도를 측정
 */
static void kMeasureProcessorSpeed( const char* pcParameterBuffer )
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    kPrintf( "Now Measuring." );
    
    // 10초 동안 변화한 타임 스탬프 카운터를 이용하여 프로세서의 속도를 간접적으로 측정
    kDisableInterrupt();    
    for( i = 0 ; i < 200 ; i++ )
    {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT( MSTOCOUNT( 50 ) );
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf( "." );
    }
    // 타이머 복원
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );    
    kEnableInterrupt();
    
    kPrintf( "\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000 );
}

/**
 *  RTC 컨트롤러에 저장된 일자 및 시간 정보를 표시
 */
static void kShowDateAndTime( const char* pcParameterBuffer )
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    // RTC 컨트롤러에서 시간 및 일자를 읽음
    kReadRTCTime( &bHour, &bMinute, &bSecond );
    kReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    kPrintf( "Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
             kConvertDayOfWeekToString( bDayOfWeek ) );
    kPrintf( "Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}

/**
 *  태스크 1
 *      화면 테두리를 돌면서 문자를 출력
 */
static void kTestTask1( void )
{
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin, j;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    
    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iMargin = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) % 10;
    
    // 화면 네 귀퉁이를 돌면서 문자 출력
    for( j = 0 ; j < 20000 ; j++ )
    {
        switch( i )
        {
        case 0:
            iX++;
            if( iX >= ( CONSOLE_WIDTH - iMargin ) )
            {
                i = 1;
            }
            break;
            
        case 1:
            iY++;
            if( iY >= ( CONSOLE_HEIGHT - iMargin ) )
            {
                i = 2;
            }
            break;
            
        case 2:
            iX--;
            if( iX < iMargin )
            {
                i = 3;
            }
            break;
            
        case 3:
            iY--;
            if( iY < iMargin )
            {
                i = 0;
            }
            break;
        }
        
        // 문자 및 색깔 지정
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bCharactor = bData;
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bAttribute = bData & 0x0F;
        bData++;
        
        // 다른 태스크로 전환
        //kSchedule();
    }

    //kExitTask();
}

/**
 *  태스크 2
 *      자신의 ID를 참고하여 특정 위치에 회전하는 바람개비를 출력
 */
void kTestTask2( void )
{
    int i = 0, iOffset;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    char vcData[ 4 ] = { '-', '\\', '|', '/' };
    
    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iOffset = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - 
        ( iOffset % ( CONSOLE_WIDTH * CONSOLE_HEIGHT ) );

    while( 1 )
    {
        // 회전하는 바람개비를 표시
        pstScreen[ iOffset ].bCharactor = vcData[ i % 4 ];
        // 색깔 지정
        pstScreen[ iOffset ].bAttribute = ( iOffset % 15 ) + 1;
        i++;
        
        // 다른 태스크로 전환
        //kSchedule();
    }
}

/**
 *  태스크를 생성해서 멀티 태스킹 수행
 */
static void kCreateTestTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcType[ 30 ];
    char vcCount[ 30 ];
    int i;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcType );
    kGetNextParameter( &stList, vcCount );

    switch( kAToI( vcType, 10 ) )
    {
    // 타입 1 태스크 생성
    case 1:
        for( i = 0 ; i < kAToI( vcCount, 10 ) ; i++ )
        {    
            if( kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask1 ) == NULL )
            {
                break;
            }
        }
        
        kPrintf( "Task1 %d Created\n", i );
        break;
    
    /**
	 * case 3:
     *     kCreateTask( TASK_FLAGS_HIGHEST | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 );
     *     kCreateTask( TASK_FLAGS_MEDIUM | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 );
     *     kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 );
	 * 	break;
	 */

    // 타입 2 태스크 생성
    case 2:
    default:
        for( i = 0 ; i < kAToI( vcCount, 10 ) ; i++ )
        {    
            if( kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 ) == NULL )
            {
                break;
            }
        }
        kPrintf( "Task2 %d Created\n", i );
        break;
    }    
}   

/**
 *  태스크의 우선 순위를 변경
 */
static void kChangeTaskPriority( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    char vcPriority[ 30 ];
    QWORD qwID;
    BYTE bPriority;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    kGetNextParameter( &stList, vcPriority );
    
    // 태스크의 우선 순위를 변경
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    bPriority = kAToI( vcPriority, 10 );
    
    kPrintf( "Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority );
    if( kChangePriority( qwID, bPriority ) == TRUE )
    {
        kPrintf( "Success\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
}
/**
 *  현재 생성된 모든 태스크의 정보를 출력
 */
static void kShowTaskList( const char* pcParameterBuffer )
{
    int i;
    TCB* pstTCB;
    int iCount = 0;
    
    kPrintf( "=========== Task Total Count [%d] ===========\n", kGetTaskCount() );
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        // TCB를 구해서 TCB가 사용 중이면 ID를 출력
        pstTCB = kGetTCBInTCBPool( i );
        if( ( pstTCB->stLink.qwID >> 32 ) != 0 )
        {
            // 태스크가 10개 출력될 때마다, 계속 태스크 정보를 표시할지 여부를 확인
            if( ( iCount != 0 ) && ( ( iCount % 10 ) == 0 ) )
            {
                kPrintf( "Press any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    kPrintf( "\n" );
                    break;
                }
                kPrintf( "\n" );
            }
            
            kPrintf( "[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n", 1 + iCount++,
                     pstTCB->stLink.qwID, GETPRIORITY( pstTCB->qwFlags ), 
                     pstTCB->qwFlags, kGetListCount( &( pstTCB->stChildThreadList ) ) );
            kPrintf( "    Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n",
                    pstTCB->qwParentProcessID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize );
            /*kPrintf( "    Ticket[0x%Q], Stride[0x%Q], Pass[0x%Q]\n",
                    pstTCB->qwTicket, pstTCB->qwStride, pstTCB->qwPass );
			*/
			kPrintf("     pass : %d, SwitchCount : %d, my ticket : %d\n", pstTCB->qwPass, pstTCB->qwSwitchCount, pstTCB->qwTicket);
        }
    }
}

/**
 *  태스크를 종료
 */
static void kKillTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    QWORD qwID;
    TCB* pstTCB;
    int i;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    
    // 태스크를 종료
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    // 특정 ID만 종료하는 경우
    if( qwID != 0xFFFFFFFF )
    {
        pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
        qwID = pstTCB->stLink.qwID;

        // 시스템 테스트는 제외
        if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
        {
            kPrintf( "Kill Task ID [0x%q] ", qwID );
            if( kEndTask( qwID ) == TRUE )
            {
                kPrintf( "Success\n" );
            }
            else
            {
                kPrintf( "Fail\n" );
            }
        }
        else
        {
            kPrintf( "Task does not exist or task is system task\n" );
        }
    }
    // 콘솔 셸과 유휴 태스크를 제외하고 모든 태스크 종료
    else
    {
        for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
        {
            pstTCB = kGetTCBInTCBPool( i );
            qwID = pstTCB->stLink.qwID;

            // 시스템 테스트는 삭제 목록에서 제외
            if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
            {
                kPrintf( "Kill Task ID [0x%q] ", qwID );
                if( kEndTask( qwID ) == TRUE )
                {
                    kPrintf( "Success\n" );
                }
                else
                {
                    kPrintf( "Fail\n" );
                }
            }
        }
    }
}

/**
 *  프로세서의 사용률을 표시
 */
static void kCPULoad( const char* pcParameterBuffer )
{
    kPrintf( "Processor Load : %d%%\n", kGetProcessorLoad() );
}
    
// 뮤텍스 테스트용 뮤텍스와 변수
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

/**
 *  뮤텍스를 테스트하는 태스크
 */
static void kPrintNumberTask( void )
{
    int i;
    int j;
    QWORD qwTickCount;

    // 50ms 정도 대기하여 콘솔 셸이 출력하는 메시지와 겹치지 않도록 함
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 50 )
    {
        kSchedule();
    }    
    
    // 루프를 돌면서 숫자를 출력
    for( i = 0 ; i < 5 ; i++ )
    {
        kLock( &( gs_stMutex ) );
        kPrintf( "Task ID [0x%Q] Value[%d]\n", kGetRunningTask()->stLink.qwID,
                gs_qwAdder );
        
        gs_qwAdder += 1;
        kUnlock( & ( gs_stMutex ) );
    
        // 프로세서 소모를 늘리려고 추가한 코드
        for( j = 0 ; j < 30000 ; j++ ) ;
    }
    
    // 모든 태스크가 종료할 때까지 1초(100ms) 정도 대기
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 1000 )
    {
        kSchedule();
    }    
    
    // 태스크 종료
    //kExitTask();
}

/**
 *  뮤텍스를 테스트하는 태스크 생성
 */
static void kTestMutex( const char* pcParameterBuffer )
{
    int i;
    
    gs_qwAdder = 1;
    
    // 뮤텍스 초기화
    kInitializeMutex( &gs_stMutex );
    
    for( i = 0 ; i < 3 ; i++ )
    {
        // 뮤텍스를 테스트하는 태스크를 3개 생성
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kPrintNumberTask );
    }    
    kPrintf( "Wait Util %d Task End...\n", i );
    kGetCh();
}

/**
 *  태스크 2를 자신의 스레드로 생성하는 태스크
 */
static void kCreateThreadTask( void )
{
    int i;
    
    for( i = 0 ; i < 3 ; i++ )
    {
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 );
    }
    
    while( 1 )
    {
        kSleep( 1 );
    }
}

/**
 *  스레드를 테스트하는 태스크 생성
 */
static void kTestThread( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, ( void * )0xEEEEEEEE, 0x1000, 
                              ( QWORD ) kCreateThreadTask );
    if( pstProcess != NULL )
    {
        kPrintf( "Process [0x%Q] Create Success\n", pstProcess->stLink.qwID ); 
    }
    else
    {
        kPrintf( "Process Create Fail\n" );
    }
}

// 난수를 발생시키기 위한 변수
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  임의의 난수를 반환
 */
QWORD kRandom( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

/**
 *  철자를 흘러내리게 하는 스레드
 */
static void kDropCharactorThread( void )
{
    int iX, iY;
    int i;
    char vcText[ 2 ] = { 0, };

    iX = kRandom() % CONSOLE_WIDTH;
    
    while( 1 )
    {
        // 잠시 대기함
        kSleep( kRandom() % 20 );
        
        if( ( kRandom() % 20 ) < 16 )
        {
            vcText[ 0 ] = ' ';
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                kPrintStringXY( iX, i , vcText );
                kSleep( 50 );
            }
        }        
        else
        {
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                vcText[ 0 ] = i + kRandom();
                kPrintStringXY( iX, i, vcText );
                kSleep( 50 );
            }
        }
    }
}

/**
 *  스레드를 생성하여 매트릭스 화면처럼 보여주는 프로세스
 */
static void kMatrixProcess( void )
{
    int i;
    
    for( i = 0 ; i < 300 ; i++ )
    {
        if( kCreateTask( TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0, 
                         ( QWORD ) kDropCharactorThread ) == NULL )
        {
            break;
        }
        
        kSleep( kRandom() % 5 + 5 );
    }
    
    kPrintf( "%d Thread is created\n", i );

    // 키가 입력되면 프로세스 종료
    kGetCh();
}

/**
 *  매트릭스 화면을 보여줌
 */
static void kShowMatrix( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, ( void* ) 0xE00000, 0xE00000, 
                              ( QWORD ) kMatrixProcess );
    if( pstProcess != NULL )
    {
        kPrintf( "Matrix Process [0x%Q] Create Success\n" );

        // 태스크가 종료 될 때까지 대기
        while( ( pstProcess->stLink.qwID >> 32 ) != 0 )
        {
            kSleep( 100 );
        }
    }
    else
    {
        kPrintf( "Matrix Process Create Fail\n" );
    }
}

/**
 *  FPU¸¦ Å×½ºÆ®ÇÏ´Â ÅÂ½ºÅ©
 */
static void kFPUTestTask( void )
{
    double dValue1;
    double dValue2;
    TCB* pstRunningTask;
    QWORD qwCount = 0;
    QWORD qwRandomValue;
    int i;
    int iOffset;
    char vcData[ 4 ] = { '-', '\\', '|', '/' };
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;

    pstRunningTask = kGetRunningTask();

    // ÀÚ½ÅÀÇ ID¸¦ ¾ò¾î¼­ È­¸é ¿ÀÇÁ¼ÂÀ¸·Î »ç¿ë
    iOffset = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - 
        ( iOffset % ( CONSOLE_WIDTH * CONSOLE_HEIGHT ) );

    // ·çÇÁ¸¦ ¹«ÇÑÈ÷ ¹Ýº¹ÇÏ¸é¼­ µ¿ÀÏÇÑ °è»êÀ» ¼öÇà
    while( 1 )
    {
        dValue1 = 1;
        dValue2 = 1;
        
        // Å×½ºÆ®¸¦ À§ÇØ µ¿ÀÏÇÑ °è»êÀ» 2¹ø ¹Ýº¹ÇØ¼­ ½ÇÇà
        for( i = 0 ; i < 10 ; i++ )
        {
            qwRandomValue = kRandom();
            dValue1 *= ( double ) qwRandomValue;
            dValue2 *= ( double ) qwRandomValue;

            kSleep( 1 );
            
            qwRandomValue = kRandom();
            dValue1 /= ( double ) qwRandomValue;
            dValue2 /= ( double ) qwRandomValue;
        }
        
        if( dValue1 != dValue2 )
        {
            kPrintf( "Value Is Not Same~!!! [%f] != [%f]\n", dValue1, dValue2 );
            break;
        }
        qwCount++;

        // È¸ÀüÇÏ´Â ¹Ù¶÷°³ºñ¸¦ Ç¥½Ã
        pstScreen[ iOffset ].bCharactor = vcData[ qwCount % 4 ];

        // »ö±ò ÁöÁ¤
        pstScreen[ iOffset ].bAttribute = ( iOffset % 15 ) + 1;
    }
}

/**
 *  ¿øÁÖÀ²(PIE)¸¦ °è»ê
 */
static void kTestPIE( const char* pcParameterBuffer )
{
    double dResult;
    int i;
    
    kPrintf( "PIE Cacluation Test\n" );
    kPrintf( "Result: 355 / 113 = " );
    dResult = ( double ) 355 / 113;
    kPrintf( "%d.%d%d\n", ( QWORD ) dResult, ( ( QWORD ) ( dResult * 10 ) % 10 ),
             ( ( QWORD ) ( dResult * 100 ) % 10 ) );
    
    // ½Ç¼ö¸¦ °è»êÇÏ´Â ÅÂ½ºÅ©¸¦ »ý¼º
    for( i = 0 ; i < 100 ; i++ )
    {
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kFPUTestTask );
    }
}

/**
 *  µ¿Àû ¸Þ¸ð¸® Á¤º¸¸¦ Ç¥½Ã
 */
static void kShowDyanmicMemoryInformation( const char* pcParameterBuffer )
{
    QWORD qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;
    
    kGetDynamicMemoryInformation( &qwStartAddress, &qwTotalSize, &qwMetaSize, 
            &qwUsedSize );

    kPrintf( "============ Dynamic Memory Information ============\n" );
    kPrintf( "Start Address: [0x%Q]\n", qwStartAddress );
    kPrintf( "Total Size:    [0x%Q]byte, [%d]MB\n", qwTotalSize, 
            qwTotalSize / 1024 / 1024 );
    kPrintf( "Meta Size:     [0x%Q]byte, [%d]KB\n", qwMetaSize, 
            qwMetaSize / 1024 );
    kPrintf( "Used Size:     [0x%Q]byte, [%d]KB\n", qwUsedSize, qwUsedSize / 1024 );
}

/**
 *  ¸ðµç ºí·Ï ¸®½ºÆ®ÀÇ ºí·ÏÀ» ¼øÂ÷ÀûÀ¸·Î ÇÒ´çÇÏ°í ÇØÁ¦ÇÏ´Â Å×½ºÆ®
 */
static void kTestSequentialAllocation( const char* pcParameterBuffer )
{
    DYNAMICMEMORY* pstMemory;
    long i, j, k;
    QWORD* pqwBuffer;
    
    kPrintf( "============ Dynamic Memory Test ============\n" );
    pstMemory = kGetDynamicMemoryManager();
    
    for( i = 0 ; i < pstMemory->iMaxLevelCount ; i++ )
    {
        kPrintf( "Block List [%d] Test Start\n", i );
        kPrintf( "Allocation And Compare: ");
        
        // ¸ðµç ºí·ÏÀ» ÇÒ´ç ¹Þ¾Æ¼­ °ªÀ» Ã¤¿î ÈÄ °Ë»ç
        for( j = 0 ; j < ( pstMemory->iBlockCountOfSmallestBlock >> i ) ; j++ )
        {
            pqwBuffer = kAllocateMemory( DYNAMICMEMORY_MIN_SIZE << i );
            if( pqwBuffer == NULL )
            {
                kPrintf( "\nAllocation Fail\n" );
                return ;
            }

            // °ªÀ» Ã¤¿î ÈÄ ´Ù½Ã °Ë»ç
            for( k = 0 ; k < ( DYNAMICMEMORY_MIN_SIZE << i ) / 8 ; k++ )
            {
                pqwBuffer[ k ] = k;
            }
            
            for( k = 0 ; k < ( DYNAMICMEMORY_MIN_SIZE << i ) / 8 ; k++ )
            {
                if( pqwBuffer[ k ] != k )
                {
                    kPrintf( "\nCompare Fail\n" );
                    return ;
                }
            }
            // ÁøÇà °úÁ¤À» . À¸·Î Ç¥½Ã
            kPrintf( "." );
        }
        
        kPrintf( "\nFree: ");
        // ÇÒ´ç ¹ÞÀº ºí·ÏÀ» ¸ðµÎ ¹ÝÈ¯
        for( j = 0 ; j < ( pstMemory->iBlockCountOfSmallestBlock >> i ) ; j++ )
        {
            if( kFreeMemory( ( void * ) ( pstMemory->qwStartAddress + 
                         ( DYNAMICMEMORY_MIN_SIZE << i ) * j ) ) == FALSE )
            {
                kPrintf( "\nFree Fail\n" );
                return ;
            }
            // ÁøÇà °úÁ¤À» . À¸·Î Ç¥½Ã
            kPrintf( "." );
        }
        kPrintf( "\n" );
    }
    kPrintf( "Test Complete~!!!\n" );
}

/**
 *  ÀÓÀÇ·Î ¸Þ¸ð¸®¸¦ ÇÒ´çÇÏ°í ÇØÁ¦ÇÏ´Â °ÍÀ» ¹Ýº¹ÇÏ´Â ÅÂ½ºÅ©
 */
static void kRandomAllocationTask( void )
{
    TCB* pstTask;
    QWORD qwMemorySize;
    char vcBuffer[ 200 ];
    BYTE* pbAllocationBuffer;
    int i, j;
    int iY;
    
    pstTask = kGetRunningTask();
    iY = ( pstTask->stLink.qwID ) % 15 + 9;

    for( j = 0 ; j < 10 ; j++ )
    {
        // 1KB ~ 32M±îÁö ÇÒ´çÇÏµµ·Ï ÇÔ
        do
        {
            qwMemorySize = ( ( kRandom() % ( 32 * 1024 ) ) + 1 ) * 1024;
            pbAllocationBuffer = kAllocateMemory( qwMemorySize );

            // ¸¸ÀÏ ¹öÆÛ¸¦ ÇÒ´ç ¹ÞÁö ¸øÇÏ¸é ´Ù¸¥ ÅÂ½ºÅ©°¡ ¸Þ¸ð¸®¸¦ »ç¿ëÇÏ°í 
            // ÀÖÀ» ¼ö ÀÖÀ¸¹Ç·Î Àá½Ã ´ë±âÇÑ ÈÄ ´Ù½Ã ½Ãµµ
            if( pbAllocationBuffer == 0 )
            {
                kSleep( 1 );
            }
        } while( pbAllocationBuffer == 0 );
            
        kSPrintf( vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Allocation Success", 
                  pbAllocationBuffer, qwMemorySize );
        // ÀÚ½ÅÀÇ ID¸¦ Y ÁÂÇ¥·Î ÇÏ¿© µ¥ÀÌÅÍ¸¦ Ãâ·Â
        kPrintStringXY( 20, iY, vcBuffer );
        kSleep( 200 );
        
        // ¹öÆÛ¸¦ ¹ÝÀ¸·Î ³ª´²¼­ ·£´ýÇÑ µ¥ÀÌÅÍ¸¦ ¶È°°ÀÌ Ã¤¿ò 
        kSPrintf( vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Write...     ", 
                  pbAllocationBuffer, qwMemorySize );
        kPrintStringXY( 20, iY, vcBuffer );
        for( i = 0 ; i < qwMemorySize / 2 ; i++ )
        {
            pbAllocationBuffer[ i ] = kRandom() & 0xFF;
            pbAllocationBuffer[ i + ( qwMemorySize / 2 ) ] = pbAllocationBuffer[ i ];
        }
        kSleep( 200 );
        
        // Ã¤¿î µ¥ÀÌÅÍ°¡ Á¤»óÀûÀÎÁö ´Ù½Ã È®ÀÎ
        kSPrintf( vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Verify...   ", 
                  pbAllocationBuffer, qwMemorySize );
        kPrintStringXY( 20, iY, vcBuffer );
        for( i = 0 ; i < qwMemorySize / 2 ; i++ )
        {
            if( pbAllocationBuffer[ i ] != pbAllocationBuffer[ i + ( qwMemorySize / 2 ) ] )
            {
                kPrintf( "Task ID[0x%Q] Verify Fail\n", pstTask->stLink.qwID );
                kExitTask();
            }
        }
        kFreeMemory( pbAllocationBuffer );
        kSleep( 200 );
    }
    
    kExitTask();
}

/**
 *  ÅÂ½ºÅ©¸¦ ¿©·¯ °³ »ý¼ºÇÏ¿© ÀÓÀÇÀÇ ¸Þ¸ð¸®¸¦ ÇÒ´çÇÏ°í ÇØÁ¦ÇÏ´Â °ÍÀ» ¹Ýº¹ÇÏ´Â Å×½ºÆ®
 */
static void kTestRandomAllocation( const char* pcParameterBuffer )
{
    int i;
    
    for( i = 0 ; i < 1000 ; i++ )
    {
        kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kRandomAllocationTask );
    }
}

/**
 *  ÇÏµå µð½ºÅ©ÀÇ Á¤º¸¸¦ Ç¥½Ã
 */
static void kShowHDDInformation( const char* pcParameterBuffer )
{
    HDDINFORMATION stHDD;
    char vcBuffer[ 100 ];
    
    // ÇÏµå µð½ºÅ©ÀÇ Á¤º¸¸¦ ÀÐÀ½
    if( kGetHDDInformation( &stHDD ) == FALSE )
    {
        kPrintf( "HDD Information Read Fail\n" );
        return ;
    }        
    
    kPrintf( "============ Primary Master HDD Information ============\n" );
    
    // ¸ðµ¨ ¹øÈ£ Ãâ·Â
    kMemCpy( vcBuffer, stHDD.vwModelNumber, sizeof( stHDD.vwModelNumber ) );
    vcBuffer[ sizeof( stHDD.vwModelNumber ) - 1 ] = '\0';
    kPrintf( "Model Number:\t %s\n", vcBuffer );
    
    // ½Ã¸®¾ó ¹øÈ£ Ãâ·Â
    kMemCpy( vcBuffer, stHDD.vwSerialNumber, sizeof( stHDD.vwSerialNumber ) );
    vcBuffer[ sizeof( stHDD.vwSerialNumber ) - 1 ] = '\0';
    kPrintf( "Serial Number:\t %s\n", vcBuffer );

    // Çìµå, ½Ç¸°´õ, ½Ç¸°´õ ´ç ¼½ÅÍ ¼ö¸¦ Ãâ·Â
    kPrintf( "Head Count:\t %d\n", stHDD.wNumberOfHead );
    kPrintf( "Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder );
    kPrintf( "Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder );
    
    // ÃÑ ¼½ÅÍ ¼ö Ãâ·Â
    kPrintf( "Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSectors, 
            stHDD.dwTotalSectors / 2 / 1024 );
}

/**
 *  ÇÏµå µð½ºÅ©¿¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿Â LBA ¾îµå·¹½º¿¡¼­ ¼½ÅÍ ¼ö ¸¸Å­ ÀÐÀ½
 */
static void kReadSector( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcLBA[ 50 ], vcSectorCount[ 50 ];
    DWORD dwLBA;
    int iSectorCount;
    char* pcBuffer;
    int i, j;
    BYTE bData;
    BOOL bExit = FALSE;
    
    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© LBA ¾îµå·¹½º¿Í ¼½ÅÍ ¼ö ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    if( ( kGetNextParameter( &stList, vcLBA ) == 0 ) ||
        ( kGetNextParameter( &stList, vcSectorCount ) == 0 ) )
    {
        kPrintf( "ex) readsector 0(LBA) 10(count)\n" );
        return ;
    }
    dwLBA = kAToI( vcLBA, 10 );
    iSectorCount = kAToI( vcSectorCount, 10 );
    
    // ¼½ÅÍ ¼ö¸¸Å­ ¸Þ¸ð¸®¸¦ ÇÒ´ç ¹Þ¾Æ ÀÐ±â ¼öÇà
    pcBuffer = kAllocateMemory( iSectorCount * 512 );
    if( kReadHDDSector( TRUE, TRUE, dwLBA, iSectorCount, pcBuffer ) == iSectorCount )
    {
        kPrintf( "LBA [%d], [%d] Sector Read Success~!!", dwLBA, iSectorCount );
        // µ¥ÀÌÅÍ ¹öÆÛÀÇ ³»¿ëÀ» Ãâ·Â
        for( j = 0 ; j < iSectorCount ; j++ )
        {
            for( i = 0 ; i < 512 ; i++ )
            {
                if( !( ( j == 0 ) && ( i == 0 ) ) && ( ( i % 256 ) == 0 ) )
                {
                    kPrintf( "\nPress any key to continue... ('q' is exit) : " );
                    if( kGetCh() == 'q' )
                    {
                        bExit = TRUE;
                        break;
                    }
                }                

                if( ( i % 16 ) == 0 )
                {
                    kPrintf( "\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i ); 
                }

                // ¸ðµÎ µÎ ÀÚ¸®·Î Ç¥½ÃÇÏ·Á°í 16º¸´Ù ÀÛÀº °æ¿ì 0À» Ãß°¡ÇØÁÜ
                bData = pcBuffer[ j * 512 + i ] & 0xFF;
                if( bData < 16 )
                {
                    kPrintf( "0" );
                }
                kPrintf( "%X ", bData );
            }
            
            if( bExit == TRUE )
            {
                break;
            }
        }
        kPrintf( "\n" );
    }
    else
    {
        kPrintf( "Read Fail\n" );
    }
    
    kFreeMemory( pcBuffer );
}

/**
 *  ÇÏµå µð½ºÅ©¿¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿Â LBA ¾îµå·¹½º¿¡¼­ ¼½ÅÍ ¼ö ¸¸Å­ ¾¸
 */
static void kWriteSector( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcLBA[ 50 ], vcSectorCount[ 50 ];
    DWORD dwLBA;
    int iSectorCount;
    char* pcBuffer;
    int i, j;
    BOOL bExit = FALSE;
    BYTE bData;
    static DWORD s_dwWriteCount = 0;

    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© LBA ¾îµå·¹½º¿Í ¼½ÅÍ ¼ö ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    if( ( kGetNextParameter( &stList, vcLBA ) == 0 ) ||
        ( kGetNextParameter( &stList, vcSectorCount ) == 0 ) )
    {
        kPrintf( "ex) writesector 0(LBA) 10(count)\n" );
        return ;
    }
    dwLBA = kAToI( vcLBA, 10 );
    iSectorCount = kAToI( vcSectorCount, 10 );

    s_dwWriteCount++;
    
    // ¹öÆÛ¸¦ ÇÒ´ç ¹Þ¾Æ µ¥ÀÌÅÍ¸¦ Ã¤¿ò. 
    // ÆÐÅÏÀº 4 ¹ÙÀÌÆ®ÀÇ LBA ¾îµå·¹½º¿Í 4 ¹ÙÀÌÆ®ÀÇ ¾²±â°¡ ¼öÇàµÈ È½¼ö·Î »ý¼º
    pcBuffer = kAllocateMemory( iSectorCount * 512 );
    for( j = 0 ; j < iSectorCount ; j++ )
    {
        for( i = 0 ; i < 512 ; i += 8 )
        {
            *( DWORD* ) &( pcBuffer[ j * 512 + i ] ) = dwLBA + j;
            *( DWORD* ) &( pcBuffer[ j * 512 + i + 4 ] ) = s_dwWriteCount;            
        }
    }
    
    // ¾²±â ¼öÇà
    if( kWriteHDDSector( TRUE, TRUE, dwLBA, iSectorCount, pcBuffer ) != iSectorCount )
    {
        kPrintf( "Write Fail\n" );
        return ;
    }
    kPrintf( "LBA [%d], [%d] Sector Write Success~!!", dwLBA, iSectorCount );

    // µ¥ÀÌÅÍ ¹öÆÛÀÇ ³»¿ëÀ» Ãâ·Â
    for( j = 0 ; j < iSectorCount ; j++ )
    {
        for( i = 0 ; i < 512 ; i++ )
        {
            if( !( ( j == 0 ) && ( i == 0 ) ) && ( ( i % 256 ) == 0 ) )
            {
                kPrintf( "\nPress any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    bExit = TRUE;
                    break;
                }
            }                

            if( ( i % 16 ) == 0 )
            {
                kPrintf( "\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i ); 
            }

            // ¸ðµÎ µÎ ÀÚ¸®·Î Ç¥½ÃÇÏ·Á°í 16º¸´Ù ÀÛÀº °æ¿ì 0À» Ãß°¡ÇØÁÜ
            bData = pcBuffer[ j * 512 + i ] & 0xFF;
            if( bData < 16 )
            {
                kPrintf( "0" );
            }
            kPrintf( "%X ", bData );
        }
        
        if( bExit == TRUE )
        {
            break;
        }
    }
    kPrintf( "\n" );    
    kFreeMemory( pcBuffer );    
}

/**
 *  ÇÏµå µð½ºÅ©¸¦ ¿¬°á
 */
static void kMountHDD( const char* pcParameterBuffer )
{
    if( kMount() == FALSE )
    {
        kPrintf( "HDD Mount Fail\n" );
        return ;
    }
    kPrintf( "HDD Mount Success\n" );
}

/**
 *  ÇÏµå µð½ºÅ©¿¡ ÆÄÀÏ ½Ã½ºÅÛÀ» »ý¼º(Æ÷¸Ë)
 */
static void kFormatHDD( const char* pcParameterBuffer )
{
    if( kFormat() == FALSE )
    {
        kPrintf( "HDD Format Fail\n" );
        return ;
    }
    kPrintf( "HDD Format Success\n" );
}

/**
 *  ÆÄÀÏ ½Ã½ºÅÛ Á¤º¸¸¦ Ç¥½Ã
 */
static void kShowFileSystemInformation( const char* pcParameterBuffer )
{
    FILESYSTEMMANAGER stManager;
    
    kGetFileSystemInformation( &stManager );
    
    kPrintf( "================== File System Information ==================\n" );
    kPrintf( "Mouted:\t\t\t\t\t %d\n", stManager.bMounted );
    kPrintf( "Reserved Sector Count:\t\t\t %d Sector\n", stManager.dwReservedSectorCount );
    kPrintf( "Cluster Link Table Start Address:\t %d Sector\n", 
            stManager.dwClusterLinkAreaStartAddress );
    kPrintf( "Cluster Link Table Size:\t\t %d Sector\n", stManager.dwClusterLinkAreaSize );
    kPrintf( "Data Area Start Address:\t\t %d Sector\n", stManager.dwDataAreaStartAddress );
    kPrintf( "Total Cluster Count:\t\t\t %d Cluster\n", stManager.dwTotalClusterCount );
}

/**
 *  ·çÆ® µð·ºÅÍ¸®¿¡ ºó ÆÄÀÏÀ» »ý¼º
 */
static void kCreateFileInRootDirectory( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    DWORD dwCluster;
    int i;
    FILE* pstFile;
    
    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© ÆÄÀÏ ÀÌ¸§À» ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }

    pstFile = fopen( vcFileName, "w" );
    if( pstFile == NULL )
    {
        kPrintf( "File Create Fail\n" );
        return;
    }
    fclose( pstFile );
    kPrintf( "File Create Success\n" );
}

/**
 *  ·çÆ® µð·ºÅÍ¸®¿¡¼­ ÆÄÀÏÀ» »èÁ¦
 */
static void kDeleteFileInRootDirectory( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    
    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© ÆÄÀÏ ÀÌ¸§À» ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }

    if( remove( vcFileName ) != 0 )
    {
        kPrintf( "File Not Found or File Opened\n" );
        return ;
    }
    
    kPrintf( "File Delete Success\n" );
}

/**
 *  ·çÆ® µð·ºÅÍ¸®ÀÇ ÆÄÀÏ ¸ñ·ÏÀ» Ç¥½Ã
 */
static void kShowRootDirectory( const char* pcParameterBuffer )
{
    DIR* pstDirectory;
    int i, iCount, iTotalCount;
    struct dirent* pstEntry;
    char vcBuffer[ 400 ];
    char vcTempValue[ 50 ];
    DWORD dwTotalByte;
    DWORD dwUsedClusterCount;
    FILESYSTEMMANAGER stManager;
    
    // ÆÄÀÏ ½Ã½ºÅÛ Á¤º¸¸¦ ¾òÀ½
    kGetFileSystemInformation( &stManager );
     
    // ·çÆ® µð·ºÅÍ¸®¸¦ ¿®
    pstDirectory = opendir( "/" );
    if( pstDirectory == NULL )
    {
        kPrintf( "Root Directory Open Fail\n" );
        return ;
    }
    
    // ¸ÕÀú ·çÇÁ¸¦ µ¹¸é¼­ µð·ºÅÍ¸®¿¡ ÀÖ´Â ÆÄÀÏÀÇ °³¼ö¿Í ÀüÃ¼ ÆÄÀÏÀÌ »ç¿ëÇÑ Å©±â¸¦ °è»ê
    iTotalCount = 0;
    dwTotalByte = 0;
    dwUsedClusterCount = 0;
    while( 1 )
    {
        // µð·ºÅÍ¸®¿¡¼­ ¿£Æ®¸® ÇÏ³ª¸¦ ÀÐÀ½
        pstEntry = readdir( pstDirectory );
        // ´õÀÌ»ó ÆÄÀÏÀÌ ¾øÀ¸¸é ³ª°¨
        if( pstEntry == NULL )
        {
            break;
        }
        iTotalCount++;
        dwTotalByte += pstEntry->dwFileSize;

        // ½ÇÁ¦·Î »ç¿ëµÈ Å¬·¯½ºÅÍÀÇ °³¼ö¸¦ °è»ê
        if( pstEntry->dwFileSize == 0 )
        {
            // Å©±â°¡ 0ÀÌ¶óµµ Å¬·¯½ºÅÍ 1°³´Â ÇÒ´çµÇ¾î ÀÖÀ½
            dwUsedClusterCount++;
        }
        else
        {
            // Å¬·¯½ºÅÍ °³¼ö¸¦ ¿Ã¸²ÇÏ¿© ´õÇÔ
            dwUsedClusterCount += ( pstEntry->dwFileSize + 
                ( FILESYSTEM_CLUSTERSIZE - 1 ) ) / FILESYSTEM_CLUSTERSIZE;
        }
    }
    
    // ½ÇÁ¦ ÆÄÀÏÀÇ ³»¿ëÀ» Ç¥½ÃÇÏ´Â ·çÇÁ
    rewinddir( pstDirectory );
    iCount = 0;
    while( 1 )
    {
        // µð·ºÅÍ¸®¿¡¼­ ¿£Æ®¸® ÇÏ³ª¸¦ ÀÐÀ½
        pstEntry = readdir( pstDirectory );
        // ´õÀÌ»ó ÆÄÀÏÀÌ ¾øÀ¸¸é ³ª°¨
        if( pstEntry == NULL )
        {
            break;
        }
        
        // ÀüºÎ °ø¹éÀ¸·Î ÃÊ±âÈ­ ÇÑ ÈÄ °¢ À§Ä¡¿¡ °ªÀ» ´ëÀÔ
        kMemSet( vcBuffer, ' ', sizeof( vcBuffer ) - 1 );
        vcBuffer[ sizeof( vcBuffer ) - 1 ] = '\0';
        
        // ÆÄÀÏ ÀÌ¸§ »ðÀÔ
        kMemCpy( vcBuffer, pstEntry->d_name, 
                 kStrLen( pstEntry->d_name ) );

        // ÆÄÀÏ ±æÀÌ »ðÀÔ
        kSPrintf( vcTempValue, "%d Byte", pstEntry->dwFileSize );
        kMemCpy( vcBuffer + 30, vcTempValue, kStrLen( vcTempValue ) );

        // ÆÄÀÏÀÇ ½ÃÀÛ Å¬·¯½ºÅÍ »ðÀÔ
        kSPrintf( vcTempValue, "0x%X Cluster", pstEntry->dwStartClusterIndex );
        kMemCpy( vcBuffer + 55, vcTempValue, kStrLen( vcTempValue ) + 1 );
        kPrintf( "    %s\n", vcBuffer );

        if( ( iCount != 0 ) && ( ( iCount % 20 ) == 0 ) )
        {
            kPrintf( "Press any key to continue... ('q' is exit) : " );
            if( kGetCh() == 'q' )
            {
                kPrintf( "\n" );
                break;
            }        
        }
        iCount++;
    }
    
    // ÃÑ ÆÄÀÏÀÇ °³¼ö¿Í ÆÄÀÏÀÇ ÃÑ Å©±â¸¦ Ãâ·Â
    kPrintf( "\t\tTotal File Count: %d\n", iTotalCount );
    kPrintf( "\t\tTotal File Size: %d KByte (%d Cluster)\n", dwTotalByte, 
             dwUsedClusterCount );
    
    // ³²Àº Å¬·¯½ºÅÍ ¼ö¸¦ ÀÌ¿ëÇØ¼­ ¿©À¯ °ø°£À» Ãâ·Â
    kPrintf( "\t\tFree Space: %d KByte (%d Cluster)\n", 
             ( stManager.dwTotalClusterCount - dwUsedClusterCount ) * 
             FILESYSTEM_CLUSTERSIZE / 1024, stManager.dwTotalClusterCount - 
             dwUsedClusterCount );
    
    // µð·ºÅÍ¸®¸¦ ´ÝÀ½
    closedir( pstDirectory );
}

/**
 *  ÆÄÀÏÀ» »ý¼ºÇÏ¿© Å°º¸µå·Î ÀÔ·ÂµÈ µ¥ÀÌÅÍ¸¦ ¾¸
 */
static void kWriteDataToFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    FILE* fp;
    int iEnterCount;
    BYTE bKey;
    
    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© ÆÄÀÏ ÀÌ¸§À» ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }
    
    // ÆÄÀÏ »ý¼º
    fp = fopen( vcFileName, "w" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", vcFileName );
        return ;
    }
    
    // ¿£ÅÍ Å°°¡ ¿¬¼ÓÀ¸·Î 3¹ø ´­·¯Áú ¶§±îÁö ³»¿ëÀ» ÆÄÀÏ¿¡ ¾¸
    iEnterCount = 0;
    while( 1 )
    {
        bKey = kGetCh();
        // ¿£ÅÍ Å°ÀÌ¸é ¿¬¼Ó 3¹ø ´­·¯Á³´Â°¡ È®ÀÎÇÏ¿© ·çÇÁ¸¦ ºüÁ® ³ª°¨
        if( bKey == KEY_ENTER )
        {
            iEnterCount++;
            if( iEnterCount >= 3 )
            {
                break;
            }
        }
        // ¿£ÅÍ Å°°¡ ¾Æ´Ï¶ó¸é ¿£ÅÍ Å° ÀÔ·Â È½¼ö¸¦ ÃÊ±âÈ­
        else
        {
            iEnterCount = 0;
        }
        
        kPrintf( "%c", bKey );
        if( fwrite( &bKey, 1, 1, fp ) != 1 )
        {
            kPrintf( "File Wirte Fail\n" );
            break;
        }
    }
    
    kPrintf( "File Create Success\n" );
    fclose( fp );
}

/**
 *  ÆÄÀÏÀ» ¿­¾î¼­ µ¥ÀÌÅÍ¸¦ ÀÐÀ½
 */
static void kReadDataFromFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    FILE* fp;
    int iEnterCount;
    BYTE bKey;
    
    // ÆÄ¶ó¹ÌÅÍ ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ¿© ÆÄÀÏ ÀÌ¸§À» ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }
    
    // ÆÄÀÏ »ý¼º
    fp = fopen( vcFileName, "r" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", vcFileName );
        return ;
    }
    
    // ÆÄÀÏÀÇ ³¡±îÁö Ãâ·ÂÇÏ´Â °ÍÀ» ¹Ýº¹
    iEnterCount = 0;
    while( 1 )
    {
        if( fread( &bKey, 1, 1, fp ) != 1 )
        {
            break;
        }
        kPrintf( "%c", bKey );
        
        // ¸¸¾à ¿£ÅÍ Å°ÀÌ¸é ¿£ÅÍ Å° È½¼ö¸¦ Áõ°¡½ÃÅ°°í 20¶óÀÎ±îÁö Ãâ·ÂÇß´Ù¸é 
        // ´õ Ãâ·ÂÇÒÁö ¿©ºÎ¸¦ ¹°¾îº½
        if( bKey == KEY_ENTER )
        {
            iEnterCount++;
            
            if( ( iEnterCount != 0 ) && ( ( iEnterCount % 20 ) == 0 ) )
            {
                kPrintf( "Press any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    kPrintf( "\n" );
                    break;
                }
                kPrintf( "\n" );
                iEnterCount = 0;
            }
        }
    }
    fclose( fp );
}

/**
 *  ÆÄÀÏ I/O¿¡ °ü·ÃµÈ ±â´ÉÀ» Å×½ºÆ®
 */
static void kTestFileIO( const char* pcParameterBuffer )
{
    FILE* pstFile;
    BYTE* pbBuffer;
    int i;
    int j;
    DWORD dwRandomOffset;
    DWORD dwByteCount;
    BYTE vbTempBuffer[ 1024 ];
    DWORD dwMaxFileSize;
    
    kPrintf( "================== File I/O Function Test ==================\n" );
    
    // 4MbyteÀÇ ¹öÆÛ ÇÒ´ç
    dwMaxFileSize = 4 * 1024 * 1024;
    pbBuffer = kAllocateMemory( dwMaxFileSize );
    if( pbBuffer == NULL )
    {
        kPrintf( "Memory Allocation Fail\n" );
        return ;
    }
    // Å×½ºÆ®¿ë ÆÄÀÏÀ» »èÁ¦
    remove( "testfileio.bin" );

    //==========================================================================
    // ÆÄÀÏ ¿­±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "1. File Open Fail Test..." );
    // r ¿É¼ÇÀº ÆÄÀÏÀ» »ý¼ºÇÏÁö ¾ÊÀ¸¹Ç·Î, Å×½ºÆ® ÆÄÀÏÀÌ ¾ø´Â °æ¿ì NULLÀÌ µÇ¾î¾ß ÇÔ
    pstFile = fopen( "testfileio.bin", "r" );
    if( pstFile == NULL )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
        fclose( pstFile );
    }
    
    //==========================================================================
    // ÆÄÀÏ »ý¼º Å×½ºÆ®
    //==========================================================================
    kPrintf( "2. File Create Test..." );
    // w ¿É¼ÇÀº ÆÄÀÏÀ» »ý¼ºÇÏ¹Ç·Î, Á¤»óÀûÀ¸·Î ÇÚµéÀÌ ¹ÝÈ¯µÇ¾î¾ßÇÔ
    pstFile = fopen( "testfileio.bin", "w" );
    if( pstFile != NULL )
    {
        kPrintf( "[Pass]\n" );
        kPrintf( "    File Handle [0x%Q]\n", pstFile );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    
    //==========================================================================
    // ¼øÂ÷ÀûÀÎ ¿µ¿ª ¾²±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "3. Sequential Write Test(Cluster Size)..." );
    // ¿­¸° ÇÚµéÀ» °¡Áö°í ¾²±â ¼öÇà
    for( i = 0 ; i < 100 ; i++ )
    {
        kMemSet( pbBuffer, i, FILESYSTEM_CLUSTERSIZE );
        if( fwrite( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) !=
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    %d Cluster Error\n", i );
            break;
        }
    }
    if( i >= 100 )
    {
        kPrintf( "[Pass]\n" );
    }
    
    //==========================================================================
    // ¼øÂ÷ÀûÀÎ ¿µ¿ª ÀÐ±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "4. Sequential Read And Verify Test(Cluster Size)..." );
    // ÆÄÀÏÀÇ Ã³À½À¸·Î ÀÌµ¿
    fseek( pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_END );
    
    // ¿­¸° ÇÚµéÀ» °¡Áö°í ÀÐ±â ¼öÇà ÈÄ, µ¥ÀÌÅÍ °ËÁõ
    for( i = 0 ; i < 100 ; i++ )
    {
        // ÆÄÀÏÀ» ÀÐÀ½
        if( fread( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) !=
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }
        
        // µ¥ÀÌÅÍ °Ë»ç
        for( j = 0 ; j < FILESYSTEM_CLUSTERSIZE ; j++ )
        {
            if( pbBuffer[ j ] != ( BYTE ) i )
            {
                kPrintf( "[Fail]\n" );
                kPrintf( "    %d Cluster Error. [%X] != [%X]\n", i, pbBuffer[ j ], 
                         ( BYTE ) i );
                break;
            }
        }
    }
    if( i >= 100 )
    {
        kPrintf( "[Pass]\n" );
    }

    //==========================================================================
    // ÀÓÀÇÀÇ ¿µ¿ª ¾²±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "5. Random Write Test...\n" );
    
    // ¹öÆÛ¸¦ ¸ðµÎ 0À¸·Î Ã¤¿ò
    kMemSet( pbBuffer, 0, dwMaxFileSize );
    // ¿©±â Àú±â¿¡ ¿Å°Ü´Ù´Ï¸é¼­ µ¥ÀÌÅÍ¸¦ ¾²°í °ËÁõ
    // ÆÄÀÏÀÇ ³»¿ëÀ» ÀÐ¾î¼­ ¹öÆÛ·Î º¹»ç
    fseek( pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_CUR );
    fread( pbBuffer, 1, dwMaxFileSize, pstFile );
    
    // ÀÓÀÇÀÇ À§Ä¡·Î ¿Å±â¸é¼­ µ¥ÀÌÅÍ¸¦ ÆÄÀÏ°ú ¹öÆÛ¿¡ µ¿½Ã¿¡ ¾¸
    for( i = 0 ; i < 100 ; i++ )
    {
        dwByteCount = ( kRandom() % ( sizeof( vbTempBuffer ) - 1 ) ) + 1;
        dwRandomOffset = kRandom() % ( dwMaxFileSize - dwByteCount );
        
        kPrintf( "    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset, 
                dwByteCount );

        // ÆÄÀÏ Æ÷ÀÎÅÍ¸¦ ÀÌµ¿
        fseek( pstFile, dwRandomOffset, SEEK_SET );
        kMemSet( vbTempBuffer, i, dwByteCount );
              
        // µ¥ÀÌÅÍ¸¦ ¾¸
        if( fwrite( vbTempBuffer, 1, dwByteCount, pstFile ) != dwByteCount )
        {
            kPrintf( "[Fail]\n" );
            break;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }
        
        kMemSet( pbBuffer + dwRandomOffset, i, dwByteCount );
    }
    
    // ¸Ç ¸¶Áö¸·À¸·Î ÀÌµ¿ÇÏ¿© 1¹ÙÀÌÆ®¸¦ ½á¼­ ÆÄÀÏÀÇ Å©±â¸¦ 4Mbyte·Î ¸¸µê
    fseek( pstFile, dwMaxFileSize - 1, SEEK_SET );
    fwrite( &i, 1, 1, pstFile );
    pbBuffer[ dwMaxFileSize - 1 ] = ( BYTE ) i;

    //==========================================================================
    // ÀÓÀÇÀÇ ¿µ¿ª ÀÐ±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "6. Random Read And Verify Test...\n" );
    // ÀÓÀÇÀÇ À§Ä¡·Î ¿Å±â¸é¼­ ÆÄÀÏ¿¡¼­ µ¥ÀÌÅÍ¸¦ ÀÐ¾î ¹öÆÛÀÇ ³»¿ë°ú ºñ±³
    for( i = 0 ; i < 100 ; i++ )
    {
        dwByteCount = ( kRandom() % ( sizeof( vbTempBuffer ) - 1 ) ) + 1;
        dwRandomOffset = kRandom() % ( ( dwMaxFileSize ) - dwByteCount );

        kPrintf( "    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset, 
                dwByteCount );
        
        // ÆÄÀÏ Æ÷ÀÎÅÍ¸¦ ÀÌµ¿
        fseek( pstFile, dwRandomOffset, SEEK_SET );
        
        // µ¥ÀÌÅÍ ÀÐÀ½
        if( fread( vbTempBuffer, 1, dwByteCount, pstFile ) != dwByteCount )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    Read Fail\n", dwRandomOffset ); 
            break;
        }
        
        // ¹öÆÛ¿Í ºñ±³
        if( kMemCmp( pbBuffer + dwRandomOffset, vbTempBuffer, dwByteCount ) 
                != 0 )
        {
            kPrintf( "[Fail]\n" );
            kPrintf( "    Compare Fail\n", dwRandomOffset ); 
            break;
        }
        
        kPrintf( "[Pass]\n" );
    }
    
    //==========================================================================
    // ´Ù½Ã ¼øÂ÷ÀûÀÎ ¿µ¿ª ÀÐ±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "7. Sequential Write, Read And Verify Test(1024 Byte)...\n" );
    // ÆÄÀÏÀÇ Ã³À½À¸·Î ÀÌµ¿
    fseek( pstFile, -dwMaxFileSize, SEEK_CUR );
    
    // ¿­¸° ÇÚµéÀ» °¡Áö°í ¾²±â ¼öÇà. ¾ÕºÎºÐ¿¡¼­ 2Mbyte¸¸ ¾¸
    for( i = 0 ; i < ( 2 * 1024 * 1024 / 1024 ) ; i++ )
    {
        kPrintf( "    [%d] Offset [%d] Byte [%d] Write...", i, i * 1024, 1024 );

        // 1024 ¹ÙÀÌÆ®¾¿ ÆÄÀÏÀ» ¾¸
        if( fwrite( pbBuffer + ( i * 1024 ), 1, 1024, pstFile ) != 1024 )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }
    }

    // ÆÄÀÏÀÇ Ã³À½À¸·Î ÀÌµ¿
    fseek( pstFile, -dwMaxFileSize, SEEK_SET );
    
    // ¿­¸° ÇÚµéÀ» °¡Áö°í ÀÐ±â ¼öÇà ÈÄ µ¥ÀÌÅÍ °ËÁõ. Random Write·Î µ¥ÀÌÅÍ°¡ Àß¸ø 
    // ÀúÀåµÉ ¼ö ÀÖÀ¸¹Ç·Î °ËÁõÀº 4Mbyte ÀüÃ¼¸¦ ´ë»óÀ¸·Î ÇÔ
    for( i = 0 ; i < ( dwMaxFileSize / 1024 )  ; i++ )
    {
        // µ¥ÀÌÅÍ °Ë»ç
        kPrintf( "    [%d] Offset [%d] Byte [%d] Read And Verify...", i, 
                i * 1024, 1024 );
        
        // 1024 ¹ÙÀÌÆ®¾¿ ÆÄÀÏÀ» ÀÐÀ½
        if( fread( vbTempBuffer, 1, 1024, pstFile ) != 1024 )
        {
            kPrintf( "[Fail]\n" );
            return ;
        }
        
        if( kMemCmp( pbBuffer + ( i * 1024 ), vbTempBuffer, 1024 ) != 0 )
        {
            kPrintf( "[Fail]\n" );
            break;
        }
        else
        {
            kPrintf( "[Pass]\n" );
        }
    }
        
    //==========================================================================
    // ÆÄÀÏ »èÁ¦ ½ÇÆÐ Å×½ºÆ®
    //==========================================================================
    kPrintf( "8. File Delete Fail Test..." );
    // ÆÄÀÏÀÌ ¿­·ÁÀÖ´Â »óÅÂÀÌ¹Ç·Î ÆÄÀÏÀ» Áö¿ì·Á°í ÇÏ¸é ½ÇÆÐÇØ¾ß ÇÔ
    if( remove( "testfileio.bin" ) != 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    
    //==========================================================================
    // ÆÄÀÏ ´Ý±â Å×½ºÆ®
    //==========================================================================
    kPrintf( "9. File Close Test..." );
    // ÆÄÀÏÀÌ Á¤»óÀûÀ¸·Î ´ÝÇô¾ß ÇÔ
    if( fclose( pstFile ) == 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }

    //==========================================================================
    // ÆÄÀÏ »èÁ¦ Å×½ºÆ®
    //==========================================================================
    kPrintf( "10. File Delete Test..." );
    // ÆÄÀÏÀÌ ´ÝÇûÀ¸¹Ç·Î Á¤»óÀûÀ¸·Î Áö¿öÁ®¾ß ÇÔ 
    if( remove( "testfileio.bin" ) == 0 )
    {
        kPrintf( "[Pass]\n" );
    }
    else
    {
        kPrintf( "[Fail]\n" );
    }
    
    // ¸Þ¸ð¸®¸¦ ÇØÁ¦
    kFreeMemory( pbBuffer );    
}

/**
 *  ÆÄÀÏÀ» ÀÐ°í ¾²´Â ¼Óµµ¸¦ ÃøÁ¤
 */
static void kTestPerformance( const char* pcParameterBuffer )
{
    FILE* pstFile;
    DWORD dwClusterTestFileSize;
    DWORD dwOneByteTestFileSize;
    QWORD qwLastTickCount;
    DWORD i;
    BYTE* pbBuffer;
    
    // Å¬·¯½ºÅÍ´Â 1Mbyte±îÁö ÆÄÀÏÀ» Å×½ºÆ®
    dwClusterTestFileSize = 1024 * 1024;
    // 1¹ÙÀÌÆ®¾¿ ÀÐ°í ¾²´Â Å×½ºÆ®´Â ½Ã°£ÀÌ ¸¹ÀÌ °É¸®¹Ç·Î 16Kbyte¸¸ Å×½ºÆ®
    dwOneByteTestFileSize = 16 * 1024;
    
    // Å×½ºÆ®¿ë ¹öÆÛ ¸Þ¸ð¸® ÇÒ´ç
    pbBuffer = kAllocateMemory( dwClusterTestFileSize );
    if( pbBuffer == NULL )
    {
        kPrintf( "Memory Allocate Fail\n" );
        return ;
    }
    
    // ¹öÆÛ¸¦ ÃÊ±âÈ­
    kMemSet( pbBuffer, 0, FILESYSTEM_CLUSTERSIZE );
    
    kPrintf( "================== File I/O Performance Test ==================\n" );

    //==========================================================================
    // Å¬·¯½ºÅÍ ´ÜÀ§·Î ÆÄÀÏÀ» ¼øÂ÷ÀûÀ¸·Î ¾²´Â Å×½ºÆ®
    //==========================================================================
    kPrintf( "1.Sequential Read/Write Test(Cluster Size)\n" );

    // ±âÁ¸ÀÇ Å×½ºÆ® ÆÄÀÏÀ» Á¦°ÅÇÏ°í »õ·Î ¸¸µê
    remove( "performance.txt" );
    pstFile = fopen( "performance.txt", "w" );
    if( pstFile == NULL )
    {
        kPrintf( "File Open Fail\n" );
        kFreeMemory( pbBuffer );
        return ;
    }
    
    qwLastTickCount = kGetTickCount();
    // Å¬·¯½ºÅÍ ´ÜÀ§·Î ¾²´Â Å×½ºÆ®
    for( i = 0 ; i < ( dwClusterTestFileSize / FILESYSTEM_CLUSTERSIZE ) ; i++ )
    {
        if( fwrite( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) != 
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "Write Fail\n" );
            // ÆÄÀÏÀ» ´Ý°í ¸Þ¸ð¸®¸¦ ÇØÁ¦ÇÔ
            fclose( pstFile );
            kFreeMemory( pbBuffer );
            return ;
        }
    }
    // ½Ã°£ Ãâ·Â
    kPrintf( "   Sequential Write(Cluster Size): %d ms\n", kGetTickCount() - 
             qwLastTickCount );

    //==========================================================================
    // Å¬·¯½ºÅÍ ´ÜÀ§·Î ÆÄÀÏÀ» ¼øÂ÷ÀûÀ¸·Î ÀÐ´Â Å×½ºÆ®
    //==========================================================================
    // ÆÄÀÏÀÇ Ã³À½À¸·Î ÀÌµ¿
    fseek( pstFile, 0, SEEK_SET );
    
    qwLastTickCount = kGetTickCount();
    // Å¬·¯½ºÅÍ ´ÜÀ§·Î ÀÐ´Â Å×½ºÆ®
    for( i = 0 ; i < ( dwClusterTestFileSize / FILESYSTEM_CLUSTERSIZE ) ; i++ )
    {
        if( fread( pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile ) != 
            FILESYSTEM_CLUSTERSIZE )
        {
            kPrintf( "Read Fail\n" );
            // ÆÄÀÏÀ» ´Ý°í ¸Þ¸ð¸®¸¦ ÇØÁ¦ÇÔ
            fclose( pstFile );
            kFreeMemory( pbBuffer );
            return ;
        }
    }
    // ½Ã°£ Ãâ·Â
    kPrintf( "   Sequential Read(Cluster Size): %d ms\n", kGetTickCount() - 
             qwLastTickCount );
    
    //==========================================================================
    // 1 ¹ÙÀÌÆ® ´ÜÀ§·Î ÆÄÀÏÀ» ¼øÂ÷ÀûÀ¸·Î ¾²´Â Å×½ºÆ®
    //==========================================================================
    kPrintf( "2.Sequential Read/Write Test(1 Byte)\n" );
    
    // ±âÁ¸ÀÇ Å×½ºÆ® ÆÄÀÏÀ» Á¦°ÅÇÏ°í »õ·Î ¸¸µê
    remove( "performance.txt" );
    pstFile = fopen( "performance.txt", "w" );
    if( pstFile == NULL )
    {
        kPrintf( "File Open Fail\n" );
        kFreeMemory( pbBuffer );
        return ;
    }
    
    qwLastTickCount = kGetTickCount();
    // 1 ¹ÙÀÌÆ® ´ÜÀ§·Î ¾²´Â Å×½ºÆ®
    for( i = 0 ; i < dwOneByteTestFileSize ; i++ )
    {
        if( fwrite( pbBuffer, 1, 1, pstFile ) != 1 )
        {
            kPrintf( "Write Fail\n" );
            // ÆÄÀÏÀ» ´Ý°í ¸Þ¸ð¸®¸¦ ÇØÁ¦ÇÔ
            fclose( pstFile );
            kFreeMemory( pbBuffer );
            return ;
        }
    }
    // ½Ã°£ Ãâ·Â
    kPrintf( "   Sequential Write(1 Byte): %d ms\n", kGetTickCount() - 
             qwLastTickCount );

    //==========================================================================
    // 1 ¹ÙÀÌÆ® ´ÜÀ§·Î ÆÄÀÏÀ» ¼øÂ÷ÀûÀ¸·Î ÀÐ´Â Å×½ºÆ®
    //==========================================================================
    // ÆÄÀÏÀÇ Ã³À½À¸·Î ÀÌµ¿
    fseek( pstFile, 0, SEEK_SET );
    
    qwLastTickCount = kGetTickCount();
    // 1 ¹ÙÀÌÆ® ´ÜÀ§·Î ÀÐ´Â Å×½ºÆ®
    for( i = 0 ; i < dwOneByteTestFileSize ; i++ )
    {
        if( fread( pbBuffer, 1, 1, pstFile ) != 1 )
        {
            kPrintf( "Read Fail\n" );
            // ÆÄÀÏÀ» ´Ý°í ¸Þ¸ð¸®¸¦ ÇØÁ¦ÇÔ
            fclose( pstFile );
            kFreeMemory( pbBuffer );
            return ;
        }
    }
    // ½Ã°£ Ãâ·Â
    kPrintf( "   Sequential Read(1 Byte): %d ms\n", kGetTickCount() - 
             qwLastTickCount );
    
    // ÆÄÀÏÀ» ´Ý°í ¸Þ¸ð¸®¸¦ ÇØÁ¦ÇÔ
    fclose( pstFile );
    kFreeMemory( pbBuffer );
}

/**
 *  ÆÄÀÏ ½Ã½ºÅÛÀÇ Ä³½Ã ¹öÆÛ¿¡ ÀÖ´Â µ¥ÀÌÅÍ¸¦ ¸ðµÎ ÇÏµå µð½ºÅ©¿¡ ¾¸ 
 */
static void kFlushCache( const char* pcParameterBuffer )
{
    QWORD qwTickCount;
    
    qwTickCount = kGetTickCount();
    kPrintf( "Cache Flush... ");
    if( kFlushFileSystemCache() == TRUE )
    {
        kPrintf( "Pass\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
    kPrintf( "Total Time = %d ms\n", kGetTickCount() - qwTickCount );
}

void kDummy(const char* pcParamegerBuffer){}

// Exception cause
void kPagefault( const char* pcParameterBuffer )
{
	DWORD **faultptr;
	faultptr = 0x1ff000;
	DWORD test = *faultptr;
}

void kProtectionfault( const char* pcParameterBuffer )
{
	DWORD **faultptr;
	faultptr = 0x1ff000;
	*faultptr = 0xdeadbeef;
}

