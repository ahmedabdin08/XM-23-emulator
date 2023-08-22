#pragma once
#include "memory.h"
#include "priority_queue.h"

#define NUMBEROFDEVICES 8
#define DEVICEBASEADDRESS 0

enum {CLK, KB, SCR};//CLK is dev0, KB is dev1, and SCR is dev2

struct CSR{
	unsigned char IE : 1;
	unsigned char IO : 1;
	unsigned char DBA : 1;
	unsigned char OF : 1;
	unsigned char ENA : 1;
	unsigned char RES : 3;
}CSR;

typedef struct device{
	struct CSR CSRbits;//control and status register
	unsigned char DR;//Data register
}device;

extern device* devs[NUMBEROFDEVICES];

void initializeMemoryMappedDevices(device * devs[]);
void updateDeviceRW(unsigned short effectiveAddress, ReadOrWrite rw, Size wb);
void checkDevices();
void console(priority_queue_t * interrupts_q);
void genericDevices(priority_queue_t* interrupts_q);
void keyboard(priority_queue_t* interrupts_q);
void programmableTimer(priority_queue_t* interrupts_q);
void viewDevices();