#include "ViEditor.h"
#include "Utility.h"
#include "Keyboard.h"
#include "Console.h"
#include "ConsoleShell.h"

const void kViManager( const char* pcParameterBuffer ){
	PARAMETERLIST stList;
	char vcFileName[ 30 ];

	// Keyboard
	char vcEditorCommandBuffer[ CONSOLE_WIDTH - 1];
	int iEditorCommandBufferIndex = 0;
	char vcEditorBuffer[ CONSOLE_WIDTH * (CONSOLE_HEIGHT - 2)];
	int iEditorBufferIndex = 0;

	int iCursorX, iCursorY;
	int iSaveEditorCursorX = 0, iSaveEditorCursorY = 1;

	BYTE bKey;

	// Parameter
	kInitializeParameter( &stList, pcParameterBuffer );
	kGetNextParameter( &stList, vcFileName );

	if(vcFileName == NULL){
		kPrintf("input filename\n");
		return;
	}
	else{
		// create filename.tmp
	}

	// save cursor
	kGetCursor( NULL, &iSaveY );

	kSaveScreen();
	kClearColorScreen();
	kSetCursor( 0, 24);

	while(1){
		bKey = kGetCh();

		// edit mode
		if((iEditorCommandBufferIndex == 0) && (( bKey == 'i') || (bKey == 'o'))){
			if(bKey == 'i') kSetCursor( iSaveEditorCursorX, iSaveEditorCursorY );
			else if(bKey == 'o'){
				iSaveEditorCursorX = 0;
				kSetCursor( iSaveEditorCursorX, ++iSaveEditorCursorY );
			}
			while(1){
				bKey = kGetCh();

				if( bKey == KEY_ESC ){
					kGetCursor( &iSaveEditorCursorX, &iSaveEditorCursorY );
					kSetCursor( 0, 24);
					break;
				}
				else if( (bKey == KEY_BACKSPACE)){
					kGetCursor( &iCursorX, &iCursorY );
					kPrintStringXY( iCursorX - 1, iCursorY, " ");
					kSetCursor( iCursorX - 1, iCursorY );
					vcEditorCommandBuffer[ --iEditorCommandBufferIndex ] = '\0';
				}
				else if( bKey == KEY_UP){
					kGetCursor(&iSaveEditorCursorX, &iSaveEditorCursorY);
					kSetCursor(iSaveEditorCursorX, --iSaveEditorCursorY);
				}
				else if( bKey == KEY_DOWN){
					kGetCursor(&iSaveEditorCursorX, &iSaveEditorCursorY);
					kSetCursor(iSaveEditorCursorX, ++iSaveEditorCursorY);
				}
				else if( bKey == KEY_LEFT){
					kGetCursor(&iSaveEditorCursorX, &iSaveEditorCursorY);
					kSetCursor(--iSaveEditorCursorX, iSaveEditorCursorY);
				}
				else if( bKey == KEY_RIGHT){
					kGetCursor(&iSaveEditorCursorX, &iSaveEditorCursorY);
					kSetCursor(++iSaveEditorCursorX, iSaveEditorCursorY);
				}
				else if(iEditorBufferIndex < CONSOLE_WIDTH * CONSOLE_HEIGHT){
					vcEditorBuffer[ iEditorBufferIndex++ ] = bKey;
					kPrintf( "%c", bKey );
				}	
			}
		}	
		// commend mode
		else{
			// delete
			if( (bKey == KEY_BACKSPACE)){
				kGetCursor( &iCursorX, &iCursorY );
				kPrintStringXY( iCursorX - 1, iCursorY, " ");
				kSetCursor( iCursorX - 1, iCursorY );
				vcEditorCommandBuffer[ --iEditorCommandBufferIndex ] = '\0';
			}
			else if( bKey == KEY_ENTER ){
				for(int i = 0; i < iEditorCommandBufferIndex; i++){
					kGetCursor( &iCursorX, &iCursorY);
					kPrintStringXY( iCursorX - 1, iCursorY, " ");
					kSetCursor( iCursorX - 1, iCursorY );
				}
				if( iEditorCommandBufferIndex > 0 ){
					// :q
					if(kMemCmp(vcEditorCommandBuffer, ":q", iEditorCommandBufferIndex) == 0){
						CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;

						for(int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++){
							pstScreen[i].bCharactor = saveScreen[i].bCharactor;
							saveScreen[i].bCharactor = '\0';
							pstScreen[i].bAttribute = saveScreen[i].bAttribute;
							saveScreen[i].bAttribute = '\0';
						}
						kSetCursor(0, iSaveY);
						return;
					}
					else{
						vcEditorCommandBuffer[ iEditorCommandBufferIndex ] = '\0';
						kExecuteEditorCommand( vcEditorCommandBuffer );
					}
				}

				kMemSet( vcEditorCommandBuffer, '\0', CONSOLE_WIDTH);
				iEditorCommandBufferIndex = 0;
			}
			else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
					( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
					( bKey == KEY_SCROLLLOCK ) || (bKey == KEY_CTRL ) ||
					( bKey == KEY_LALT )){}

			// command
			else if(iEditorCommandBufferIndex < CONSOLE_WIDTH){
				if(bKey == KEY_ESC){
					for(int i = 0; i < iEditorCommandBufferIndex; i++){
						kGetCursor( &iCursorX, &iCursorY);
						kPrintStringXY( iCursorX - 1, iCursorY, " ");
						kSetCursor( iCursorX - 1, iCursorY );
					}
					kMemSet( vcEditorCommandBuffer, '\0', CONSOLE_WIDTH);
					iEditorCommandBufferIndex = 0;
				}
				else{
					kGetCursor(&iCursorX, &iCursorY);
					int tmpX = iCursorX;

					if((iEditorCommandBufferIndex == 0) && (tmpX != 0)){
						for(int i = 0; i < tmpX; i++){
						kGetCursor(&iCursorX, &iCursorY);
						kPrintStringXY( iCursorX - 1, iCursorY, " ");
						kSetCursor( iCursorX - 1, iCursorY );
						}
					}
					vcEditorCommandBuffer[ iEditorCommandBufferIndex++ ] = bKey;
					kPrintf( "%c", bKey );


				}	
			}
		}
	}
}

void kExecuteEditorCommand( const char* pcEditorCommandBuffer){
	int iEditorCommandLength = kStrLen( pcEditorCommandBuffer );
	if(kMemCmp(pcEditorCommandBuffer, ":wq", iEditorCommandLength) == 0){

	}
	else{
		kPrintf("Not an editor command: [%s]", pcEditorCommandBuffer);
	}
}

void kSaveScreen(){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	saveScreen = (CHARACTER*) 0xB9000;

	for(int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++){
		saveScreen[i].bCharactor = pstScreen[i].bCharactor;
		saveScreen[i].bAttribute = pstScreen[i].bAttribute;
	}
}

void kClearColorScreen(){
	CHARACTER* colorScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;

	for(int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++){
		colorScreen[ i ].bCharactor = ' ';
		colorScreen[ i ].bAttribute = VI_DEFAULTTEXTCOLOR;
	}
}

