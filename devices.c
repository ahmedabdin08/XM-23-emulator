#include "devices.h"
#include "exceptions.h"
#include <conio.h>
#include <time.h>
#include "memory.h"
#include <stdlib.h>

#define SCREENDELAY 50
#define MILLISECONDS 1000
#define DEV3 3

device* devs[NUMBEROFDEVICES];

clock_t end_time = 0;
unsigned short DRClk = 0;

enum {FALSE, TRUE};
enum {OUTPUT, INPUT};

void initializeMemoryMappedDevices(device* devs[]) {
	for (int i = 0; i < NUMBEROFDEVICES; i++)
	{
		devs[i] = (device*)&memory.byte[DEVICEBASEADDRESS + i * sizeof(device)];
	}
	devs[KB]->CSRbits.IO = TRUE;
	devs[CLK]->CSRbits.IO = TRUE;
	devs[SCR]->CSRbits.IO = FALSE;

	devs[KB]->CSRbits.ENA = TRUE;
	devs[CLK]->CSRbits.ENA = TRUE;
	devs[SCR]->CSRbits.ENA = TRUE;
}

void checkDevices() {
	priority_queue_t* interrupts_q = initialize_pq();//interrupts priority queue to get the interrupts from the devices

	genericDevices(interrupts_q);
	console(interrupts_q);
	keyboard(interrupts_q);
	programmableTimer(interrupts_q);

	while (!is_empty(interrupts_q)) {
		vectorEntry* ive = peek(interrupts_q);
		dequeue(interrupts_q);
		unsigned long long index = ive - IVTptr;//pointer arthimetic to get the vector index
		exception(index, INTERRUPT);
	}
}

static void delay(int milliseconds) {
	clock_t start_time = clock();
	while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / MILLISECONDS);
}

void programmableTimer(priority_queue_t* interrupts_q) {
	if (devs[CLK]->CSRbits.ENA) {
		if (clock() > end_time) {
			if (devs[CLK]->CSRbits.DBA) devs[CLK]->CSRbits.OF = TRUE;
			devs[CLK]->CSRbits.DBA = TRUE;
			unsigned int DRVal = devs[CLK]->DR == 0 ? 1 : devs[CLK]->DR;
			DRClk = 1000/DRVal;
			//printf("\n%d milliseconds ellapsed and PC is %4X\n", MILLISECONDS/DRVal, PC);//sanity test to verify that this is done at the right time
			end_time = clock() + (clock_t)(MILLISECONDS / DRVal);
			if (devs[CLK]->CSRbits.IE) enqueue(interrupts_q, &IVTptr[CLK], IVTptr[CLK].psw.bit.currentPriority);
		}
		else devs[CLK]->CSRbits.DBA = FALSE;
	}
}

void keyboard(priority_queue_t* interrupts_q) {
	if (kbhit() && devs[KB]->CSRbits.ENA) {
		if (devs[KB]->CSRbits.DBA) devs[KB]->CSRbits.OF = TRUE;
		devs[KB]->CSRbits.DBA = TRUE;
		devs[KB]->DR = getch();
		if (devs[KB]->CSRbits.IE) enqueue(interrupts_q, &IVTptr[KB], IVTptr[KB].psw.bit.currentPriority);
	}
}

void console(priority_queue_t* interrupts_q) {
	if (devs[SCR]->CSRbits.ENA && devs[SCR]->CSRbits.DBA == FALSE) {
		delay(SCREENDELAY);
		devs[SCR]->CSRbits.DBA = TRUE;
		putch(devs[SCR]->DR);
		fflush(stdout);
		if (devs[SCR]->CSRbits.IE) enqueue(interrupts_q, &IVTptr[SCR], IVTptr[SCR].psw.bit.currentPriority);
	}
}

void genericDevices(priority_queue_t* interrupts_q) {
	for (int i = DEV3; i < NUMBEROFDEVICES; i++) {
		if (devs[i]->CSRbits.ENA) {
			if (devs[i]->CSRbits.DBA) devs[i]->CSRbits.OF = TRUE;
			devs[i]->CSRbits.DBA = TRUE;
			if (devs[i]->CSRbits.IE) enqueue(interrupts_q, &IVTptr[i], IVTptr[i].psw.bit.currentPriority);
		}
	}
}

void updateDeviceRW(unsigned short effectiveAddress, ReadOrWrite rw, Size wb){
	unsigned int device_index = effectiveAddress / 2;
	devs[KB]->CSRbits.IO = TRUE;
	devs[CLK]->CSRbits.IO = TRUE;
	devs[SCR]->CSRbits.IO = FALSE;
	int clearDBA = devs[device_index]->CSRbits.ENA && ((devs[device_index]->CSRbits.IO == INPUT && rw == READ)
				|| (devs[device_index]->CSRbits.IO == OUTPUT && rw == WRITE));

	if (device_index != CLK) {
		if (wb == BYTE && effectiveAddress % 2 == 0) clearDBA = FALSE;//if we didnt rw from the DR except for the clock we dont clear DBA
	}
	if (clearDBA) {
		devs[device_index]->CSRbits.DBA = FALSE;
	}
}

void viewDevices() {
	printf("number\ttype\tIE\tIO\tDBA\tOF\tENA\tData\tPriority\tmemory\n");
	for (int i = 0; i < NUMBEROFDEVICES; i++) {
		printf("%d\t", i);
		if (i == CLK) printf("CLK\t");
		else if (i == KB) printf("KB\t");
		else if (i == SCR) printf("SCR\t");
		else printf("General\t");
		printf("%d\t%d\t%d\t%d\t%d\t", devs[i]->CSRbits.IE, devs[i]->CSRbits.IO, devs[i]->CSRbits.DBA, devs[i]->CSRbits.OF, devs[i]->CSRbits.ENA);
		isprint(devs[i]->DR) ? printf("%c\t", devs[i]->DR) : printf(".\t");
		printf("%d\t\t", IVTptr[i].psw.bit.currentPriority);
		printWordInMemory(i * 2);
	}
}