#include "userinterface.h"
#include "loader.h"
#include "cpu.h"
#include "cache.h"
#include "devices.h"
#pragma warning(disable: 4996)
enum{FALSE, TRUE};


unsigned short *breakPoint = NULL, oneStep = FALSE;
unsigned short exited = FALSE;

void printMenu() {
	printf("Choose a character to do the following\n");
	printf("G or g for running the CPU\n");
	printf("L or l for loading\n");
	printf("M or m for printing memory\n");
	printf("B or b for setting a breakpoint\n");
	printf("R or r for printing register content\n");
	printf("S or s for setting the register values\n");
	printf("P or p to display the PSW\n");
	printf("D or d for accessing CPU information\n");
	printf("I or i for toggling the single step\n");
	printf("X or x for exiting\n");
	printf("W or w for setting the write of the cache\n");
	printf("C or c for declaring which cache state to run\n");
	printf("T or t for inspecting the cache\n");
	printf("U or u for resetting the CPU and/or memory\n");
	printf("E or e for accessing external devices information\n");
	printf("? for printing the menu\n");
}

void userInterface() {
	printMenu();
	while (!exited) {
		characterRead();
	}
}
printRegisterContent() {
	printf("Register 0	= %.4X\n", regFile[REG][0]);
	printf("Register 1	= %.4X\n", regFile[REG][1]);
	printf("Register 2	= %.4X\n", regFile[REG][2]);
	printf("Register 3	= %.4X\n", regFile[REG][3]);
	printf("Register 4 (BP) = %.4X\n", regFile[REG][4]);
	printf("Register 5 (LR) = %.4X\n", regFile[REG][5]);
	printf("Register 6 (SP) = %.4X\n", regFile[REG][6]);
	printf("Register 7 (PC) = %.4X\n", regFile[REG][7]);
}

void characterRead() {
	char inputCharacter;
	if (scanf(" %c", &inputCharacter) != 1) {
		return;
	}
	switch (inputCharacter)
	{
	case 'g':
	case 'G':
		goCPU();
		break;
	case 'b':
	case 'B':
		setBreakPoint();
		break;
	case 'l':
	case 'L':
		loaderFile();
		break;
	case 'm':
	case 'M':
		printingMemory();
		break;
	case 'r':
	case 'R':
		printRegisterContent();
		break;
	case 'p':
	case 'P':
		displayPSW();
		break;
	case 'X':
	case 'x':
		exitEmulator();
		break;
	case 'I':
	case 'i':
		oneStep = !oneStep;
		oneStep ? printf("Single step enabled\n") : printf("Single step disabled\n");
		break;
	case 'D':
	case 'd':
		deviceInformation();
		break;
	case 's':
	case 'S':
		setRegisterValues();
		break;
	case 'C':
	case 'c':
		cachePolicy();
		break;
	case 'W':
	case 'w':
		cacheWrite();
		break;
	case 'T':
	case 't':
		inspectCache();
		break;
	case 'U':
	case 'u':
		resetCPU();
		break;
	case 'E':
	case 'e':
		viewDevices();
		break;
	case '?':
		printMenu();
		break;
	default:
		printf("Invalid input try again\n");
		break;
	}
}
void goCPU() {
	printf("Running the CPU\n");
	printf("Press control-C to exit\n");
	runCPU();
}
void loaderFile() {
	char buffer[MAXSTRLEN], cleanedBuffer[MAXSTRLEN];
	printf("Enter the file of the exutable to be loaded\n");
	getchar();
	int j = 0;
	scanf("%[^\n]s", buffer);

	// Loop to clean the buffer of any quotation marks
	for (int i = 0; i < strlen(buffer); i++) {
		if (buffer[i] != '\"') {
			cleanedBuffer[j++] = buffer[i];
		}
	}
	cleanedBuffer[j] = NUL;

	FILE* fptr = fopen(cleanedBuffer, "r");

	if (fptr == NULL) {
		printf("Error in opening file.\n");
		return;
	}
	loader(fptr);
	fclose(fptr);
}

void changeProgramCounter() {
	int pcValue = 0;
	printf("Enter desired program counter value: \n");
	if (scanf("%x", &pcValue) != 1){
		printf("Error in pc value input.\n");
		return;
	}
	PC = pcValue;
}

void cachePolicy() {
	char ch;
	printf("Enter H or h for hybrid\nEnter A or a for associative\nEnter D or d for direct\n");
	scanf(" %c", &ch);
	switch (ch)
	{
	case 'A':
	case 'a':
		cache_flag = ASSOCIATIVE;
		break;
	case 'D':
	case 'd':
		cache_flag = DIRECT;
		break;
	case 'H':
	case 'h':
	default:
		cache_flag = HYBRID;
		break;
	}
}

