#include "loader.h"
#include"userinterface.h"
#include "priority_queue.h"

int main(int argc, char* argv[]) {
	initialize_MLQ(MLQ);
	initializeMemory();
	initializeCPU();
	userInterface();
	free(breakPoint);
	free(pswptr);
}