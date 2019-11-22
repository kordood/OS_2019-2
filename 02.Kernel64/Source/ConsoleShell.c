/**onsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ÄÜŒÖ ŒÐ¿¡ °ü·ÃµÈ ŒÒœº ÆÄÀÏ
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

// Ä¿žÇµå Å×ÀÌºí Á€ÀÇ
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
        { "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
                kChangeTaskPriority },
        { "tasklist", "Show Task List", kShowTaskList },
        { "killtask", "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)", kKillTask },
        { "cpuload", "Show Processor Load", kCPULoad },
        { "testmutex", "Test Mutex Function", kTestMutex },
        { "testthread", "Test Thread And Process Function", kTestThread },
        { "showmatrix", "Show Matrix Screen", kShowMatrix },	
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
//  œÇÁŠ ŒÐÀ» ±žŒºÇÏŽÂ ÄÚµå
//==============================================================================
/**
 *  ŒÐÀÇ žÞÀÎ ·çÇÁ
 */
void kStartConsoleShell( void )
{
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

	// ¸í·É¹® 10°³ ÀúÀå¿ë list »ý¼º
	DoubleLinkedList spdll;
	DoubleLinkedList* spDLL = CreateLinkedList(&spdll);
	int iCNT = 0;
	SNode node[10];
	char data[10][CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	char* tmpData;
	int UPnDOWN = 0;
	int DownOn = 0;
	int UpOn = 0;

    // ÇÁ·ÒÇÁÆ® Ãâ·Â
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );
    
    while( 1 )
    {
		if(iCommandBufferIndex == 0){
			DownOn = 0;
			UPnDOWN = 0;
		}
        // Å°°¡ ŒöœÅµÉ ¶§±îÁö Žë±â
        bKey = kGetCh();
        // Backspace Å° Ã³ž®
        if( bKey == KEY_BACKSPACE )
        {
			tabflag = 0;
            if( iCommandBufferIndex > 0 )
            {
                // ÇöÀç Ä¿Œ­ À§Ä¡žŠ ŸòŸîŒ­ ÇÑ ¹®ÀÚ ŸÕÀž·Î ÀÌµ¿ÇÑ ŽÙÀœ °ø¹éÀ» Ãâ·ÂÇÏ°í 
                // Ä¿žÇµå ¹öÆÛ¿¡Œ­ ž¶Áöž· ¹®ÀÚ »èÁŠ
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
        // ¿£ÅÍ Å° Ã³ž®
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
                // Ä¿žÇµå ¹öÆÛ¿¡ ÀÖŽÂ ží·ÉÀ» œÇÇà
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                kExecuteCommand( vcCommandBuffer );
            }
            
			if(iCommandBufferIndex > 0){
				//¸í·É¹®À» DoubleLinkedList¿¡ »ðÀÔ
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

            // ÇÁ·ÒÇÁÆ® Ãâ·Â ¹× Ä¿žÇµå ¹öÆÛ ÃÊ±âÈ­
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
        }
        // œÃÇÁÆ® Å°, CAPS Lock, NUM Lock, Scroll LockÀº ¹«œÃ
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
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
 *  Ä¿žÇµå ¹öÆÛ¿¡ ÀÖŽÂ Ä¿žÇµåžŠ ºñ±³ÇÏ¿© ÇØŽç Ä¿žÇµåžŠ Ã³ž®ÇÏŽÂ ÇÔŒöžŠ ŒöÇà
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // °ø¹éÀž·Î ±žºÐµÈ Ä¿žÇµåžŠ ÃßÃâ
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // Ä¿žÇµå Å×ÀÌºíÀ» °Ë»çÇØŒ­ µ¿ÀÏÇÑ ÀÌž§ÀÇ Ä¿žÇµå°¡ ÀÖŽÂÁö È®ÀÎ
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // Ä¿žÇµåÀÇ ±æÀÌ¿Í ³»¿ëÀÌ ¿ÏÀüÈ÷ ÀÏÄ¡ÇÏŽÂÁö °Ë»ç
        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
            break;
        }
    }

    // ž®œºÆ®¿¡Œ­ Ã£À» Œö ŸøŽÙžé ¿¡·¯ Ãâ·Â
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
    }
}

