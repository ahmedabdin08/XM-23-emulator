#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include "userinterface.h"
#include "memory.h"
#include "cache.h"
#include "devices.h"
#include "exceptions.h"

int running;
enum {FALSE, TRUE};
unsigned short CEXState = NOTCEX, breakPointCount = 0;
unsigned int TC = 0, FC = 0;

unsigned short regFile[REGCONSTNUMBERS][REGCONSTSIZE] = { {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 2, 4, 8, 16, 32, -1} };
unsigned short CPU_State = FETCH;
const char* state[] = { "Fetch", "Decode", "Execute" };
unsigned long long CPU_clock = 0;

void signalHandler(int dummy) {
	running = FALSE;
	signal(SIGINT, (_crt_signal_t)signalHandler);
}

void runCPU() {
	int didBreak = FALSE;
	running = TRUE;
	signal(SIGINT, (_crt_signal_t)signalHandler);
	do {
		if(pswptr->bit.SLP) CPU_State = SLEEP;//infinitely loop till an interrupt occurs
		if(CPU_State == FETCH) fetch();
		if(CPU_State == DECODE) decode();
		if(CPU_State == EXECUTE) execute();
		checkDevices();
		didBreak = processBreakPoints();
	} while (running && !didBreak && !oneStep);
	printf("\nYou have exited the CPU its PC is %.4X and clock cycles %llu used since the start\n", PC, CPU_clock);
}

void handleCEX() {
	if (CEXState == EXECUTETRUE || CEXState == SKIPTRUE) {
		if (TC > 0)
		{
			TC--;
			if (CEXState == EXECUTETRUE) CPU_State = DECODE;
			else CPU_State = FETCH;
		}
		else {
			if (CEXState == EXECUTETRUE) CEXState = SKIPFALSE;
			else if (CEXState == SKIPTRUE) CEXState = EXECUTEFALSE;
		}
	}
	if (CEXState == EXECUTEFALSE || CEXState == SKIPFALSE) {
		if (FC > 0)
		{
			FC--;
			if (CEXState == EXECUTEFALSE) CPU_State = DECODE;
			else CPU_State = FETCH;
		}
		else {
			CEXState = NOTCEX;
		}
	}
}


void fetch() {
	CPU_clock++;
	if (PC == 0xFFFF) {//return from ISR
		return_from_exception();
	}
	else if (PC % 2 == 1) {//if it is odd and not 0xFFFF
		fault_handler(INVALID_ADDRESS);
		CPU_clock += 2;
		return;
	}

	if (PC >= VECTORTABLEBASE) PC = 0;
	MAR = PC;
	//bus(MAR, &MBR, READ, WORD);
	Cache(MAR, &MBR, READ, WORD);
	IR = MBR;
	PC += 2;
	CPU_State = DECODE;
	handleCEX();
}

void initializeCPU() {
	pswptr = (PSW*)malloc(sizeof(PSWbit));
	pswptr->word = 0;
	pswptr->bit.currentPriority = 6;
	IVTptr = &memory.byte[VECTORTABLEBASE];
	SP = VECTORTABLEBASE;
	//clock = 0;
}

int processBreakPoints() {
	if (breakPoint == NULL) {
		return FALSE;
	}

	for (int i = 0; i < breakPointCount; i++) {
		if (breakPoint[i] == PC) {
			return TRUE;
		}
	}

	return FALSE;
}