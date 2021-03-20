//ViEditor.h

#ifndef __VIEDITOR_H__
#define __VIEDITOR_H__

#include "Types.h"
#include "Utility.h"
#include "Keyboard.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "FileSystem.h"

#define VI_DEFAULTTEXTCOLOR	0xF0

const void kViManager( const char* pcParameterBuffer );

void kSaveScreen();
void kClearColorScreen();
void kExecuteEditorCommand( const char* pcCommandBuffer, FILE* pstFile );

FILE* kViFileOpenManager(const char* pcFileName, const char* pcMode);
void kViSaveFile();
void kViShowFile(FILE* pstFile);

CHARACTER* saveScreen;

int iSaveY;

#endif
