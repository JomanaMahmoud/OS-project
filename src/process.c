#include <stdio.h>
#include <stdlib.h>
#include "../include/process.h"


// Function to create and initialize a PCB
PCB* createPCB(int pid, int priority, int lower, int upper) {
    PCB* pcb;
    pcb->pid = pid;
    pcb->state = New;
    pcb->priority = priority;
    pcb->programCounter = lower;
    pcb->memoryLowerBound = lower;
    pcb->memoryUpperBound = upper;
    return pcb;
}

// Function to print PCB* info
void printPCB(PCB* pcb) {
    const char* stateNames[] = { "READY", "RUNNING", "BLOCKED" };
    printf("Process ID: %d\n", pcb->pid);
    printf("State: %s\n", stateNames[pcb->state]);
    printf("Priority: %d\n", pcb->priority);
    printf("Program Counter: %d\n", pcb->programCounter);
    printf("Memory Bounds: %d - %d\n", pcb->memoryLowerBound, pcb->memoryUpperBound);
}

int processFinished(PCB* p) {
    return p->programCounter > p->memoryUpperBound;
}