#include "Exec.h"
#include "Console.h"

/*
void kExecveTest(){
	kExecve(0x1200000, "binary.txt");
}*/

void kExecve(QWORD qwEntryPoint, char *filePath){		// need to add filepath
	//insertbinary();		// example
	DWORD *ptr, *src;
	char *file_load_temp;
    file_load_temp = kAllocateMemory( BLOCK_SIZE );
/*	src = 0x1fc000;
	ptr = 0x1049000;
*/	src = kFileLoad(file_load_temp, filePath);
	ptr = qwEntryPoint;
	SSUHEADER header;
	kSSUParser(&header, src);
	kMemCpy(ptr, (src + ( header.header_size / sizeof(DWORD))), (header.file_size - header.header_size ));
    kFreeMemory(src);
    
	//QWORD val = (QWORD)(header.data_offset - header.header_size) + 0x1049000;
	QWORD val = (QWORD)(header.data_offset - header.header_size) + qwEntryPoint;
	QWORD RIP = qwEntryPoint;
	__asm__ __volatile__(
        "push %%rbp \n\t"
        "mov %%rsp, %%rbp \n\t"
        "call %%rsi \n\t"
        "pop %%rbp \n\t"
        "add $0x10, %%rbp \n\t"
        "mov %%rbp, %%rsp \n\t"
        "ret \n\t"
	: : "D" (val), "S" (RIP) : "memory");
}

void *kFileLoad(char *loadingAddress, char *filePath){
    FILE* fp;

    fp = fopen( filePath, "r" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", filePath );
        return ;
    }
	fread(loadingAddress, 1, BLOCK_SIZE, fp);
	kChangeEndian(loadingAddress, BLOCK_SIZE);
    return loadingAddress;
}

void kSSUParser(SSUHEADER *header, DWORD *src){
	DWORD tmp = *(src + 3);		// endian
	if(tmp == 0x12345678) {
		DWORD binary_size = *(src + 2) - *(src + 1) + (sizeof(DWORD *));
		kChangeEndian(src + 0x08, binary_size);
        DWORD * ptr;
        ptr = src;
	}
    else{
        kChangeEndian(src, 0x20);
    }
	headerParse(header, src, sizeof(SSUHEADER));
	if(header->endian == 0x12345678) {
		DWORD data_size = header->file_size - header->text_offset;
        DWORD offset = header->data_offset / sizeof(DWORD);
        if(header->data_offset % sizeof(DWORD)){
            offset++;
        }
		kChangeEndian(src + offset, data_size);
	}
}

void headerParse(SSUHEADER *header, DWORD *src, int size){
	DWORD *ptr;
	ptr = src;
	header->magic_number = *ptr;
	ptr++;
	header->header_size = *ptr;
	kMemCpy(header, src, header->header_size);

	return;
}

void kChangeEndian(DWORD *ptr, int qwSize){
	DWORD tmp = 0;
	DWORD t0, t1, t2, t3;
    int count = qwSize / sizeof(DWORD *);
    if(qwSize % sizeof(DWORD *)){
        count++;
    }
	for(int i = 0; i < count; i++){	// 0 1 2 3 > 3 2 1 0
		tmp  = *ptr;
		t0 = (tmp>>24) & 0xFF;
		t1 = (tmp>>16) & 0xFF;
		t2 = (tmp>>8) & 0xFF;
		t3 = tmp & 0xFF;

		t1 = t1 << 8;
		t2 = t2 << 16;
		t3 = t3 << 24;
		tmp = t0 + t1 + t2 + t3;
		*ptr = tmp;
		ptr++;
	}
	return;
}