void cacheWrite() {
	char ch;
	printf("Enter B or b for Write Back\nEnter T or t for Write Through\n");
	scanf(" %c", &ch);
	switch (ch)
	{
	case 'B':
	case 'b':
		write_flag = WRITEBACK;
		break;
	case 'T':
	case 't':
	default:
		write_flag = WRITETHROUGH;
		break;
	}
}

void resetCPU() {
	unsigned char input, numberOfRegisters = 8;
	printf("Enter 1 for resetting memory, the CPU's registers, and the clock\nEnter 2 for resetting the CPU's registers and the clock\nEnter 3 for resetting memory\n");
	scanf(" %c", &input);
	switch (input)
	{
	case '2':
		CPU_clock = 0;
		initializeCPU();
		break;
	case '3':
		initializeMemory();
		break;
	case '1':
	default:
		CPU_clock = 0;
		initializeCPU();
		initializeMemory();
		break;
	}
}

void printingMemory() {
	unsigned int startAddress = 1, endAddress = 1;

	while (startAddress % 2 != 0 || endAddress % 2 != 0) {
		printf("Enter the starting address and the final address in hex that you would desire to see it's content. Make sure they are even addresses\n");
		if (scanf("%x%x", &startAddress, &endAddress) != 2) {
			printf("Error in reading the address\n");
			return;
		}

		if (startAddress % 2 != 0 || endAddress % 2 != 0) {
			printf("Address isn't even please try again\n");
		}
	}

	startAddress &= 0xFFFF;
	endAddress &= 0xFFFF;

	printMemoryRange(startAddress, endAddress);
}

void setBreakPoint() {
	unsigned int temp, numberOfBreakPoints =0;
	printf("Enter number of break points\n");
	if (scanf("%d", &numberOfBreakPoints) != 1) {
		printf("Error in number of breakpoints");
	}
	breakPointCount = numberOfBreakPoints;
	if (numberOfBreakPoints == 0) {
		free(breakPoint);
		breakPoint = NULL;
		return;
	}

	breakPoint = (unsigned short*)malloc(sizeof(unsigned short) * numberOfBreakPoints);
	if (breakPoint == NULL) {
		return;
	}
	printf("Enter the break points\n");
	for (int i = 0; i < numberOfBreakPoints; i++) {
		if (scanf("%4x", &temp) != 1) {
			printf("Error in setting break point\n");
		}
		while (temp % 2 != 0) {
			printf("Please ensure the break point is on an even address\n");
			if (scanf("%4x", &temp) != 1) {
				printf("Error in setting break point\n");
			}
		}
		breakPoint[i] = temp;
	}
}

void exitEmulator() {
	exited = TRUE;
}
void setRegisterValues() {
	unsigned int numberOfRegisters = 0, registerNumber = 0, value = 0;
	printf("How many registers do you want to set\n");
	scanf("%d", &numberOfRegisters);
	
	while (numberOfRegisters > 8) {
		printf("Only 8 registers can be changed\n");
		scanf("%d", &numberOfRegisters);
	}

	for (int i = 0; i < numberOfRegisters; i++) {
		printf("Enter the register number you want to change\n");
		scanf("%d", &registerNumber);
		printf("Enter the value you want to change to in hex\n");//implement a way to channge both the word and the byte
		scanf("%4x", &value);
		printf("The value before changing is %.4X ", regFile[REG][registerNumber]);
		regFile[REG][registerNumber] = value;
		printf("and the value after changing is %.4X\n", regFile[REG][registerNumber]);
	}
}

void displayPSW() {
	printf("PSW.C = %1X\n", pswptr->bit.C);
	printf("PSW.Z = %1X\n", pswptr->bit.Z);
	printf("PSW.N = %1X\n", pswptr->bit.N);
	printf("PSW.SLP = %1X\n", pswptr->bit.SLP);
	printf("PSW.V = %1X\n", pswptr->bit.V);
	printf("PSW.current_priority = %1X\n", pswptr->bit.currentPriority);
	printf("PSW.FLT = %1X\n", pswptr->bit.flt);
	printf("PSW.previous_priority = %1X\n", pswptr->bit.previousPriority);
}

void deviceInformation() {
	printf("Clock cycles used %llu and the CPU state is %s\n", CPU_clock, state[CPU_State]);
}