#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "gui.h"
#include "readyQueue.h"
#include "interpreter.h"
#include "Scheduler.h"
#include "memory.h"  // Needed for reading from memory

extern int clockCycle;
char schedulerName[20];

// Helper function to add arrived processes from memory to ReadyQueue
void addArrivedProcessesToReadyQueue(ReadyQueue* readyQueue, int clockCycle) {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        MemoryCell cell = read_from_memory(i);

        if (strcmp(cell.name, "pid") == 0) {
            int pid = atoi(cell.data);

            int arrival_time = -1;
            char state[20] = "";
            int programCounter = -1;
            int memoryLowerBound = -1;
            int memoryUpperBound = -1;
            int priority = -1;

            for (int j = i + 1; j < MEMORY_SIZE && j <= i + 10; j++) {
                MemoryCell nextCell = read_from_memory(j);

                if (strcmp(nextCell.name, "arrival_time") == 0)
                    arrival_time = atoi(nextCell.data);
                else if (strcmp(nextCell.name, "state") == 0)
                    strcpy(state, nextCell.data);
                else if (strcmp(nextCell.name, "programCounter") == 0)
                    programCounter = atoi(nextCell.data);
                else if (strcmp(nextCell.name, "memoryLowerBound") == 0)
                    memoryLowerBound = atoi(nextCell.data);
                else if (strcmp(nextCell.name, "memoryUpperBound") == 0)
                    memoryUpperBound = atoi(nextCell.data);
                else if (strcmp(nextCell.name, "priority") == 0)
                    priority = atoi(nextCell.data);
            }

            if (arrival_time <= clockCycle && strcmp(state, "NEW") == 0) {
                PCB newPCB;
                newPCB.pid = pid;
                newPCB.arrival_time = arrival_time;
                newPCB.state = READY;  

                newPCB.programCounter = programCounter;
                newPCB.memoryLowerBound = memoryLowerBound;
                newPCB.memoryUpperBound = memoryUpperBound;
                newPCB.priority = priority;

                enqueueReadySortedByArrival(readyQueue, newPCB);

                // Update state in memory
                for (int j = i + 1; j < MEMORY_SIZE && j <= i + 10; j++) {
                    MemoryCell nextCell = read_from_memory(j);

                    if (strcmp(nextCell.name, "state") == 0) {
                        write_to_memory(j, "state", "READY", 0);
                        break;
                    }
                }
            }
        }
    }
}

// FCFS Scheduling Function
void startFCFS(ReadyQueue* readyQueue) {
    strcpy(schedulerName, "FCFS");

    while (!isReadyQueueEmpty(readyQueue) || clockCycle < MEMORY_SIZE) {
        addArrivedProcessesToReadyQueue(readyQueue, clockCycle);

        PCB* currentProcess = dequeueReady(readyQueue);

        if (currentProcess != NULL) {
            currentProcess->state = READY;

            executeInstruction(currentProcess);
            clockCycle++;
        } else {
            // No process ready, just increment clock
            clockCycle++;
        }

        updateGUI();
    }
}

// Round Robin Scheduling Function
void startRoundRobin(ReadyQueue* readyQueue, int quantum) {
    strcpy(schedulerName, "Round Robin");

    while (!isReadyQueueEmpty(readyQueue) || clockCycle < MEMORY_SIZE) {
        addArrivedProcessesToReadyQueue(readyQueue, clockCycle);

        PCB* currentProcess = dequeueReady(readyQueue);
        int quantumCounter = 0;

        if (currentProcess != NULL) {
            currentProcess->state = READY;

            

            while (quantumCounter < quantum && !processFinished(currentProcess)) {
                executeInstruction(currentProcess);
                clockCycle++;
                quantumCounter++;
                updateGUI();
            }

            if (!processFinished(currentProcess)) {
                enqueueReadySortedByArrival(readyQueue, *currentProcess);
            }
        } else {
            clockCycle++;
        }
    }
}

// MLFQ Scheduling Function
void mlfq_schedule(ReadyQueue* queues[4]) {
    strcpy(schedulerName, "MLFQ");

    printf("\n--- MLFQ Scheduling ---\n");

    int quantums[4] = {1, 2, 4, 8};

    while (1) {
        int foundProcess = 0;

        // Add any newly arrived processes to the highest queue (Level 0)
        addArrivedProcessesToReadyQueue(queues[0], clockCycle);

        for (int level = 0; level < 4; level++) {
            if (!isReadyQueueEmpty(queues[level])) {
                foundProcess = 1;

                PCB* p = dequeueReady(queues[level]);
                currentProcess->state = READY;


            
                int quantum = quantums[level];

                if (level == 3) {
                    // Level 4 -> Round Robin
                    startRoundRobin(queues[3], 1);
                    break;
                }

                for (int i = 0; i < quantum && p->programCounter <= p->memoryUpperBound; i++) {
                    MemoryCell cell = read_from_memory(p->programCounter);

                    if (cell.data[0] != '\0') {
                        executeInstruction(cell.data);
                    }

                    p->programCounter++;
                    clockCycle++;
                    updateGUI();
                }

                if (p->programCounter > p->memoryUpperBound) {
                    // Finished
                    currentProcess->state = READY;

                } else {
                    // Still has instructions
                    currentProcess->state = READY;


                    if (level < 3) {
                        enqueueReadySortedByArrival(queues[level + 1], *p);
                    } else {
                        enqueueReadySortedByArrival(queues[3], *p);
                    }
                }

                break; // Execute one process per clock tick
            }
        }

        if (!foundProcess) {
            break;
        }
    }
}
