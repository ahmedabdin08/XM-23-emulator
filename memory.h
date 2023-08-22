#pragma once
/* Header to for memory utility functions and for the bus
/* Author: Ahmed Khairallah
*/
#include <stdio.h>
#include <string.h>

#define MEMORYBYTESIZE 1<<16
#define MEMORYWORDSIZE 1<<15
#define EVENMASK 0xFFFE
#define DEVICEMEMORYMAX 0x0F

#pragma warning(disable: 4996)

typedef union memory{
	unsigned char byte[MEMORYBYTESIZE];
	unsigned short word[MEMORYWORDSIZE];

} mem;

mem memory;

typedef union word_byte {
	unsigned short word;
	unsigned char byte[2];
} word_byte;

#define B7(x) x>>7 & 0x01
#define B15(x) x>>15 & 0x01
#define B8(x) x>>8 &0x01
#define B9(x) x>>9 &0x01

typedef enum{READ, WRITE} ReadOrWrite;
typedef enum{WORD, BYTE} Size;

void memoryByteRead(unsigned short address, unsigned char* data);
void memoryByteWrite(unsigned short address, unsigned char data);
void memoryWordRead(unsigned short address, unsigned short* data);
void memoryWordWrite(unsigned short address, unsigned short data);
void printWordInMemory(unsigned short address);
void printMemoryRange(unsigned short startAddress, unsigned short endAddress);
void bus(unsigned short mar, unsigned short* mbr, ReadOrWrite rw, Size wb);
void initializeMemory();



enum {
	WORDFFFF = 0xFFFF,
};