/**
 *  ÆÄ¶ó¹ÌÅÍ ÀÚ·á±žÁ¶žŠ ÃÊ±âÈ­
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  °ø¹éÀž·Î ±žºÐµÈ ÆÄ¶ó¹ÌÅÍÀÇ ³»¿ë°ú ±æÀÌžŠ ¹ÝÈ¯
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // Žõ ÀÌ»ó ÆÄ¶ó¹ÌÅÍ°¡ ŸøÀžžé ³ª°š
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // ¹öÆÛÀÇ ±æÀÌžžÅ­ ÀÌµ¿ÇÏžéŒ­ °ø¹éÀ» °Ë»ö
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // ÆÄ¶ó¹ÌÅÍžŠ º¹»çÇÏ°í ±æÀÌžŠ ¹ÝÈ¯
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // ÆÄ¶ó¹ÌÅÍÀÇ À§Ä¡ Ÿ÷µ¥ÀÌÆ®
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  Ä¿žÇµåžŠ Ã³ž®ÇÏŽÂ ÄÚµå
//==============================================================================
/**
 *  ŒÐ µµ¿òž»À» Ãâ·Â
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

    // °¡Àå ±ä Ä¿žÇµåÀÇ ±æÀÌžŠ °è»ê
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // µµ¿òž» Ãâ·Â
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

/**
 *  È­žéÀ» Áö¿ò 
 */
static void kCls( const char* pcParameterBuffer )
{
    // žÇ À­ÁÙÀº µð¹ö±ë ¿ëÀž·Î »ç¿ëÇÏ¹Ç·Î È­žéÀ» Áö¿î ÈÄ, ¶óÀÎ 1·Î Ä¿Œ­ ÀÌµ¿
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  ÃÑ žÞžðž® Å©±âžŠ Ãâ·Â
 */
static void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  ¹®ÀÚ¿­·Î µÈ ŒýÀÚžŠ ŒýÀÚ·Î º¯È¯ÇÏ¿© È­žé¿¡ Ãâ·Â
 */
static void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    // ÆÄ¶ó¹ÌÅÍ ÃÊ±âÈ­
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {
        // ŽÙÀœ ÆÄ¶ó¹ÌÅÍžŠ ±žÇÔ, ÆÄ¶ó¹ÌÅÍÀÇ ±æÀÌ°¡ 0ÀÌžé ÆÄ¶ó¹ÌÅÍ°¡ ŸøŽÂ °ÍÀÌ¹Ç·Î
        // ÁŸ·á
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        // ÆÄ¶ó¹ÌÅÍ¿¡ ŽëÇÑ Á€ºžžŠ Ãâ·ÂÇÏ°í 16ÁøŒöÀÎÁö 10ÁøŒöÀÎÁö ÆÇŽÜÇÏ¿© º¯È¯ÇÑ ÈÄ
        // °á°úžŠ printf·Î Ãâ·Â
        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        // 0x·Î œÃÀÛÇÏžé 16ÁøŒö, ±×¿ÜŽÂ 10ÁøŒö·Î ÆÇŽÜ
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
 *  PCžŠ ÀçœÃÀÛ(Reboot)
 */
static void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // Å°ºžµå ÄÁÆ®·Ñ·¯žŠ ÅëÇØ PCžŠ ÀçœÃÀÛ
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

/**
 *  PIT ÄÁÆ®·Ñ·¯ÀÇ Ä«¿îÅÍ 0 Œ³Á€
 */
static void kSetTimer( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    // ÆÄ¶ó¹ÌÅÍ ÃÊ±âÈ­
    kInitializeParameter( &stList, pcParameterBuffer );
    
    // milisecond ÃßÃâ
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
        return ;
    }
    lValue = kAToI( vcParameter, 10 );

    // Periodic ÃßÃâ
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
 *  PIT ÄÁÆ®·Ñ·¯žŠ Á÷Á¢ »ç¿ëÇÏ¿© ms µ¿ŸÈ Žë±â  
 */
static void kWaitUsingPIT( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;
    
    // ÆÄ¶ó¹ÌÅÍ ÃÊ±âÈ­
    kInitializeParameter( &stList, pcParameterBuffer );
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)wait 100(ms)\n" );
        return ;
    }
    
    lMillisecond = kAToI( pcParameterBuffer, 10 );
    kPrintf( "%d ms Sleep Start...\n", lMillisecond );
    
    // ÀÎÅÍ·ŽÆ®žŠ ºñÈ°ŒºÈ­ÇÏ°í PIT ÄÁÆ®·Ñ·¯žŠ ÅëÇØ Á÷Á¢ œÃ°£À» ÃøÁ€
    kDisableInterrupt();
    for( i = 0 ; i < lMillisecond / 30 ; i++ )
    {
        kWaitUsingDirectPIT( MSTOCOUNT( 30 ) );
    }
    kWaitUsingDirectPIT( MSTOCOUNT( lMillisecond % 30 ) );   
    kEnableInterrupt();
    kPrintf( "%d ms Sleep Complete\n", lMillisecond );
    
    // ÅžÀÌžÓ º¹¿ø
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );
}

