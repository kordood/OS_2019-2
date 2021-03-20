#ifndef __BIN2SSU_H__
#define __BIN2SSU_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILENAME_MAX 512
#define BUFFER_MAX 1024

#define HEADER_SIZE 0x20

int endianCheck();
void appendSSU(uint8_t *dest, uint8_t *filename);
void initializeHeader(uint32_t *header, uint32_t fileSize, uint32_t dataOffset);
int readFile(int fp, uint32_t *dest, off_t fileSize);
off_t getDataOffset(uint8_t *arg);
off_t getFileSize(int fp);

#endif /*__BIN2SSU_H__*/

