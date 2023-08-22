#include"loader.h"
#include "memory.h"


static char programName[MAXSTRLEN] = { NUL };

static unsigned short dataAddress = 0;

void processRecord1(unsigned char length, int startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber) {
	unsigned char sum = length + startAddressHigh + startAddressLow;
	unsigned int index = 8, byte;
	//printf("This S1 record has the following byte values stored in the respective memory locations\n");

	for (int i = 0; i < length - 3; i++) {
		sscanf(&buffer[index], "%2x", &byte);
		byte = byte & 0xFF;

		memoryByteWrite(dataAddress, byte);

		//printf("memory at location %X is %.2X\n", dataAddress, memory.byte[dataAddress]);
		dataAddress++;
		sum += byte;
		index += 2;
	}

	sscanf(&buffer[index], "%2x", &byte);
	byte = byte & 0xFF;
	if ((sum + byte) != 0xFF) {
		printf("Error in record's checksum in line %d as it isn't valid\n", lineNumber);
	}
}


void processRecord0(int length, unsigned char startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber) {
	unsigned char sum = length + startAddressHigh + startAddressLow;
	unsigned int index = 8, byte;

	for (int i = 0; i < length - 3; i++) {
		sscanf(&buffer[index], "%2x", &byte);
		byte = byte & 0xFF;
		programName[i] = byte;
		sum += byte;
		index += 2;
	}

	sscanf(&buffer[index], "%2x", &byte);
	byte = byte & 0xFF;
	if ((sum + byte) != 0xFF) {
		printf("Error in record's checksum in line %d as it isn't valid\n", lineNumber);
	}
}

void processRecord9(unsigned char length, unsigned char startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber) {
	unsigned char sum = length + startAddressHigh + startAddressLow;
	unsigned int index = 8, byte;

	sscanf(&buffer[index], "%2x", &byte);
	byte = byte & 0xFF;
	if ((sum + byte) != 0xFF) {
		printf("Error in record's checksum in line %d as it isn't valid\n", lineNumber);
	}
}

void loader(FILE* fptr) {
	char buffer[MAXSTRLEN];
	unsigned int higherstartAddress = 0, lowerstartAddress = 0, recordLength = 0, startAddress = 0, i = 0;

	PC = 0;

	while (fgets(buffer, MAXSTRLEN, fptr) != NULL) {
		i++;
		if (buffer[FIRSTCHARACTER] != 'S') {
			printf("First character in the S record in line number %d isn't an S\n", i);
		}
		else if (strcmp(buffer, "\n") == 0) {
			printf("S record in line number %d is empty S\n", i);
		}
		else {
			sscanf(&buffer[THIRDCHARACTER], "%2x%2x%2x", &recordLength, &higherstartAddress, &lowerstartAddress);
			startAddress = higherstartAddress << 8 | lowerstartAddress;
			switch (buffer[SECONDCHARACTER]) {
			case '0':
				//process S0 record
				processRecord0(recordLength, higherstartAddress, lowerstartAddress, buffer, i);
				break;
			case '1':
				//process S1 record
				dataAddress = startAddress;
				processRecord1(recordLength, higherstartAddress, lowerstartAddress, buffer, i);
				break;
			case '9':
				//process S9 record
				PC = startAddress;
				processRecord9(recordLength, higherstartAddress, lowerstartAddress, buffer, i);
				break;
			default:
				printf("Error in S-record at line %d since it isn't an S0, S1, or S9 record\n", i);
				break;
			}
		}
	}

	printf("Name of the program is %s\n", programName);
	printf("The program counter's starting address is %X\n", PC);

	if (i == 0) {
		printf("Input file is empty\n");
	}
}