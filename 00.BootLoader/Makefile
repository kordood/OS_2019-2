all: BootLoader.bin

BootLoader.bin: BootLoader.asm
	nasm -o BootLoader1.bin BootLoader1.asm
	nasm -o BootLoader2.bin BootLoader2.asm
	
clean:
	rm -f BootLoader.bin
