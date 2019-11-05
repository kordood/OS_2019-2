/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   肄 몄 愿�⑤  
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "DoubleLinkedList.h"

// 而ㅻ㎤ 대 �
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
//	{ "rmdir", "Remove Directory(dummy)", kDummy },
	{ "pwd", "Present Working Directory(dummy)", kDummy },
    { "pagefault", "Cause page fault", kPagefault },
    { "protfault", "Cause protection fault", kProtectionfault },
};                                     

int tabflag = 0;
//==============================================================================
/**
 *  몄 硫 猷⑦
 */
void kStartConsoleShell( void )
{
	char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
	int iCommandBufferIndex = 0;
	BYTE bKey;
	int iCursorX, iCursorY;

	// 쨍챠쨌쨔짰 10째쨀 첬책쩔챘 list 쨩첵쩌쨘
	DoubleLinkedList spdll;
	DoubleLinkedList* spDLL = CreateLinkedList(&spdll);
	int iCNT = 0;
	SNode node[10];
	char data[10][CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	char* tmpData;
	int UPnDOWN = 0;
	int DownOn = 0;
	int UpOn = 0;

	// 濡ы 異�
	kPrintf( CONSOLESHELL_PROMPTMESSAGE );

	while( 1 )
	{
		if(iCommandBufferIndex == 0){
			DownOn = 0;
			UPnDOWN = 0;
		}
		// ㅺ  源吏 湲
		bKey = kGetCh();
		// Backspace  泥由
		if( bKey == KEY_BACKSPACE )
		{
			tabflag = 0;
			if( iCommandBufferIndex > 0 )
			{
				//  而ㅼ 移瑜 살댁  臾몄 쇰 대 ㅼ 怨듬갚 異�ν怨 
				// 而ㅻ㎤ 踰쇱 留吏留 臾몄 �
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
		//   泥由
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
				// 而ㅻ㎤ 踰쇱  紐�뱀 ㅽ
				vcCommandBuffer[ iCommandBufferIndex ] = '\0';
				kExecuteCommand( vcCommandBuffer );
			}

			if(iCommandBufferIndex > 0){
				//쨍챠쨌쨔짰쨩 DoubleLinkedList쩔징 쨩챨
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

			// 濡ы 異� 諛 而ㅻ㎤ 踰 珥湲고
			kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
			kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
			iCommandBufferIndex = 0;
		}
		//  , CAPS Lock, NUM Lock, Scroll Lock 臾댁
		else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) || ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) || ( bKey == KEY_SCROLLLOCK ) )
		{
			tabflag = 0;
		}
		else if((bKey == KEY_UP) || (bKey == KEY_DOWN)){
			if(bKey == KEY_UP){
				if(UPnDOWN > 0) DownOn = 1;

				if((UPnDOWN == 0)&&(UpOn == 0)){
					tmpData = IteratePrevList(spDLL, UPnDOWN);	
					if(spDLL->m_spIterator!=spDLL->m_spHead) UPnDOWN++;
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
				if(iCommandBufferIndex > 0)
					tmpData = IterateNextList(spDLL,UPnDOWN);	
				else tmpData = '\0';	
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

			// 踰쇱 怨듦 ⑥ 留 媛
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
	int iCount;					//�泥 而ㅻ㎤ 媛�
	int iCommandLength;			//而ㅻ㎤  湲몄
	char subString[100];		//而ㅻ㎤substring

	int pcCommandBufferLength;	// 踰쇱 ㅼ댁 湲몄

	int i;	
	int j;
	int cnt = 0;
	int cmdIndex[100];
	char* pCommand;


	iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

	pcCommandBufferLength = kStrLen(pcCommandBuffer);


	for( i = 0 ; i < iCount ; i++ )
	{
		//buffer ㅼ댁 湲몄대 substring 댁＜湲
		pCommand = gs_vstCommandTable[i].pcCommand;

		for(j = 0; j < pcCommandBufferLength; j++){
			subString[j] = pCommand[j];
		}
		// 而ㅻ㎤ 댁⑹ 쇱吏 寃
		if(( kMemCmp( subString, pcCommandBuffer, pcCommandBufferLength ) == 0 ) )	//null臾몄 명대낫湲
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
			if(cmdIndex[i] == 1)	//紐踰吏 紐�뱀댁몄 
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

				if(cmdIndex[i] == 1){	//쇱寃 

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
 *  而ㅻ㎤ 踰쇱  而ㅻ㎤瑜 鍮援 대 而ㅻ㎤瑜 泥由ы ⑥瑜 
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
	int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;
	int iCount;

	// 怨듬갚쇰 援щ 而ㅻ㎤瑜 異異
	iCommandBufferLength = kStrLen( pcCommandBuffer );
	for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
	{
		if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
		{
			break;
		}
	}

	// 而ㅻ㎤ 대 寃ы댁 쇳 대 而ㅻ㎤媛 吏 
	iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
	for( i = 0 ; i < iCount ; i++ )
	{
		iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
		// 而ㅻ㎤ 湲몄댁 댁⑹ � 쇱吏 寃
		if( ( iCommandLength == iSpaceIndex ) &&
				( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
						   iSpaceIndex ) == 0 ) )
		{
			gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
			break;
		}
	}

	// 由ъㅽ몄 李얠  ㅻ㈃  異�
	if( i >= iCount )
	{
		kPrintf( "'%s' is not found.\n", pcCommandBuffer );
	}
}

/**
 *  쇰� 猷援ъ“瑜 珥湲고
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
	pstList->pcBuffer = pcParameter;
	pstList->iLength = kStrLen( pcParameter );
	pstList->iCurrentPosition = 0;
}

/**
 *  怨듬갚쇰 援щ 쇰�곗 댁⑷낵 湲몄대� 諛
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
	int i;
	int iLength;

	//  댁 쇰�곌 쇰㈃ 媛
	if( pstList->iLength <= pstList->iCurrentPosition )
	{
		return 0;
	}

	// 踰쇱 湲몄대 대硫댁 怨듬갚 寃
	for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
	{
		if( pstList->pcBuffer[ i ] == ' ' )
		{
			break;
		}
	}

	// 쇰�곕� 蹂듭ы怨 湲몄대� 諛
	kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
	iLength = i - pstList->iCurrentPosition;
	pcParameter[ iLength ] = '\0';

	// 쇰�곗 移 곗댄
	pstList->iCurrentPosition += iLength + 1;
	return iLength;
}

//==============================================================================
//  而ㅻ㎤瑜 泥由ы 肄
//==============================================================================
/**
 *   留 異�
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

	// 媛 湲 而ㅻ㎤ 湲몄대� 怨
	for( i = 0 ; i < iCount ; i++ )
	{
		iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
		if( iLength > iMaxCommandLength )
		{
			iMaxCommandLength = iLength;
		}
	}

	// 留 異�
	for( i = 0 ; i < iCount ; i++ )
	{
		kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
		kGetCursor( &iCursorX, &iCursorY );
		kSetCursor( iMaxCommandLength, iCursorY );
		kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
	}
}

/**
 *  硫댁 吏 
 */
void kCls( const char* pcParameterBuffer )
{
	// 留 以 踰源 ⑹쇰 ъ⑺誘濡 硫댁 吏 , 쇱 1濡 而ㅼ 대
	kClearScreen();
	kSetCursor( 0, 1 );
}

/**
 *  珥 硫紐⑤━ ш린瑜 異�
 */
void kShowTotalRAMSize( const char* pcParameterBuffer )
{
	kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  臾몄대  レ瑜 レ濡 蹂 硫댁 異�
 */
void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
	char vcParameter[ 100 ];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	// 쇰� 珥湲고
	kInitializeParameter( &stList, pcParameterBuffer );

	while( 1 )
	{
		// ㅼ 쇰�곕� 援ы, 쇰�곗 湲몄닿 0대㈃ 쇰�곌  寃대濡
		// 醫猷
		iLength = kGetNextParameter( &stList, vcParameter );
		if( iLength == 0 )
		{
			break;
		}

		// 쇰�곗  �蹂대� 異�ν怨 16吏몄 10吏몄 ⑦ 蹂 
		// 寃곌낵瑜 printf濡 異�
		kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
				vcParameter, iLength );

		// 0x濡 硫 16吏, 洹몄몃 10吏濡 
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
 *  PC瑜 ъ(Reboot)
 */
void kShutdown( const char* pcParamegerBuffer )
{
	kPrintf( "System Shutdown Start...\n" );

	// ㅻ낫 而⑦몃·щ� 듯 PC瑜 ъ
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
