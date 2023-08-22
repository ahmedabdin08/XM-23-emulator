#include "exceptions.h"
#include "decode.h"
#include "priority_queue.h"

static char* flt[] = { "Illegal Instruction FAULT\n", "Invalid Address FAULT\n","Priority FAULT\n" ,"DOUBLE FAULT\n" };

void push(unsigned short* value) {
	SP -= 2;
	bus(SP, value, WRITE, WORD);
}

void pull(unsigned short* value) {
	bus(SP, value, READ, WORD);
	SP += 2;
}

void fault_handler(unsigned short index) {
	unsigned short first_fault_vect = 8;
	printf("%s", flt[index - first_fault_vect]);
	if (pswptr->bit.flt) {
		printf("%s", flt[DOUBLE_FAULT - first_fault_vect]);
		exception(DOUBLE_FAULT, FAULT);
	}
	else exception(index, FAULT);
}

void exception(unsigned short index, unsigned short type) {
	int new_priority = pswptr->bit.currentPriority, fault = type == FAULT, priority = IVTptr[index].psw.bit.currentPriority;;
	if (priority > pswptr->bit.currentPriority || (fault && pswptr->bit.flt)) {
		pswptr->bit.SLP = 0;
		push(&PC);
		push(&LR);
		push(&pswptr->word);
		push(&CEXState);
		if (fault && index == ILLEGAL_INSTRUCTION) groupFlag = INVALID;
		PC = IVTptr[index].address;
		LR = 0xFFFF;
		CEXState = NOTCEX;
		new_priority = pswptr->bit.currentPriority;
		pswptr->word = IVTptr[index].psw.word;
		pswptr->bit.previousPriority = new_priority;
		pswptr->bit.flt = fault;
		CPU_State = FETCH;
	}
	else if (type == INTERRUPT) {//else if it is interrupt with lower or same priority it is pending
		enqueue(MLQ[priority], &IVTptr[index], index);
	}
}

void return_from_exception() {
	vectorEntry* ive = check_pending_interrupts();
	if (ive == NULL) {
		pull(&CEXState);
		pull(&pswptr->word);
		pull(&LR);
		pull(&PC);
		return;
	}

	if (ive->psw.bit.currentPriority > pswptr->bit.previousPriority) {//tail chainning
		//we dequeue here the pending interrupt
		for (int i = PRIORITYLEVELS - 1; i >= 0; i--) { // Check from highest to lowest priority
			if (!is_empty(MLQ[i])) {
				dequeue(MLQ[i]);
				break;
			}
		}

		PC = ive->address;
		LR = 0xFFFF;
		CEXState = NOTCEX;
		unsigned int newprev = pswptr->bit.currentPriority;
		pswptr->word = ive->psw.word;
		pswptr->bit.previousPriority = newprev;
		pswptr->bit.SLP = 0;
		CPU_State = FETCH;
	}
	else {
		pull(&CEXState);
		pull(&pswptr->word);
		pull(&LR);
		pull(&PC);
	}
}