/**
 *  ÅžÀÓ œºÅÆÇÁ Ä«¿îÅÍžŠ ÀÐÀœ  
 */
static void kReadTimeStampCounter( const char* pcParameterBuffer )
{
    QWORD qwTSC;
    
    qwTSC = kReadTSC();
    kPrintf( "Time Stamp Counter = %q\n", qwTSC );
}

/**
 *  ÇÁ·ÎŒŒŒ­ÀÇ ŒÓµµžŠ ÃøÁ€
 */
static void kMeasureProcessorSpeed( const char* pcParameterBuffer )
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    kPrintf( "Now Measuring." );
    
    // 10ÃÊ µ¿ŸÈ º¯È­ÇÑ ÅžÀÓ œºÅÆÇÁ Ä«¿îÅÍžŠ ÀÌ¿ëÇÏ¿© ÇÁ·ÎŒŒŒ­ÀÇ ŒÓµµžŠ °£Á¢ÀûÀž·Î ÃøÁ€
    kDisableInterrupt();    
    for( i = 0 ; i < 200 ; i++ )
    {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT( MSTOCOUNT( 50 ) );
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf( "." );
    }
    // ÅžÀÌžÓ º¹¿ø
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );    
    kEnableInterrupt();
    
    kPrintf( "\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000 );
}

/**
 *  RTC ÄÁÆ®·Ñ·¯¿¡ ÀúÀåµÈ ÀÏÀÚ ¹× œÃ°£ Á€ºžžŠ Ç¥œÃ
 */
static void kShowDateAndTime( const char* pcParameterBuffer )
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    // RTC ÄÁÆ®·Ñ·¯¿¡Œ­ œÃ°£ ¹× ÀÏÀÚžŠ ÀÐÀœ
    kReadRTCTime( &bHour, &bMinute, &bSecond );
    kReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    kPrintf( "Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
             kConvertDayOfWeekToString( bDayOfWeek ) );
    kPrintf( "Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}

/**
 *  ÅÂœºÅ© 1
 *      È­žé Å×µÎž®žŠ µ¹žéŒ­ ¹®ÀÚžŠ Ãâ·Â
 */
static void kTestTask1( void )
{
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin, j;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    
    // ÀÚœÅÀÇ IDžŠ ŸòŸîŒ­ È­žé ¿ÀÇÁŒÂÀž·Î »ç¿ë
    pstRunningTask = kGetRunningTask();
    iMargin = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) % 10;
    
    // È­žé ³× ±ÍÅüÀÌžŠ µ¹žéŒ­ ¹®ÀÚ Ãâ·Â
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
        
        // ¹®ÀÚ ¹× »ö±ò ÁöÁ€
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bCharactor = bData;
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bAttribute = bData & 0x0F;
        bData++;
        
        // ŽÙž¥ ÅÂœºÅ©·Î ÀüÈ¯
        //kSchedule();
    }

    //kExitTask();
}

/**
 *  ÅÂœºÅ© 2
 *      ÀÚœÅÀÇ IDžŠ Âü°íÇÏ¿© Æ¯Á€ À§Ä¡¿¡ ÈžÀüÇÏŽÂ ¹Ù¶÷°³ºñžŠ Ãâ·Â
 */
