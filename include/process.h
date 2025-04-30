#ifndef PROCESS_H
#define PROCESS_H

typedef enum {New, READY, RUNNING, BLOCKED } ProcessState;

typedef struct PCB {
    int pid;                    // Process ID
    ProcessState state;         // Current process state
    int priority;               // Priority level
    int programCounter;         // Next instruction to execute
    int memoryLowerBound;       // Start of memory allocation
    int memoryUpperBound;       // End of memory allocation
    int arrival_time;           // Time when the process arrives (new field)
} PCB;


PCB* createPCB(int pid, int priority, int lower, int upper); 
void printPCB(PCB* pcb);
int processFinished(PCB* p);

#endif