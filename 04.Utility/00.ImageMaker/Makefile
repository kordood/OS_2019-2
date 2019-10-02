# file		makefile
# date		2008/12/16
# author	kkamagui 
# brief		디스크 이미지를 생성하는 프로그램을 빌드하는 make 파일

all: ImageMaker

ImageMaker: ImageMaker.c
	gcc -o $@ $<
	cp ImageMaker ../../
	
clean:
	rm -f ImageMaker