void insertbinary(){			// example
    DWORD *ptr;
    ptr = 0x1fc000;
    DWORD array[28];
    array[0] = 0x7F535355;      // magic_number
    array[1] = 0x00000020;      // header_size
    array[2] = 0x00000070;      // file_size
    array[3] = 0x12345678;      // endian
    array[4] = 0x00000001;      // version
    array[5] = 0x00000040;      // bits
    array[6] = 0x00000020;      // text_offset
    array[7] = 0x00000050;      // data_offset

    array[8] = 0x554889E5;
    array[9] = 0x5657488B;
    array[10] = 0x5D104889;
    array[11] = 0xDEB80080;

    array[12] = 0x0B004889;
    array[13] = 0xC78A0E83;
    array[14] = 0xF900740C;
    array[15] = 0x880F4883;

    array[16] = 0xC6014883;

    array[17] = 0xC702EBED;
    array[18] = 0x5F5E4889;
    array[19] = 0xEC5DEBFE;

    array[20] = 0x45786563;
    array[21] = 0x76652053;
    array[22] = 0x75636365;
    array[23] = 0x73736675;
    array[24] = 0x6C6C7920;
    array[25] = 0x53746172;
    array[26] = 0x74656421;
    array[27] = 0x21210000;
    for(int i = 0; i < 28; i++){
        *ptr = array[i];
        ptr++;
    }
}
/*

    DWORD array[35];
    array[0] = 0x7F535355;      // magic_number
    array[1] = 0x00000020;      // header_size
    array[2] = 0x0000008A;      // file_size
    array[3] = 0x12345678;      // endian
    array[4] = 0x00000001;      // version
    array[5] = 0x00000040;      // bits
    array[6] = 0x00000020;      // text_offset
    array[7] = 0x0000006C;      // data_offset

    array[8] = 0x554889E5;
    array[9] = 0x5657488B;
    array[10] = 0x5D104889;
    array[11] = 0xDEB85080;


    array[12] = 0x0B004889;
    array[13] = 0xC7C60700;
    array[14] = 0xC647010A;
    array[15] = 0x4883C702;
    array[16] = 0x4881FFB0;
    array[17] = 0x8F0B007C;
    array[18] = 0xECB80080;


    array[19] = 0x0B004889;
    array[20] = 0xC78A0E83;
    array[21] = 0xF900740C;
    array[22] = 0x880F4883;

    array[23] = 0xC6014883;
    array[24] = 0xC702EBED;
    array[25] = 0x5F5E4889;
    array[26] = 0xEC5DEBFE;

    array[27] = 0x45786563;
    array[28] = 0x76652053;
    array[29] = 0x75636365;
    array[30] = 0x73736675;
    array[31] = 0x6C6C7920;
    array[32] = 0x53746172;
    array[33] = 0x74656421;
    array[34] = 0x21000000;
*/
/*
    DWORD array[36];
    array[0] = 0x7F535355;      // magic_number
    array[1] = 0x00000020;      // header_size
    array[2] = 0x0000008E;      // file_size
    array[3] = 0x12345678;      // endian
    array[4] = 0x00000001;      // version
    array[5] = 0x00000040;      // bits
    array[6] = 0x00000020;      // text_offset
    array[7] = 0x00000070;      // data_offset

    array[8] = 0x554889E5;
    array[9] = 0x5657488B;
    array[10] = 0x5D104889;
    array[11] = 0xDEB80080;

    array[12] = 0x0B004889;
    array[13] = 0xC7C60700;
    array[14] = 0xC647010A;
    array[15] = 0x4883C702;

    array[16] = 0x4881FFB0;
    array[17] = 0x8F0B007C;
    array[18] = 0xECB80080;
    array[19] = 0x0B004889;

    array[20] = 0xC78A0E83;
    array[21] = 0xF900740C;
    array[22] = 0x880FC647;
    array[23] = 0x010a4883;
    array[24] = 0xC6014883;

    array[25] = 0xC702EBED;
    array[26] = 0x5F5E4889;

    array[27] = 0xEC5DEBFE;
    array[28] = 0x45786563;
    array[29] = 0x76652053;
    array[30] = 0x75636365;
    array[31] = 0x73736675;
    array[32] = 0x6C6C7920;
    array[33] = 0x53746172;
    array[34] = 0x74656421;
    array[35] = 0x21000000;*/
/*
7F 53 53 55 00 00 00 20 00 00 00 82 12 34 56 78
00 00 00 01 00 00 00 40 00 00 00 20 00 00 00 64

55 48 89 E5 56 57 48 8B 5D 10 48 89 DE B8 00 80
0B 00 48 89 C7 C6 07 00 C6 47 01 0A 48 83 C7 02
48 81 FF B0 8F 0B 00 7C EC B8 00 80 0B 00 48 89
C7 8A 0E 83 F9 00 74 0C 

88 0F 48 83 C6 01 48 83
C7 02 EB ED 5F 5E 48 89 EC 5D EB FE 45 78 65 63
76 65 20 53 75 63 63 65 73 73 66 75 6C 6C 79 20
53 74 61 72 74 65 64 21 21 00


*/

/*

    DWORD array[28];
    array[0] = 0x7F535355;		// magic_number
    array[1] = 0x00000020;		// header_size
    array[2] = 0x00000070;		// file_size
    array[3] = 0x12345678;		// endian
    array[4] = 0x00000001;		// version
    array[5] = 0x00000040;		// bits
    array[6] = 0x00000020;		// text_offset
    array[7] = 0x00000050;		// data_offset
    array[8] = 0x554889E5;
    array[9] = 0x5657488B;
    array[10] = 0x5D104889;
    array[11] = 0xDEB80080;
    array[12] = 0x0B004889;
    array[13] = 0xC78A0E83;
    array[14] = 0xF900740C;
    array[15] = 0x880F4883;
    array[16] = 0xC6014883;
    array[17] = 0xC702EBED;
    array[18] = 0x5F5E4889;
    array[19] = 0xEC5DEBFE;

    array[20] = 0x45786563;
    array[21] = 0x76652053;
    array[22] = 0x75636365;
    array[23] = 0x73736675;
    array[24] = 0x6C6C7920;
    array[25] = 0x53746172;
    array[26] = 0x74656421;
    array[27] = 0x21210000;
7F 53 53 55 00 00 00 20 00 00 00 6E 12 34 56 78
00 00 00 01 00 00 00 40 00 00 00 20 00 00 00 50

55 48 89 E5 56 57 48 8B 5D 10 48 89 DE B8 00 80
0B 00 48 89 C7 8A 0E 83 F9 00 74 0C 88 0F 48 83
C6 01 48 83 C7 02 EB ED 5F 5E 48 89 EC 5D EB FE
45 78 65 63 76 65 20 53 75 63 63 65 73 73 66 75
6C 6C 79 20 53 74 61 72 74 65 64 21 21 00


*/