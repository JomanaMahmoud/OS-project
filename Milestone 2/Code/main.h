#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h> // For bool type
#include <stdio.h>   // For FILE, fopen, etc.
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For string operations

// Constants
#define MAX_QUEUE_SIZE 3
#define MEMORY_SIZE 60
#define MAX_PROCESSES 10

// Enums
typedef enum { New, READY, RUNNING, BLOCKED, TERMINATED } ProcessState;

// Structures
typedef struct {
    char name[32];        // "Instruction", "PID", etc.
    char data[64];        // The actual instruction or value
    int isInstruction;    // 1 if it's an instruction, 0 otherwise
} MemoryCell;

typedef struct PCB {
    int pid;                    // Process ID
    ProcessState state;         // Current process state
    int priority;               // Priority level
    int programCounter;         // Next instruction to execute
    int memoryLowerBound;       // Start of memory allocation
    int memoryUpperBound;       // End of memory allocation
} PCB;

typedef struct {
    PCB* items[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

typedef struct {
    int pid;
    int priority;
    int arrivalTime;
    char filename[64];
    int isCreated; // 0 until the process is created
} ProcessRegistration;

typedef struct {
    int isAvailable;
    Queue* blockedQueue;  // Priority queue
} Mutex;

// Global Variables
extern MemoryCell memory[MEMORY_SIZE];
extern int memoryPointer;

extern ProcessRegistration pendingProcesses[MAX_PROCESSES];
extern int pendingCount;
extern int nextPID;
extern int clockCycle;
extern char schedulerName[32];

extern Queue readyQueue;
extern Queue globalBlockedQueue;
extern PCB* runningProcess;

extern Mutex mutex_userInput;
extern Mutex mutex_userOutput;
extern Mutex mutex_file;

extern Queue* L1;
extern Queue* L2;
extern Queue* L3;
extern Queue* L4;

// Function Declarations
void trim(char* str);
void initQueue(Queue* q);
int isEmpty(Queue* q);
int isFull(Queue* q);
void enqueue(Queue* q, PCB* pcb);
PCB* dequeue(Queue* q);
Queue* createQueue();
bool isQueueEmpty(Queue* q);
void enqueueByPriority(Queue* q, PCB* pcb);
void removeFromReadyQueue(PCB* pcb);
void removeFromBlockedQueue(Queue* q, PCB* pcb);
void initializeMutexes();
Mutex* getMutexByName(char* name);
void enqueueAtLevel(PCB* p, int level);
Queue* getReadyQueueByLevel(int level);
int semWait(char* resourceName, PCB* pcb);
void semSignal(char* resourceName);
void initializeProcess(const char* filename, int priority, int arrivalTime);
char* getVariableValue(PCB* pcb, const char* varName);
void setVariableValue(PCB* pcb, const char* varName, const char* value);
char* readFromFile(const char* filename);
void writeToFile(const char* data, const char* filename);
void executeOneInstruction(PCB* pcb);
int isProcessFinished(PCB* pcb);
void freeMemory(PCB* pcb);
void stepFirstComeFirstServe();
void stepRoundRobin();
void step_MLFQ();
void insertToMemory(int index, const char* name, const char* data, int isInstruction);
char* intToString(int num);
void checkArrivals(int clockCycle);
void printMemory();
void printReadyQueue();
void printPendingProcesses();
void passTurn();
void printSchedulerName();
void setSchedulerName(const char* name);
void printAllProcesses();
void create_gui();

#endif // MAIN_H