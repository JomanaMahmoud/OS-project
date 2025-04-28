#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "memory.h"  // Include memory handling header for memory-related functions
#include "process.h" // Include process header for PCB definition


// Function prototypes

// Function to store the PCB fields into memory
void storePCBInMemory(PCB pcb);

// Function to initialize a process from a file and store its PCB and instructions in memory
void initializeProcessFromFile(const char* filename, int priority);

// Helper functions (if needed for internal operations)
PCB createPCB(int pid, int priority, int memoryLowerBound, int memoryUpperBound);
void printPCB(PCB pcb);
void executeInstruction(char* instruction);
// Optionally, external memory management functions (you may have to define them in memory.h)
// void write_to_memory(int address, const char* name, const char* value, int isInstruction);

extern int currentPID;            // Global variable to track the current process ID
extern int nextFreeMemoryIndex;   // Global variable to track the next free memory index

#endif // INTERPRETER_H
