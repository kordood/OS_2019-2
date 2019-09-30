# file		makefile
# date		2019/09/30
# author	kordood 
# brief		32-bit kernel make file	

all: Kernel32.bin

Kernel32.bin: Source/EntryPoint.s
	nasm -o Kernel32.bin $<

clean:
	rm -f Kernel32.bin
