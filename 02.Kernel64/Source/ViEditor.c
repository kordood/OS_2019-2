
#include "ViEditor.h"

FILE* kViFileOpenManager(const char *pcFileName, const char* pcMode){

	return kOpenFile(pcFileName,pcMode);

	//	kLock( &( gs_stFileSystemManager.stMutex ) );


	/*	int iDirectoryEntryOffset;
		DIRECTORYENTRY stEntry;


	// 파일이 존재하는가 확인
	iDirectoryEntryOffset = knFindDirectoryEntry( pcFileName, &stEntry );	


	//파일 존재 안함
	if(iDirectoryEntryOffset == -1){
	if(knCreateFile( pcFileName, &stEntry, &iDirectoryEntryOffset ) == FALSE){
	//			kUnlock( & ( gs_stFileSystemManager.stMutex ) );
	return NULL ; 
	}


	pstFile = knAllocateFileDirectoryHandle();

	if(pstFile = NULL){
	//			kUnlock( & ( gs_stFileSystemManager.stMutex ) );
	return NULL;
	}

	pstFile->bType = FILESYSTEM_TYPE_FILE;
	pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
	pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
	pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
	pstFile->stFileHandle.dwCurrentOffset = 0;

	//		kUnlock( & ( gs_stFileSystemManager.stMutex ) );
	}

	//파일 존재 함
	else{
	pstFile = fopen(pcFileName, "r");
	//kReadFile(void* buffer, DWORD dwSize, DWORD dwCount, FILE* pstFile)
	}

	return pstFile;

*/

}


void kViShowFile( FILE* pstFile ){

	int iEnterCount = 0;

	BYTE bKey;

	while( 1 )
	{
		if( fread( &bKey, 1, 1, pstFile ) != 1 )
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

	fclose( pstFile );

}

void kSaveFile(FILE* pstFile){

}


const void kViManager( const char* pcParameterBuffer ){

	PARAMETERLIST stList;

	char pcFileName[ 50 ];
	int iFileNameLength;

	FILE* pstFile;

	char vcEditorCommandBuffer[ CONSOLE_WIDTH - 1];
	int iEditorCommandBufferIndex = 0;
	char vcEditorBuffer[ CONSOLE_WIDTH * (CONSOLE_HEIGHT - 2)];
	int iEditorBufferIndex = 0;

	int iCursorX, iCursorY;
	int iSaveEditorCursorX = 0, iSaveEditorCursorY = 1;

	BYTE bKey;

	kInitializeParameter( &stList, pcParameterBuffer );

	iFileNameLength = kGetNextParameter( &stList, pcFileName );

	if( ( iFileNameLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iFileNameLength == 0 ) )
	{
		kPrintf( "Too Long or Too Short File Name\n" );
		return NULL;
	}
	pcFileName[ iFileNameLength ] = '\0';

	pstFile = kViFileOpenManager(pcFileName, "w");

	//	kViShowFile(pstFile);

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
				}else if( bKey == KEY_ENTER){
					kGetCursor( &iCursorX, &iCursorY );
					kSetCursor( 0, iCursorY + 1);
				}else if( bKey == KEY_UP){
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
				}else if(iEditorBufferIndex < CONSOLE_WIDTH * CONSOLE_HEIGHT){
					if(('9' >= bKey) && (bKey >= '0') || ('F' >= bKey) && (bKey >= 'A') ||
							('f' >= bKey) && (bKey >= 'a')){
						vcEditorBuffer[ iEditorBufferIndex++ ] = bKey;
						kPrintf( "%c", bKey );
					}
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
						kExecuteEditorCommand( vcEditorCommandBuffer, pstFile );
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

void kExecuteEditorCommand( const char* pcEditorCommandBuffer, FILE* pstFile){
	int iEditorCommandLength = kStrLen( pcEditorCommandBuffer );
	int cnt = 0;
	int writecnt = 0;
	int flag = 0;
	int cntt= 0;

	if(kMemCmp(pcEditorCommandBuffer, ":w", iEditorCommandLength + 1) == 0){
		CHARACTER* fileScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
		BYTE bFileBuffer[ CONSOLE_WIDTH * 23 ] = {'\0',};
		BYTE bTmp = '\0';
		BYTE bWrite = '\0';
		BYTE bChar;

		for(int i = CONSOLE_WIDTH; i < CONSOLE_WIDTH * 21; i++){ 
			//if(fileScreen[i].bCharactor != '\0')
			bChar = fileScreen[i].bCharactor;
			if((('9' >= bChar) && (bChar >= '0'))||(('F' >= bChar) && (bChar >= 'A'))||(('f' >= bChar) && (bChar >= 'a')))
			{
				bFileBuffer[cntt++] = bChar;
			}
		}
		//for(int i = 0; i < cntt; i++) kPrintf("@");//kPrintf("%x, ", bFileBuffer[i]);

		for(int i = 0; i < cntt; i++){

			if(('9' >= bFileBuffer[i]) && (bFileBuffer[i] >= '0')){
				bTmp = bFileBuffer[i] - 48;
				bFileBuffer[i] = '\0';
				cnt++;
				if(flag == 1){
					bWrite |= bTmp;
					flag = 0;
				}
				else if(flag == 0){
					bWrite = bTmp<<4;
					flag = 1;
				}
			}
			else if(('F' >= bFileBuffer[i]) && (bFileBuffer[i] >= 'A')){
				bTmp = bFileBuffer[i] - 55;
				bFileBuffer[i] = '\0';
				cnt++;
				if(flag == 1){
					bWrite |= bTmp;
					flag = 0;
				}
				else if(flag == 0){
					bWrite = bTmp<<4;
					flag = 1;

				}
			}
			else if(('f' >= bFileBuffer[i]) && (bFileBuffer[i] >= 'a')){
				bTmp = bFileBuffer[i] - 87;
				bFileBuffer[i] = '\0';
				cnt++;
				if(flag == 1){
					bWrite |= bTmp;
					flag = 0;
				}
				else if(flag == 0){
					bWrite = bTmp<<4;
					flag = 1;

				}
			}

			if((i == cntt - 1) && (cntt % 2 == 1)){
				kWriteFile(&bWrite,1,1, pstFile);
				bWrite='\0';
				writecnt++;
				
			}
			if(flag == 0){
				kWriteFile(&bWrite,1,1, pstFile);
				bWrite='\0';
				writecnt++;

			}
			//			else if(flag == 0 && )
			bTmp = '\0';
			//bWrite = '\0';

		}




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

