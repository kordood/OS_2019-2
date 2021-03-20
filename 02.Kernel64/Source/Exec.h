/**
 *  file    Exec.h
 *  date    2019/12/01
 *  author  Liberty 
 *          Copyright(c)2019 All rights reserved by Liberty
 *  brief   Functional support of exec()
 */

#ifndef __EXEC_H__
#define __EXEC_H__

#include "Types.h"
#include "Utility.h"
#include "FileSystem.h"
#include "DynamicMemory.h"

#define BLOCK_SIZE 0x1000

typedef struct SSUHEADER
{
	DWORD magic_number;
	DWORD header_size;
	DWORD file_size;
	DWORD endian;
	DWORD version;
	DWORD bits;
	DWORD text_offset;
	DWORD data_offset;
}SSUHEADER;

void kExecveTest();
void kExecve(QWORD qwEntryPoint, char *filePath);
void *kFileLoad(char *loadingAddress, char *filePath);
void kSSUParser(SSUHEADER *header, DWORD *src);
void headerParse(SSUHEADER *header, DWORD *src, int size);
void kChangeEndian(DWORD *ptr, int qwSize);
void insertbinary();

#endif /*__EXEC_H__*/
