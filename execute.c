#include "cpu.h"
#include "execute.h"
#include "cache.h"
#include "exceptions.h"
#include "devices.h"

enum {FALSE, TRUE};
void execute() {
	switch (groupFlag)
	{
	case BRANCHLINK:
		branchLink();
		break;
	case BRANCHCONDITION:
		branchCondition();
		break;
	case ALUArthimetic:
		executeALUArthimetic();
		break;
	case ALULogic:
		executeALULogic();
		break;
	case MOVSWAP:
		moveSwap();
		break;
	case SHIFTROTATE:
		shiftRotate();
		break;
	case REGINIT:
		registerInitialization();
		break;
	case PSWCONTROL:
		pswControl();
		break;
	case CONDEX:
		coniditionalExecution();
		break;
	case DIRECTADDR:
		directAddressing();
		break;
	case RELADDR:
		relativeAddressing();
		break;
	case INVALID:
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	CPU_clock++;
	CPU_State = FETCH;
}

void moveSwap() {
	unsigned short wb = WB(IR);
	word_byte source, destination, result = { 0 };
	source.word = regFile[REG][SRC(IR)];
	destination.word = regFile[REG][DST(IR)];

	if (wb == BYTE) {//we know only mov has a byte
		result.byte[0] = source.byte[0];
		destination.byte[0] = result.byte[0];
	}
	else {
		result.word = source.word;
		if (opcode == SWAP) {
			regFile[REG][SRC(IR)] = destination.word;
		}
		destination.word = result.word;
	}
	regFile[REG][DST(IR)] = destination.word;
}

void executeALUArthimetic() {
	unsigned short wb = WB(IR);
	word_byte source, destination, source_compliment;
	source.word = regFile[RC(IR)][SRC(IR)];
	destination.word = regFile[REG][DST(IR)];
	source_compliment.word = ~(source.word);

	switch (opcode)
	{
	case ADD:
		destination = ADD_Func(destination, source, 0, WB(IR));
		break;
	case ADDC:
		destination = ADD_Func(destination, source, pswptr->bit.C, WB(IR));
		break;
	case SUB:
	case CMP:
		destination = ADD_Func(destination, source_compliment, 1, WB(IR));
		break;
	case SUBC:
		destination = ADD_Func(destination, source_compliment, pswptr->bit.C, WB(IR));
		break;
	case DADD:
		destination = DADD_func(destination, source);
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	if (opcode != CMP){
		regFile[REG][DST(IR)] = destination.word;
	}
}

void executeALULogic() {
	unsigned short wb = WB(IR);
	word_byte source, destination;
	source.word = regFile[RC(IR)][SRC(IR)];
	destination.word = regFile[REG][DST(IR)];

	switch (opcode) {
	case XOR:
		if (wb == WORD) destination.word = source.word ^ destination.word;
		else destination.byte[0] = source.byte[0] ^ destination.byte[0];
		break;
	case AND:
		if (wb == WORD) destination.word = source.word & destination.word;
		else destination.byte[0] = source.byte[0] & destination.byte[0];
		break;
	case OR:
		if (wb == WORD) destination.word = source.word | destination.word;
		else destination.byte[0] = source.byte[0] | destination.byte[0];
		break;
	case BIT:
		if (wb == WORD) pswptr->bit.Z = (destination.word & (1 << source.word)) == 0;
		else pswptr->bit.Z = (destination.byte[0] & (1 << source.byte[0])) == 0;
		break;
	case BIC:
		if (wb == WORD) destination.word = destination.word & ~(1 << source.word);
		else destination.byte[0] = destination.byte[0] & ~(1 << source.byte[0]);
		break;
	case BIS:
		if (wb == WORD) destination.word = destination.word | (1 << source.word);
		else destination.byte[0] = destination.byte[0] | (1 << source.byte[0]);
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	if (opcode != BIT) {
		regFile[REG][DST(IR)] = destination.word;
		update_pswNZ(destination.word, WB(IR));
	}
}

void branchLink() {
	LR = PC;
	signed short newPC = PC, offset = extractOffset();
	newPC += offset;
	PC = newPC;//idk if this will cause unexpected behaviour
}

void branchCondition() {
	unsigned int shouldBranch = FALSE;
	switch (opcode) {
	case BEQ:
		shouldBranch = (pswptr->bit.Z == 1);
		break;
	case BNE:
		shouldBranch = (pswptr->bit.Z == 0);
		break;
	case BC:
		shouldBranch = (pswptr->bit.C == 1);
		break;
	case BNC:
		shouldBranch = (pswptr->bit.C == 0);
		break;
	case BN:
		shouldBranch = (pswptr->bit.N == 1);
		break;
	case BGE:
		shouldBranch = (pswptr->bit.N == pswptr->bit.V);
		break;
	case BLT:
		shouldBranch = (pswptr->bit.N != pswptr->bit.V);
		break;
	case BRA:
		shouldBranch = TRUE;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	if (shouldBranch) {
		signed short newPC = PC, offset = extractOffset();
		newPC += offset;
		PC = newPC;
	}
}

void shiftRotate() {
	unsigned short wb = WB(IR);
	word_byte destination, temporary;
	destination.word = regFile[REG][DST(IR)];
	temporary = destination;
	unsigned short newCarry;
	switch (opcode)
	{
	case SRA:
		if (wb == BYTE) { 
			destination.byte[0] = destination.byte[0] >> 1;
			destination.byte[0] |= (destination.byte[0] >> 7) << 8;//to perform sign extension
		}
		else {
			destination.word = destination.word >> 1;
			signExtendFromBit(&destination.word, 14);//sign extension
		}
		break;
	case RRC:
		newCarry = destination.word & 0x01;
		if (wb == BYTE) { 
			destination.byte[0] >>= 1; 
			destination.byte[0] |= pswptr->bit.C << 7;
		}
		else {
			destination.word >>= 1;
			destination.word |= pswptr->bit.C << 15;
		}
		pswptr->bit.C = newCarry;
		break;
	case COMP:
		if (wb == BYTE) destination.byte[0] = ~destination.byte[0];
		else destination.word = ~destination.word;
		break;
	case SWPB:
		destination.byte[0] = temporary.byte[1];
		destination.byte[1] = temporary.byte[0];
		break;
	case SXT:
		signExtendFromBit(&destination.word, 7);
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	regFile[REG][DST(IR)] = destination.word;
}

void registerInitialization() {
	unsigned char byteMovement = MOVBYTE(IR);
	word_byte destination;
	destination.word = regFile[REG][DST(IR)];
	
	switch (opcode)
	{
	case MOVL:
		destination.byte[0] = byteMovement;
		break;
	case MOVLZ:
		destination.byte[0] = byteMovement;
		destination.byte[1] = 0x00;
		break;
	case MOVLS:
		destination.byte[0] = byteMovement;
		destination.byte[1] = 0xFF;
		break;
	case MOVH:
		destination.byte[1] = byteMovement;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}

	regFile[REG][DST(IR)] = destination.word;
}

void relativeAddressing() {
	unsigned short wb = WB(IR);
	signed short relativeOffset = IR >> 7 & 0x7F, effectiveAddress = 0;//0x7f is to extract the 7 bits
	word_byte source, destination;
	signExtendFromBit(&relativeOffset, 6);
	source.word = regFile[REG][SRC(IR)];
	destination.word = regFile[REG][DST(IR)];
	switch (opcode)
	{
	case LDR:
		effectiveAddress = source.word + relativeOffset;
		if ((unsigned)effectiveAddress <= DEVICEMEMORYMAX) {
			bus(effectiveAddress, &destination.word, READ, WB(IR));
			updateDeviceRW(effectiveAddress, READ, wb);
		}
		else Cache(effectiveAddress, &destination.word, READ, wb);

		break;
	case STR:
		effectiveAddress = destination.word + relativeOffset;
		if ((unsigned)effectiveAddress <= DEVICEMEMORYMAX) {
			bus(effectiveAddress, &source.word, WRITE, WB(IR));
			updateDeviceRW(effectiveAddress, WRITE, wb);
		}
		else Cache(effectiveAddress, &source.word, WRITE, wb);

		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}

	regFile[REG][DST(IR)] = destination.word;
}

enum {POST, PRE};
void directAddressing() {
	signed short relativeOffset = 0, effectiveAddress = 0;
	word_byte source, destination;

	unsigned short PRPO = B9(IR), DEC = B8(IR), INC = B7(IR), wb = WB(IR);
	source.word = regFile[REG][SRC(IR)];
	destination.word = regFile[REG][DST(IR)];

	if ((wb == WORD) && DEC) relativeOffset = -2;
	else if ((wb == WORD) && INC) relativeOffset = 2;
	else if ((wb == BYTE) && DEC) relativeOffset = -1;
	else if ((wb == BYTE) && INC) relativeOffset = 1;

	switch (opcode)
	{
	case LD:
		if (PRPO == PRE) source.word += relativeOffset;
		effectiveAddress = source.word;

		if((unsigned)effectiveAddress <= DEVICEMEMORYMAX) {
			bus(effectiveAddress, &destination.word, READ, WB(IR));
			updateDeviceRW(effectiveAddress, READ, wb);
		}
		else Cache(effectiveAddress, &destination.word, READ, wb);

		if (PRPO == POST) source.word += relativeOffset;
		break;
	case ST:
		if (PRPO == PRE) destination.word += relativeOffset;
		effectiveAddress = destination.word;

		if((unsigned)effectiveAddress <= DEVICEMEMORYMAX){
			bus(effectiveAddress, &source.word, WRITE, WB(IR));
			updateDeviceRW(effectiveAddress, WRITE, wb);
		}
		else Cache(effectiveAddress, &source.word, WRITE, wb);

		if (PRPO == POST) destination.word += relativeOffset;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
	regFile[REG][SRC(IR)] = source.word;
	regFile[REG][DST(IR)] = destination.word;
}

void pswControl() {
	unsigned short priority = IR & 0x07, serviceAddress = IR & 0x0F;
	unsigned short C = IR & 0x01, Z = IR & 0x02, N = IR & 0x04, SLP = IR & 0x08, V = IR & 0x10;
	switch (opcode)
	{
	case SETPRI:
		if (priority > pswptr->bit.currentPriority) fault_handler(PRIORITY_FAULT);
		else if (priority < pswptr->bit.previousPriority) {
			pswptr->bit.currentPriority = pswptr->bit.previousPriority;
			pswptr->bit.previousPriority = priority;
		}
		else {
			pswptr->bit.previousPriority = pswptr->bit.currentPriority;
			pswptr->bit.currentPriority = priority;
		}
		break;
	case SVC:
		if (IVTptr[serviceAddress].psw.bit.currentPriority > pswptr->bit.currentPriority) exception(serviceAddress, TRAP);
		else fault_handler(PRIORITY_FAULT);
		break;
	case CLRCC:
		if (C) pswptr->bit.C = 0;
		if (Z) pswptr->bit.Z = 0;
		if (SLP) pswptr->bit.SLP = 0;
		if (N) pswptr->bit.N = 0;
		if (V) pswptr->bit.V = 0;
		break;
	case SETCC:
		if (C) pswptr->bit.C = 1;
		if (Z) pswptr->bit.Z = 1;
		if (SLP && pswptr->bit.currentPriority != 7) pswptr->bit.SLP = 1;
		if (N) pswptr->bit.N = 1;
		if (V) pswptr->bit.V = 1;
		break;
	default:
		fault_handler(ILLEGAL_INSTRUCTION);
		break;
	}
}

void coniditionalExecution() {
	unsigned int condition = CONDITION(IR), isTrue = FALSE;
	TC = TRUECEXCOUNT(IR), FC = FALSECEXCOUNT(IR);
	switch (condition)
	{
	case EQ:
		isTrue = pswptr->bit.Z == 1;
		break;
	case NE:
		isTrue = pswptr->bit.Z == 0;
		break;
	case CS:
		isTrue = pswptr->bit.C == 1;
		break;
	case CC:
		isTrue = pswptr->bit.C == 0;
		break;
	case MI:
		isTrue = pswptr->bit.N == 1;
		break;
	case PL:
		isTrue = pswptr->bit.N == 0;
		break;
	case VS:
		isTrue = pswptr->bit.V == 1;
		break;
	case VC:
		isTrue = pswptr->bit.V == 0;
		break;
	case HI:
		isTrue = pswptr->bit.C == 1 && pswptr->bit.Z == 0;
		break;
	case LS:
		isTrue = pswptr->bit.C == 0 || pswptr->bit.Z == 1;
		break;
	case GE:
		isTrue = pswptr->bit.N == pswptr->bit.V;
		break;
	case LT:
		isTrue = pswptr->bit.N != pswptr->bit.V;
		break;
	case GT:
		isTrue = pswptr->bit.Z == 0 && pswptr->bit.N == pswptr->bit.V;
		break;
	case LE:
		isTrue = pswptr->bit.Z == 1 && pswptr->bit.N != pswptr->bit.V;
		break;
	case TR:
		isTrue = TRUE;
		break;
	case FL:
		isTrue = FALSE;
		break;
	}
	if (isTrue) CEXState = EXECUTETRUE;
	else CEXState = SKIPTRUE;
}

word_byte ADD_Func(word_byte dst, word_byte src, unsigned short carry, Size wb) {
	//Used for ADD, ADDC, SUB, SUBC, and CMP
	word_byte result = dst;
	if (wb == WORD) {
		src.word += carry;
		result.word = dst.word + src.word;
	}
	else {
		src.byte[0] += carry;
		result.byte[0] = dst.byte[0] + src.byte[0];
	}
	update_psw(src.word, dst.word, result.word, wb);
	return result;
}

word_byte DADD_func(word_byte destination, word_byte source) {
	unsigned short wb = WB(IR), nibbleSize = 4;
	unsigned int numberofBCDAdds = 0, HC = pswptr->bit.C;
	word_byte result = destination;
	if (wb == BYTE) numberofBCDAdds = 2;
	else numberofBCDAdds = 4;
	for (int i = 0; i < numberofBCDAdds; i++) {
		unsigned char res = (destination.word & 0x0F) + (source.word & 0x0F) + HC;
		HC = 0;
		if (res > 9) {
			res -= 10;
			HC = 1;
		}
		result.word |= res << (i * nibbleSize);
		destination.word >>= nibbleSize;
		source.word >>= nibbleSize;
	}
	update_psw(source.word, destination.word, result.word, WB(IR));
	pswptr->bit.C = HC;
	return result;
}