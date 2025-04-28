#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include <process.h>
#include <time.h>  // Include for time handling
#include "readyQueue.h"

#define MEMORY_SIZE 60

int currentPID = 1;
int nextFreeMemoryIndex = 0; // Keep track of free memory

// Helper function to store PCB fields in memory
void storePCBInMemory(PCB pcb) {
    char buffer[40];

    // Store PID
    sprintf(buffer, "%d", pcb.pid);
    write_to_memory(pcb.memoryLowerBound, "PID", buffer, 0);

    // Store Process State
    write_to_memory(pcb.memoryLowerBound + 1, "State", "READY", 0);

    // Store Priority
    sprintf(buffer, "%d", pcb.priority);
    write_to_memory(pcb.memoryLowerBound + 2, "Priority", buffer, 0);

    // Store Program Counter (PC)
    sprintf(buffer, "%d", pcb.programCounter);
    write_to_memory(pcb.memoryLowerBound + 3, "PC", buffer, 0);

    // Store Memory Bounds
    sprintf(buffer, "%d-%d", pcb.memoryLowerBound, pcb.memoryUpperBound);
    write_to_memory(pcb.memoryLowerBound + 4, "MemoryBounds", buffer, 0);

    // Store Arrival Time
    sprintf(buffer, "%d", pcb.arrival_time);
    write_to_memory(pcb.memoryLowerBound + 5, "ArrivalTime", buffer, 0);  // Storing arrival time in memory
}

// Function to initialize a process and store its PCB + instructions
void initializeProcessFromFile(const char* filename, int priority, ReadyQueue* readyQueue) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open %s\n", filename);
        return;
    }

    // Reserve memory: 3 vars + 5 PCB + up to 12 instructions (you can modify this)
    int requiredSpace = 3 + 5 + 12;
    if (nextFreeMemoryIndex + requiredSpace > MEMORY_SIZE) {
        printf("Not enough memory to load process from %s\n", filename);
        fclose(file);
        return;
    }

    int varStart = nextFreeMemoryIndex;
    int pcbStart = varStart + 3;
    int instrStart = pcbStart + 5;
    int instrIndex = instrStart;

    // Set the arrival time to the current time when the process is created
    time_t current_time = time(NULL);  // Get the current time (seconds since epoch)
    PCB pcb = createPCB(currentPID++, priority, varStart, instrStart + 11); // memory range
    pcb.arrival_time = (int)current_time;  // Assign current time as the arrival time

    // Store the PCB in memory
    storePCBInMemory(pcb);

    // Now enqueue it to the ready queue (sorted by arrival time)
    enqueueReadySortedByArrival(readyQueue, pcb);

    // Read instructions from the file and load into memory
    char line[100];
    while (fgets(line, sizeof(line), file) && instrIndex <= pcb.memoryUpperBound) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline
        write_to_memory(instrIndex++, "Instruction", line, 1);
    }

    nextFreeMemoryIndex = instrIndex;
    printf("Loaded process from %s\n", filename);
    fclose(file);
}

void executeInstruction(char* instruction) {
    char command[20], arg1[20], arg2[20];
    int numArgs = sscanf(instruction, "%s %s %s", command, arg1, arg2);

    if (strcmp(command, "print") == 0) {
        // Print the value of the variable
        if (numArgs == 2) {
            printf("%s\n", arg1);
        }
    }
    else if (strcmp(command, "assign") == 0) {
        // Assign value to variable
        if (numArgs == 3) {
            printf("Assigning %s to %s\n", arg2, arg1);
            // Handle the assignment logic (e.g., store the value in a variable)
        }
    }
    else if (strcmp(command, "writeFile") == 0) {
        // Write data to a file
        if (numArgs == 3) {
            printf("Writing %s to file %s\n", arg2, arg1);
            // Open file and write data
            FILE* file = fopen(arg1, "w");
            if (file) {
                fprintf(file, "%s\n", arg2);
                fclose(file);
            } else {
                printf("Failed to write to file.\n");
            }
        }
    }
    else if (strcmp(command, "readFile") == 0) {
        // Read data from a file
        if (numArgs == 2) {
            printf("Reading from file %s\n", arg1);
            // Open the file and read data
            FILE* file = fopen(arg1, "r");
            if (file) {
                char line[100];
                while (fgets(line, sizeof(line), file)) {
                    printf("%s", line);
                }
                fclose(file);
            } else {
                printf("Failed to read from file.\n");
            }
        }
    }
    else if (strcmp(command, "printFromTo") == 0) {
        // Print numbers from x to y
        if (numArgs == 3) {
            int x = atoi(arg1);
            int y = atoi(arg2);
            for (int i = x; i <= y; i++) {
                printf("%d ", i);
            }
            printf("\n");
        }
    }
    else if (strcmp(command, "semWait") == 0) {
        // Acquire a semaphore resource
        if (numArgs == 2) {
            printf("Waiting for resource %s\n", arg1);
            // Implement semaphore wait (blocking, etc.)
        }
    }
    else if (strcmp(command, "semSignal") == 0) {
        // Release a semaphore resource
        if (numArgs == 2) {
            printf("Releasing resource %s\n", arg1);
            // Implement semaphore signal (releasing the resource)
        }
    }
    else {
        printf("Unknown command: %s\n", command);
    }
}


