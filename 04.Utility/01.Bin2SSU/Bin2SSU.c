#include "Bin2SSU.h"

int main(int argc, char *argv[]){
	uint8_t *file_name, *ssu_file_name;
	uint32_t *header, *binary, buffer[BUFFER_MAX];
	int fp, ssu_fp;
	ssize_t rd_size;
	off_t file_size, data_offset;


	if(argc != 3) {
		if(argc == 2) {
			file_name = argv[1];
			if (0 < ( fp = open(file_name, O_RDONLY))) {
		   		puts("\n");
		   		while(0 < (rd_size = read(fp, buffer, BUFFER_MAX-1))) {
			   		buffer[rd_size]  = '\0';      // puts() end NULL
			   		puts(buffer);
		   		}
		   		close(fp);
		   	}
		}
		else{
			printf("Usage : %s <file name> <data section address>\n", argv[0]);
			exit(1);
		}
	}

	file_name = argv[1];
	printf("your file: %s\n", file_name);

	data_offset = getDataOffset(argv[2]);
	if(data_offset <= 0) {
		printf("data offset is too small!\n");
		exit(1);
	}

	fp = open(file_name, O_RDONLY);

	if(fp == -1) {
		printf("No such file : %s\n", file_name);
		exit(1);
	}
	else if(fp < 0) {
		printf("File read error : %s\n", file_name);
		exit(1);
	}
	else {
		file_size = getFileSize(fp);
		if(file_size < 0) {
			printf("file is too small!\n");
			exit(1);
		}

		ssu_file_name = mmap(0, FILENAME_MAX, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);		// Allocate header
		appendSSU(ssu_file_name, file_name);
		ssu_fp = open(ssu_file_name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);							// open .ssu file

		header = mmap(0, HEADER_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);		// Allocate header
		initializeHeader(header, file_size, data_offset);
		write(ssu_fp, header, HEADER_SIZE);

		binary = mmap(0, file_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if(readFile(fp, binary, file_size) < 0){
			printf("binary read error!\n");
			exit(1);
		}

		write(ssu_fp, binary, file_size);
	}

	close(ssu_fp);
	close(fp);
	return 0;
}

int endianCheck(){
	int fd;
	uint32_t endian = 0x12345678;
	uint32_t check;
	fd = open("endian.tmp", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if(fd < 0){
		printf("endian check open error\n");
	}
	write(fd, &endian, sizeof(uint32_t));
	close(fd);

	fd = open("endian.tmp", O_RDONLY);
	if(fd < 0){
		printf("endian check open error\n");
	}
	read(fd, &check, sizeof(uint32_t));

	printf("endian: %x, check: %x\n", endian, check);
	if(check == endian){
		return 1;
	}
	else{
		return 0;
	}
}

void appendSSU(uint8_t *dest, uint8_t *filename){
	sprintf(dest, "%s.ssu", filename);
}

void initializeHeader(uint32_t *header, uint32_t fileSize, uint32_t dataOffset){
	uint32_t magic_number;
	uint32_t header_size;
	uint32_t file_size;
	uint32_t endian;
	uint32_t version;
	uint32_t bits;
	uint32_t text_offset;
	uint32_t data_offset;

	if(endianCheck()){
		magic_number	= 0x5553537F;
		header_size	= 0x20000000;
		file_size		= 0x00000000;
		endian			= 0x78563412;
		version		= 0x01000000;
		bits			= 0x40000000;
		text_offset	= 0x20000000;
		data_offset	= 0x00000000;
		uint32_t tmp = fileSize + 0x00000020;

		uint32_t t0, t1, t2, t3;
		t0 = (tmp>>24) & 0xFF;
		t1 = (tmp>>16) & 0xFF;
		t2 = (tmp>>8) & 0xFF;
		t3 = tmp & 0xFF;
		t1 = t1 << 8;
		t2 = t2 << 16;
		t3 = t3 << 24;
		tmp = t0 + t1 + t2 + t3;
		file_size = tmp;

		tmp = dataOffset + 0x00000020;
		t0 = (tmp>>24) & 0xFF;
		t1 = (tmp>>16) & 0xFF;
		t2 = (tmp>>8) & 0xFF;
		t3 = tmp & 0xFF;
		t1 = t1 << 8;
		t2 = t2 << 16;
		t3 = t3 << 24;
		tmp = t0 + t1 + t2 + t3;
		data_offset = tmp;
	}
	else{
		magic_number	= 0x7F535355;
		header_size	= 0x00000020;
		file_size		= fileSize;
		endian			= 0x12345678;
		version		= 0x00000001;
		bits			= 0x00000040;
		text_offset	= 0x00000020;
		data_offset	= dataOffset + header_size;
	}

	uint32_t *ptr;
	ptr = header;

	memcpy(ptr, &magic_number, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &header_size, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &file_size, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &endian, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &version, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &bits, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &text_offset, sizeof(uint32_t));
	ptr++;
	memcpy(ptr, &data_offset, sizeof(uint32_t));
}

int readFile(int fp, uint32_t *dest, off_t fileSize){								// return read size
	uint32_t *buffer;
	off_t rd_size;

	lseek(fp, 0, SEEK_SET);
	buffer = mmap(0, fileSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	while(0 < (rd_size = read(fp, buffer, fileSize))) {
		memcpy(dest, buffer, fileSize);
		dest += fileSize / sizeof(uint32_t *);
	}

	return 0;
}

off_t getDataOffset(uint8_t *arg){
	if(arg[0] == '0' && arg[1] == 'x'){
		return strtol(arg, NULL, 16);
	}
	else{
		return atoi(arg);
	}
}

off_t getFileSize(int fp){
	off_t size;
	size = lseek(fp, 0, SEEK_END);
	if(size <= 0){
		return -1;
	}
	else{
		return size;
	}
}