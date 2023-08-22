/* Header for executing the instructions that have been decoded
/* Author: Ahmed Khairallah
*/

#pragma once
#include "decode.h"
#include "memory.h"

void execute();

void executeALUArthimetic();
void executeALULogic();
void branchLink();
void branchCondition();
void moveSwap();
void shiftRotate();
void pswControl();
void coniditionalExecution();
void directAddressing();
void registerInitialization();
void relativeAddressing();

enum{EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, TR, FL};//condition execution codes

void printInstructionValue();

word_byte DADD_func(word_byte destination, word_byte source);
word_byte ADD_Func(word_byte dst, word_byte src, unsigned short carry, Size wb);