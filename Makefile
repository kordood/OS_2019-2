all: BootLoader Kernel32 Disk.img

BootLoader:
	@echo 
	@echo ============== Build Boot Loader ===============
	@echo 
	
	make -C 00.BootLoader

	@echo 
	@echo =============== Build Complete ===============
	@echo 

Kernel32:
	@echo 
	@echo ============== Build 32bit Kernel ===============
	@echo 
	
	make -C 01.Kernel32

	@echo 
	@echo =============== Build Complete ===============
	@echo 
	
Disk.img: BootLoader Kernel32
	@echo 
	@echo =========== Disk Image Build Start ===========
	@echo 

	cat 00.BootLoader/BootLoader1.bin 00.BootLoader/BootLoader2.bin 01.Kernel32/VirtualOS.bin> Disk.img

	@echo 
	@echo ============= All Build Complete =============
	@echo 

run:
	qemu-system-x86_64 -L . -fda Disk.img -m 64 -localtime -M pc -rtc base=localtime 
	
clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	rm -f Disk.img	
