//ViEditor.h

#ifndef __VIEDITOR_H__
#define __VIEDITOR_H__

#include "Types.h"

#define VI_DEFAULTTEXTCOLOR	0xF0

const void kViManager( const char* pcParameterBuffer );

void kSaveScreen();
void kClearColorScreen();
void kExecuteEditorCommand( const char* pcCommandBuffer );


CHARACTER* saveScreen;

int iSaveY;

#endif
