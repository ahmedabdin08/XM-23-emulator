#pragma once
/* Header for the Interrupt Vector Table and handling exceptions
/* Author: Ahmed Khairallah
*/

#include "cpu.h"

typedef struct vectorEntry {
	PSW psw;//NOTE PSW COMES BEFORE THE ADDRESS
	unsigned short address;
}vectorEntry;

vectorEntry* IVTptr;

enum { INTERRUPT, FAULT, TRAP };
typedef enum { ILLEGAL_INSTRUCTION = 8, INVALID_ADDRESS = 9, PRIORITY_FAULT = 10, DOUBLE_FAULT = 11 } Fault_t;

void exception(unsigned short index, unsigned short type);
void push(unsigned short* value);
void pull(unsigned short* value);
void fault_handler(unsigned short index);
void return_from_exception();