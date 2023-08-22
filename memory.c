#include "memory.h"
#include <ctype.h>
#include <math.h>
#include "cpu.h"
#include "cache.h"
#include "devices.h"

#define NUL '\0'
#define MAXSTRLEN 256

void bus(unsigned short mar, unsigned short* mbr, ReadOrWrite rw, Size bw) {
	switch (rw) {
	case READ:
		if (bw == BYTE) memoryByteRead(mar, mbr);
		else if (bw == WORD) memoryWordRead(mar, mbr);
		else printf("Error not word or byte\n");
		break;
	case WRITE:
		if (bw == BYTE) memoryByteWrite(mar, *mbr);
		else if (bw == WORD) memoryWordWrite(mar, *mbr);
		else printf("Error not word or byte\n");
		break;
	default:
		printf("Error not read or write\n");
		break;
	}
	CPU_clock += 3;
}

void memoryByteRead(unsigned short address, unsigned char* data) {
	*data = memory.byte[address];
}

void memoryByteWrite(unsigned short address, unsigned char data) {
	memory.byte[address] = data;
}

void memoryWordRead(unsigned short address, unsigned short* data) {
	*data = memory.word[address >> 1];
}

void memoryWordWrite(unsigned short address, unsigned short data) {
	memory.word[address >> 1] = data;
}


void printWordInMemory(unsigned short address) {
	word_byte data = { 0 };
	memoryWordRead(address, &data.word);

	printf("mem[%.4X].word = #%.4X ", address, data.word);
	isprint(data.byte[1]) ? printf("%c", data.byte[1]) : printf(".");
	isprint(data.byte[0]) ? printf("%c", data.byte[0]) : printf(".");
	printf("\n");
}



void printMemoryRange(unsigned short startAddress, unsigned short endAddress) {
	do {
		printWordInMemory(startAddress);
		startAddress += 2;
	} while (startAddress <= endAddress);
}

void initializeMemory() {
	memset(cache, 0, CACHESIZE * sizeof(cache_line));
	memset(memory.byte, 0, MEMORYBYTESIZE);
	initializeMemoryMappedDevices(devs);
}