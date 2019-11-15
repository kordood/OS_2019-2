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

// 커맨드 테이블 정의
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{ "help", "Show Help", kHelp },
	{ "cls", "Clear Screen", kCls },
	{ "totalram", "Show Total RAM Size", kShowTotalRAMSize },
	{ "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
	{ "shutdown", "Shutdown And Reboot OS", kShutdown },
	{ "strcat", "String Cat(dummy)", kDummy },
	{ "stra", "String (dummy)", kDummy },
	{ "strcbat", "String (dummy)", kDummy },
	{ "strcact", "String (dummy)", kDummy },
	{ "strcats", "String (dummy)", kDummy },
	{ "strcp", "String Copy(dummy)", kDummy },
	{ "rm", "Remove File(dummy)", kDummy },
	{ "rmdir", "Remove Directory(dummy)", kDummy },
	{ "pwd", "Present Working Directory(dummy)", kDummy },
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
		else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) || ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) || ( bKey == KEY_SCROLLLOCK ) )
		{
			tabflag = 0;
		}
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

/*char* kSubstr( const char * str, int size)
  {
  char sub[100];
  int i;
  for(i = 0;i<size;i++){
  sub[i] = str[i];
  }
//sub[i] = "\0";

return sub;

}*/

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
void kHelp( const char* pcCommandBuffer )
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
void kCls( const char* pcParameterBuffer )
{
	// 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지운 후, 라인 1로 커서 이동
	kClearScreen();
	kSetCursor( 0, 1 );
}

/**
 *  총 메모리 크기를 출력
 */
void kShowTotalRAMSize( const char* pcParameterBuffer )
{
	kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  문자열로 된 숫자를 숫자로 변환하여 화면에 출력
 */
void kStringToDecimalHexTest( const char* pcParameterBuffer )
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
void kShutdown( const char* pcParamegerBuffer )
{
	kPrintf( "System Shutdown Start...\n" );

	// 키보드 컨트롤러를 통해 PC를 재시작
	kPrintf( "Press Any Key To Reboot PC..." );
	kGetCh();
	kReboot();
}


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

void kDummy(const char* pcParamegerBuffer){}