static void kTestTask2( void )
{
    int i = 0, iOffset;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    char vcData[ 4 ] = { '-', '\\', '|', '/' };
    
    // ÀÚœÅÀÇ IDžŠ ŸòŸîŒ­ È­žé ¿ÀÇÁŒÂÀž·Î »ç¿ë
    pstRunningTask = kGetRunningTask();
    iOffset = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - 
        ( iOffset % ( CONSOLE_WIDTH * CONSOLE_HEIGHT ) );

    while( 1 )
    {
        // ÈžÀüÇÏŽÂ ¹Ù¶÷°³ºñžŠ Ç¥œÃ
        pstScreen[ iOffset ].bCharactor = vcData[ i % 4 ];
        // »ö±ò ÁöÁ€
        pstScreen[ iOffset ].bAttribute = ( iOffset % 15 ) + 1;
        i++;
        
        // ŽÙž¥ ÅÂœºÅ©·Î ÀüÈ¯
        //kSchedule();
    }
}

/**
 *  ÅÂœºÅ©žŠ »ýŒºÇØŒ­ žÖÆŒ ÅÂœºÅ· ŒöÇà
 */
static void kCreateTestTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcType[ 30 ];
    char vcCount[ 30 ];
    int i;
    
    // ÆÄ¶ó¹ÌÅÍžŠ ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcType );
    kGetNextParameter( &stList, vcCount );

    switch( kAToI( vcType, 10 ) )
    {
    // ÅžÀÔ 1 ÅÂœºÅ© »ýŒº
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
        
    // ÅžÀÔ 2 ÅÂœºÅ© »ýŒº
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
 *  ÅÂœºÅ©ÀÇ ¿ìŒ± ŒøÀ§žŠ º¯°æ
 */
static void kChangeTaskPriority( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    char vcPriority[ 30 ];
    QWORD qwID;
    BYTE bPriority;
    
    // ÆÄ¶ó¹ÌÅÍžŠ ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    kGetNextParameter( &stList, vcPriority );
    
    // ÅÂœºÅ©ÀÇ ¿ìŒ± ŒøÀ§žŠ º¯°æ
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
 *  ÇöÀç »ýŒºµÈ žðµç ÅÂœºÅ©ÀÇ Á€ºžžŠ Ãâ·Â
 */
static void kShowTaskList( const char* pcParameterBuffer )
{
    int i;
    TCB* pstTCB;
    int iCount = 0;
    
    kPrintf( "=========== Task Total Count [%d] ===========\n", kGetTaskCount() );
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        // TCBžŠ ±žÇØŒ­ TCB°¡ »ç¿ë ÁßÀÌžé IDžŠ Ãâ·Â
        pstTCB = kGetTCBInTCBPool( i );
        if( ( pstTCB->stLink.qwID >> 32 ) != 0 )
        {
            // ÅÂœºÅ©°¡ 10°³ Ãâ·ÂµÉ ¶§ž¶ŽÙ, °èŒÓ ÅÂœºÅ© Á€ºžžŠ Ç¥œÃÇÒÁö ¿©ºÎžŠ È®ÀÎ
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
			kPrintf("Link Addr: %x\n", &(pstTCB->stLink));
        }
    }
}

/**
 *  ÅÂœºÅ©žŠ ÁŸ·á
 */
static void kKillTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    QWORD qwID;
    TCB* pstTCB;
    int i;
    
    // ÆÄ¶ó¹ÌÅÍžŠ ÃßÃâ
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    
    // ÅÂœºÅ©žŠ ÁŸ·á
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    // Æ¯Á€ IDžž ÁŸ·áÇÏŽÂ °æ¿ì
    if( qwID != 0xFFFFFFFF )
    {
        pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
		kPrintf("ID: %x\n", &(pstTCB->stLink));
        qwID = pstTCB->stLink.qwID;

        // œÃœºÅÛ Å×œºÆ®ŽÂ ÁŠ¿Ü
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
    // ÄÜŒÖ ŒÐ°ú À¯ÈÞ ÅÂœºÅ©žŠ ÁŠ¿ÜÇÏ°í žðµç ÅÂœºÅ© ÁŸ·á
    else
    {
        for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
        {
            pstTCB = kGetTCBInTCBPool( i );
            qwID = pstTCB->stLink.qwID;

            // œÃœºÅÛ Å×œºÆ®ŽÂ »èÁŠ žñ·Ï¿¡Œ­ ÁŠ¿Ü
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
 *  ÇÁ·ÎŒŒŒ­ÀÇ »ç¿ë·üÀ» Ç¥œÃ
 */
static void kCPULoad( const char* pcParameterBuffer )
{
    kPrintf( "Processor Load : %d%%\n", kGetProcessorLoad() );
}
    
// ¹ÂÅØœº Å×œºÆ®¿ë ¹ÂÅØœº¿Í º¯Œö
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

/**
 *  ¹ÂÅØœºžŠ Å×œºÆ®ÇÏŽÂ ÅÂœºÅ©
 */
static void kPrintNumberTask( void )
{
    int i;
    int j;
    QWORD qwTickCount;

    // 50ms Á€µµ Žë±âÇÏ¿© ÄÜŒÖ ŒÐÀÌ Ãâ·ÂÇÏŽÂ žÞœÃÁö¿Í °ãÄ¡Áö ŸÊµµ·Ï ÇÔ
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 50 )
    {
        kSchedule();
    }    
    
    // ·çÇÁžŠ µ¹žéŒ­ ŒýÀÚžŠ Ãâ·Â
    for( i = 0 ; i < 5 ; i++ )
    {
        kLock( &( gs_stMutex ) );
        kPrintf( "Task ID [0x%Q] Value[%d]\n", kGetRunningTask()->stLink.qwID,
                gs_qwAdder );
        
        gs_qwAdder += 1;
        kUnlock( & ( gs_stMutex ) );
    
        // ÇÁ·ÎŒŒŒ­ ŒÒžðžŠ ŽÃž®·Á°í Ãß°¡ÇÑ ÄÚµå
        for( j = 0 ; j < 30000 ; j++ ) ;
    }
    
    // žðµç ÅÂœºÅ©°¡ ÁŸ·áÇÒ ¶§±îÁö 1ÃÊ(100ms) Á€µµ Žë±â
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 1000 )
    {
        kSchedule();
    }    
    
    // ÅÂœºÅ© ÁŸ·á
    //kExitTask();
}

/**
 *  ¹ÂÅØœºžŠ Å×œºÆ®ÇÏŽÂ ÅÂœºÅ© »ýŒº
 */
static void kTestMutex( const char* pcParameterBuffer )
{
    int i;
    
    gs_qwAdder = 1;
    
    // ¹ÂÅØœº ÃÊ±âÈ­
    kInitializeMutex( &gs_stMutex );
    
    for( i = 0 ; i < 3 ; i++ )
    {
        // ¹ÂÅØœºžŠ Å×œºÆ®ÇÏŽÂ ÅÂœºÅ©žŠ 3°³ »ýŒº
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kPrintNumberTask );
    }    
    kPrintf( "Wait Util %d Task End...\n", i );
    kGetCh();
}

/**
 *  ÅÂœºÅ© 2žŠ ÀÚœÅÀÇ œº·¹µå·Î »ýŒºÇÏŽÂ ÅÂœºÅ©
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
 *  œº·¹µåžŠ Å×œºÆ®ÇÏŽÂ ÅÂœºÅ© »ýŒº
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

// ³­ŒöžŠ ¹ß»ýœÃÅ°±â À§ÇÑ º¯Œö
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  ÀÓÀÇÀÇ ³­ŒöžŠ ¹ÝÈ¯
 */
QWORD kRandom( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

/**
 *  Ã¶ÀÚžŠ Èê·¯³»ž®°Ô ÇÏŽÂ œº·¹µå
 */
static void kDropCharactorThread( void )
{
    int iX, iY;
    int i;
    char vcText[ 2 ] = { 0, };

    iX = kRandom() % CONSOLE_WIDTH;
    
    while( 1 )
    {
        // ÀáœÃ Žë±âÇÔ
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
 *  œº·¹µåžŠ »ýŒºÇÏ¿© žÅÆ®ž¯œº È­žéÃ³·³ ºž¿©ÁÖŽÂ ÇÁ·ÎŒŒœº
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

    // Å°°¡ ÀÔ·ÂµÇžé ÇÁ·ÎŒŒœº ÁŸ·á
    kGetCh();
}

/**
 *  žÅÆ®ž¯œº È­žéÀ» ºž¿©ÁÜ
 */
static void kShowMatrix( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, ( void* ) 0xE00000, 0xE00000, 
                              ( QWORD ) kMatrixProcess );
    if( pstProcess != NULL )
    {
        kPrintf( "Matrix Process [0x%Q] Create Success\n" );

        // ÅÂœºÅ©°¡ ÁŸ·á µÉ ¶§±îÁö Žë±â
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

