/* Header for the CPU and its functions and registerfile
/* Author: Ahmed Khairallah
*/

#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "memory.h"

enum {REG, CONST};
enum { REGCONSTNUMBERS = 2, REGCONSTSIZE = 8 };

unsigned short regFile[REGCONSTNUMBERS][REGCONSTSIZE];

#define	R0 regFile[REG][0] 
#define	R1 regFile[REG][1] 
#define	R2 regFile[REG][2] 
#define	R3 regFile[REG][3] 
#define	BP regFile[REG][4]
#define	LR regFile[REG][5]
#define	SP regFile[REG][6]
#define	PC regFile[REG][7]

#define VECTORTABLEBASE 0xFFC0

extern unsigned short *breakPoint, breakPointCount, oneStep, CPU_State;
enum { FETCH, DECODE, EXECUTE, SLEEP};
extern const char* state[];
unsigned short MBR, IR, MAR, tempReg;
extern unsigned long long CPU_clock;

typedef struct PSWbit{
	unsigned short C : 1;
	unsigned short Z : 1;
	unsigned short N : 1;
	unsigned short SLP : 1;
	unsigned short V : 1;
	unsigned short currentPriority : 3;
	unsigned short flt : 1;
	unsigned short : 4;
	unsigned short previousPriority: 3;
}PSWbit;

typedef union PSW{
	unsigned short word;
	PSWbit bit;
}PSW;

PSW* pswptr;

typedef enum{NOTCEX, SKIPTRUE, SKIPFALSE, EXECUTETRUE, EXECUTEFALSE} CEXState_t;
extern unsigned short CEXState;
extern unsigned int TC, FC;

void runCPU();
void fetch();
void initializeCPU();
void handleCEX();

void update_psw(unsigned short src, unsigned short dst, unsigned short res, Size wb);
void update_pswNZ(unsigned short res, Size wb);
