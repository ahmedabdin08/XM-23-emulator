/* Header for decoding the instructions that were fetched and will be executed
/* Author: Ahmed Khairallah
*/
#pragma once

#include "memory.h"
//ALU instruction MACROS
#define RC(x) (x >> 7) & 0x01
#define SRC(x) (x>> 3) & 0x07
#define DST(x) x &0x07
#define WB(x) (x>>6) & 0x01

#define CONDITION(x) x>>6 &0x0F
#define TRUECEXCOUNT(x) x>>3 &0x07
#define FALSECEXCOUNT(x) x &0x07

#define MOVBYTE(x) (x>>3) & 0x00FF

//implement sign extension for the branch offsets

void decode();
short extractOffset();

void signExtendFromBit(short* x, int bit);

typedef enum{BRANCHLINK, BRANCHCONDITION, ALUArthimetic, ALULogic, MOVSWAP, SHIFTROTATE, PSWCONTROL, CONDEX, DIRECTADDR, REGINIT, RELADDR, INVALID} group_t;
enum {BEQ, BNE, BC, BNC, BN, BGE, BLT, BRA, BL = 0};//BRANCH
enum {ADD, ADDC, SUB, SUBC, DADD, CMP, XOR, AND, OR, BIT, BIC, BIS}; //ALU
enum {SETPRI, SVC, SETCC, CLRCC, CEX};//PSW and CEX
enum {MOV, SWAP};//MOVSWAP
enum {LD, ST};//DIRECTADDR
enum {SRA, RRC, COMP, SWPB, SXT};//shift and rotate
enum {MOVL, MOVLZ, MOVLS, MOVH};//Moving bytes
enum{LDR, STR};//relative address
unsigned short opcode;
group_t groupFlag;

//note to self: enums such as BEQ BL ADD SETPRI etc all have a value of 0 the groupflag is what's used to distinguish them


void extractBranchCondition();
void extractRegisterInit();
void extractRest();
void extractALU();
void extractMisc();
void extractShiftRotate();
void extractPSWControl();