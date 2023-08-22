#include "cpu.h"
#include "decode.h"
#include "exceptions.h"

void signExtendFromBit(short* x, int bit) {
	unsigned short size = 15;
	*x = *x << (size - bit);
	*x = *x >> (size - bit);
}


signed short extractOffset() {
	signed short offset = 0, msb = 0;
	if (groupFlag == BRANCHLINK) {
		offset = IR & 0x1FFF;
		offset = offset << 1;
		msb = IR >> 11 & 0x01;
		if (msb) offset |= 0xC000;
	}
	else{
		offset = IR & 0x03FF;
		offset = offset << 1;
		msb = IR >> 8 & 0x01;
		if (msb) offset |= 0xF800;
	}
	return offset;
}

void decode() {
	unsigned short mostSignifcantThreeBits = IR >> 13 & 0x07;
	switch (mostSignifcantThreeBits)
	{
	case 0:
		groupFlag = BRANCHLINK;
		opcode = BL;
		break;
	case 1:
		groupFlag = BRANCHCONDITION;
		extractBranchCondition();
		break;
	case 2:
		extractRest();
		break;
	case 3:
		groupFlag = REGINIT;
		extractRegisterInit();
		break;
	case 4:
	case 5:
		opcode = LDR;
		groupFlag = RELADDR;
		break;
	case 6:
	case 7:
		opcode = STR;
		groupFlag = RELADDR;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	CPU_clock++;
	CPU_State = EXECUTE;
}

void extractBranchCondition() {
	unsigned short threeMoreBits = IR >> 10 & 0x07;
	switch (threeMoreBits)
	{
	case 0:
		opcode = BEQ;
		break;
	case 1:
		opcode = BNE;
		break;
	case 2:
		opcode = BC;
		break;
	case 3:
		opcode = BNC;
		break;
	case 4:
		opcode = BN;
		break;
	case 5:
		opcode = BGE;
		break;
	case 6:
		opcode = BLT;
		break;
	case 7:
		opcode = BRA;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void extractRegisterInit() {
	unsigned short nextTwoBits = IR >> 11 & 0x03;
	switch (nextTwoBits)
	{
	case 0:
		opcode = MOVL;
		break;
	case 1:
		opcode = MOVLZ;
		break;
	case 2:
		opcode = MOVLS;
		break;
	case 3:
		opcode = MOVH;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}
void extractRest() {
	unsigned short threeMoreBits = IR >> 10 & 0x07;
	switch (threeMoreBits)
	{
	case 0:
	case 1:
	case 2:
		extractALU();
		break;
	case 3:
		extractMisc();
		break;
	case 4:
		groupFlag = CONDEX;
		opcode = CEX;
		break;
	case 6:
		opcode = LD;
		groupFlag = DIRECTADDR;
		break;
	case 7:
		opcode = ST;
		groupFlag = DIRECTADDR;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void extractALU() {
	unsigned short ALUFourBits = IR >> 8 & 0x0F;
	if (ALUFourBits > 5) groupFlag = ALULogic;
	else groupFlag = ALUArthimetic;
	switch (ALUFourBits)
	{
	case 0:
		opcode = ADD;
		break;
	case 1:
		opcode = ADDC;
		break;
	case 2:
		opcode = SUB;
		break;
	case 3:
		opcode = SUBC;
		break;
	case 4:
		opcode = DADD;
		break;
	case 5:
		opcode = CMP;
		break;
	case 6:
		opcode = XOR;
		break;
	case 7:
		opcode = AND;
		break;
	case 8:
		opcode = OR;
		break;
	case 9:
		opcode = BIT;
		break;
	case 10:
		opcode = BIC;
		break;
	case 11:
		opcode = BIS;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void extractMisc() {
	unsigned short twoBits = IR >> 7 & 0x03;
	switch (twoBits)
	{
	case 0:
		opcode = MOV;
		groupFlag = MOVSWAP;
		break;
	case 1:
		opcode = SWAP;
		groupFlag = MOVSWAP;
		break;
	case 2:
		extractShiftRotate();
		groupFlag = SHIFTROTATE;
		break;
	case 3:
		extractPSWControl();
		groupFlag = PSWCONTROL;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void extractShiftRotate() {
	unsigned short threeBits = IR >> 3 & 0x07;
	switch (threeBits)
	{
	case 0:
		opcode = SRA;
		break;
	case 1:
		opcode = RRC;
		break;
	case 2:
		opcode = COMP;
		break;
	case 3:
		opcode = SWPB;
		break;
	case 4:
		opcode = SXT;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void extractPSWControl() {
	unsigned short twoMoreBits = IR >> 4 & 0x07;
	switch (twoMoreBits)
	{
	case 0:
		opcode = SETPRI;
		break;
	case 1:
		opcode = SVC;
		break;
	case 2:
	case 3:
		opcode = SETCC;
		break;
	case 4:
	case 5:
		opcode = CLRCC;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}