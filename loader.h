#pragma once
/* Header to for loader functions to load the instructions from data onto Memory
/* Author: Ahmed Khairallah
*/
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "cpu.h"

enum { FIRSTCHARACTER, SECONDCHARACTER, THIRDCHARACTER };

#pragma warning(disable: 4996)
#define MAXSTRLEN 256
#define NUL '\0'


void processRecord1(unsigned char length, int startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber);
void processRecord0(int length, unsigned char startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber);
void processRecord9(unsigned char length, unsigned char startAddressHigh, unsigned char startAddressLow, char buffer[], int lineNumber);
void loader(FILE* fptr);