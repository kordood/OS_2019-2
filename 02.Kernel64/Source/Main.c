/**
 *  file    Main.c
 *  date    2009/01/02
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 커널의 엔트리 포인트 파일
 */

#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"

// 함수 선언
void kPrintStringMapped( int iX, int iY, const char* pcString );
void kRWROTest();

/**
 *  아래 함수는 C 언어 커널의 시작 부분임
 */
void Main( void )
{
    int iCursorX, iCursorY;

    // 콘솔을 먼저 초기화한 후, 다음 작업을 수행
    kInitializeConsole( 0, 10 );    
    kPrintf( "Switch To IA-32e Mode Success~!!\n" );
    kPrintf( "IA-32e C Language Kernel Start..............[Pass]\n" );
    kPrintf( "Initialize Console..........................[Pass]\n" );
    kSetCursor( 45, iCursorY++ );
	iCursorY += 13;
    kPrintStringMapped( 0, 13, "This message is printed through the video memory relocated to 0xAB8000" );
    kSetCursor( 0, iCursorY++ );

	// kRWROTest(); unuse
    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "TSS Segment Load............................[    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "IDT Initialize..............................[    ]" );
    kInitializeIDTTables();    
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "Total RAM Size Check........................[    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );
    
    kPrintf( "Keyboard Activate And Queue Initialize......[    ]" );
    
    // 키보드를 활성화
    if( kInitializeKeyboard() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 ) ;
    }

    kPrintf( "PIC Controller And Interrupt Initialize.....[    ]" );
    // PIC 컨트롤러 초기화 및 모든 인터럽트 활성화
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );

	/*
	* DWORD** kernel1ff;
	* kernel1ff = 0x1ff000;
	* kPrintf( 0, 23, "Write to 0x1ff000 [  ]");
 	* *kernel1ff = 0xDEADBEEF;
	* kPrintf( 19, 23, "OK");
	*/
    
    // 셸을 시작
    kStartConsoleShell();
}

void kPrintStringMapped( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xAB8000;
    int i;
    
    // X, Y 옜� 옜옜 옜옜 옜� 옜옜� 옜
    pstScreen += ( iY * 80 ) + iX;

    // NULL� 옜 옜� 옜� 옜
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}

void kRWROTest(){

	DWORD **kernel1fe;
	DWORD **kernel1ff;
	kernel1fe = 0x1fe000;

	kPrintStringMapped( 0, 14, "Read from 0x1fe000 [  ]");

	if( *kernel1fe == 0x050011FE ){				// OS Life~~
		kPrintStringMapped( 20, 14, "OK");
		kPrintStringMapped( 0, 15, "Write to 0x1fe000 [  ]");
	}
	else{
		kPrintStringMapped( 20, 14, "FAIL]");
		while(1);
	}
	*kernel1fe = 0xDEADBEEF;
	kPrintStringMapped( 19, 15, "OK");
	kPrintStringMapped( 0, 16, "Read from 0x1ff000 [  ]");

	kernel1ff = 0x1ff000;
	if( *kernel1ff == 0xCAFEBABE ){
		kPrintStringMapped( 20, 16, "OK");
	}
	else{
		kPrintStringMapped( 20, 16, "FAIL]");
		while(1);
	}
	// write to read-only page(0x1ff000)
	/**	kPrintStringMapped( 0, 18, "Write to 0x1ff000 [  ]");
	 *	*kernel1ff = 0xDEADBEEF;
	 *	kPrintStringMapped( 19, 18, "OK");
	 */

	return;
}

