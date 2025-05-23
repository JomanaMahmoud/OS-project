#define MAX_QUEUE_SIZE 3
#define MEMORY_SIZE 60
#include <stddef.h> // For NULL
#include <stdio.h>  // For FILE, fopen, etc.
#include <string.h>
#include <stdlib.h> // For malloc, free
#define MAX_PROCESSES 10
#include <ctype.h> // For isspace()
#include <stdbool.h> // For bool type
#include <glib.h>
void trim(char* str) {
    char* end;

    // Trim leading spaces
    while (isspace((unsigned char)*str)) str++;

    // If the string is all spaces, return an empty string
    if (*str == 0) {
        *str = '\0';
        return;
    }

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write the null terminator
    *(end + 1) = '\0';
}

typedef enum {New, READY, RUNNING, BLOCKED ,TERMINATED} ProcessState;

typedef struct {
    char name[32];        // "Instruction", "PID", etc.
    char data[64];        // The actual instruction or value
    int isInstruction;    // 1 if it's an instruction, 0 otherwise
} MemoryCell;

MemoryCell memory[60];
int memoryPointer = 0; // Points to the next free memory cell


typedef struct PCB {
    int pid;                    // Process ID
    ProcessState state;         // Current process state
    int priority;               // Priority level
    int programCounter;         // Next instruction to execute
    int memoryLowerBound;       // Start of memory allocation
    int memoryUpperBound;       // End of memory allocation         
    } PCB;

typedef struct {
    int pid;
    int priority;
    int arrivalTime;
    char filename[64];
    int isCreated; // 0 until the process is created
} ProcessRegistration;

ProcessRegistration pendingProcesses[3]; // or any upper limit
int pendingCount = 0;
int nextPID = 1;
int clockCycle = -1;
char schedulerName[32];

typedef struct {
    PCB* items[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

Queue globalBlockedQueue; 
void append_to_output(const char* text);
void append_to_user_output(const char* text);

typedef struct {
    int isAvailable;
    Queue* blockedQueue;  // Priority queue
} Mutex;

Mutex mutex_userInput;
Mutex mutex_userOutput;
Mutex mutex_file;

Queue readyQueue;
PCB* runningProcess = NULL;
int quantum = 2;
int quantumCounter = 0;

// Global function pointer for the input handler
char* (*frontendInputHandler)(const char* prompt_message) = NULL;

// Function to set the input handler
void setInputHandler(char* (*handler)(const char* prompt_message)) {
    frontendInputHandler = handler;
}

void initQueue(Queue* q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    
}

int isEmpty(Queue* q) {
    return q->size == 0;
}

int isFull(Queue* q) {
    return q->size == MAX_QUEUE_SIZE;
}

void enqueue(Queue* q, PCB* pcb) {
    if (isFull(q)) return;

    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->items[q->rear] = pcb;
    q->size++;
}

PCB* dequeue(Queue* q) {
    if (isEmpty(q)) return NULL;

    PCB* pcb = q->items[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return pcb;
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    initQueue(q);
    return q;
}
bool isQueueEmpty(Queue* q) {
    return q->size == 0;
}
void enqueueByPriority(Queue* q, PCB* pcb) {
    if (isFull(q)) return;

    int insertPos = q->rear + 1;
    if (insertPos >= MAX_QUEUE_SIZE) insertPos = 0;

    // Shift elements to make space (basic insertion sort logic)
    int i = q->size - 1;
    int idx = (q->front + i) % MAX_QUEUE_SIZE;
    while (i >= 0 && q->items[idx]->priority < pcb->priority) {
        int nextIdx = (idx + 1) % MAX_QUEUE_SIZE;
        q->items[nextIdx] = q->items[idx];
        i--;
        idx = (q->front + i) % MAX_QUEUE_SIZE;
    }

    int insertIdx = (q->front + i + 1) % MAX_QUEUE_SIZE;
    q->items[insertIdx] = pcb;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->size++;
}

void removeFromReadyQueue(PCB* pcb) {
    int i, count = readyQueue.size;
    int idx = readyQueue.front;
    Queue temp;
    initQueue(&temp);

    for (i = 0; i < count; i++) {
        PCB* current = dequeue(&readyQueue);
        if (current->pid != pcb->pid) {
            enqueue(&temp, current);
        }
    }
    // Copy back
    readyQueue = temp;
}
void removeFromBlockedQueue(Queue* q, PCB* pcb) {
    int i, count = q->size;
    Queue temp;
    initQueue(&temp);

    for (i = 0; i < count; i++) {
        PCB* current = dequeue(q);
        if (current->pid != pcb->pid) {
            enqueue(&temp, current);
        }
    }
    *q = temp; // Replace old queue
}

void initializeMutexes() {
    mutex_userInput.isAvailable = 1;
    mutex_userInput.blockedQueue = createQueue();

    mutex_userOutput.isAvailable = 1;
    mutex_userOutput.blockedQueue = createQueue();

    mutex_file.isAvailable = 1;
    mutex_file.blockedQueue = createQueue();
}
Mutex* getMutexByName(char* name) {
    if (strcmp(name, "userInput") == 0) return &mutex_userInput;
    if (strcmp(name, "userOutput") == 0) return &mutex_userOutput;
    if (strcmp(name, "file") == 0) return &mutex_file;
}
// Define these globally or within your MLFQ file
Queue* L1;
Queue* L2;
Queue* L3;
Queue* L4;

void printQueue(Queue* q) {
    if(q == &globalBlockedQueue){
        printf("\n🟨 Global Blocked Queue:\n");
    }
    if (q == L1) {
        printf("\n🟨 Level L1:\n");
    } else if (q == L2) {
        printf("\n🟨 Level L2:\n");
    } else if (q == L3) {
        printf("\n🟨 Level L3:\n");
    } else if (q == L4) {
        printf("\n🟨 Level L4:\n");
    } 
    
   
    if (q->size == 0) {  // Check if the queue is empty based on size
        printf(" (empty)\n");
        return;
    }    
    int idx = q->front;
    for (int i = 0; i < q->size; i++) {
        PCB* currentPCB = q->items[idx];
        printf("  → PID %d at memory[%d]\n", currentPCB->pid, currentPCB->memoryLowerBound);
        idx = (idx + 1) % MAX_QUEUE_SIZE;
    }
}
void enqueueAtLevel(PCB* p, int level) {
    switch (level) {
        case 1: enqueue(L1, p); break;
        case 2: enqueue(L2, p); break;
        case 3: enqueue(L3, p); break;
        case 4: enqueue(L4, p); break;
    }
}

Queue* getReadyQueueByLevel(int level) {
    switch (level) {
        case 1:
            return L1;
        case 2:
            return L2;
        case 3:
            return L3;
        case 4:
            return L4;
        default:
            printf("Invalid level: %d\n", level);
            
    }
}
int semWait(char* resourceName, PCB* pcb) {
    trim(resourceName);
    printf("Process %d is waiting for resource %s\n", pcb->pid, resourceName);
    char output[256];

    snprintf(output, sizeof(output), "Process %d is waiting for resource %s\n", pcb->pid, resourceName);
    append_to_output(output); 
    Mutex* m = getMutexByName(resourceName);

    // Check if the mutex is found
    if (m == NULL) {
        printf("Error: Mutex with resource name %s not found.\n", resourceName);
        snprintf(output, sizeof(output), "Error: Mutex with resource name %s not found.\n", resourceName);
        append_to_output(output); // Append error to the general output section
        return -1;  // Handle error, possibly return an error code
    }

    if (m->isAvailable) {
        m->isAvailable = 0;
        printf("Resource %s is now locked by process %d\n", resourceName, pcb->pid);
        snprintf(output, sizeof(output), "Resource %s is now locked by process %d\n", resourceName, pcb->pid);
        append_to_output(output); // Append to the general output section
        return 0; // Process continues
    }
    
    else
    {// Block the process
    pcb->state = BLOCKED;
    enqueueByPriority(m->blockedQueue, pcb);
    enqueueByPriority(&globalBlockedQueue, pcb);  // optional: general queue
    removeFromReadyQueue(pcb);  // Remove from ready queue
    return 1; // Process is blocked
    }
    
}

void semSignal(char* resourceName) {
    trim(resourceName);
    Mutex* m = getMutexByName(resourceName);
    char output[256];
    if (m == NULL) {
        printf("Error: Mutex with resource name %s not found.\n", resourceName);
        
        return;
    }

    printf("Process %d is signaling resource %s\n", runningProcess->pid, resourceName);

    if (!isEmpty(m->blockedQueue)) {
        PCB* next = dequeue(m->blockedQueue);
        removeFromBlockedQueue(&globalBlockedQueue, next);
        next->state = READY;

        if (strcmp(schedulerName, "mlfq") == 0) {
            enqueue(getReadyQueueByLevel(next->priority), next);
        } else {
            enqueue(&readyQueue, next);
        }

        m->isAvailable = 0;
    } else {
        m->isAvailable = 1;
    }
}


void initializeProcess(const char* filename, int priority, int arrivalTime) {
    if (pendingCount >= 3) {
    printf("Error: Maximum number of processes reached.\n");
    return;
    }
    ProcessRegistration* proc = &pendingProcesses[pendingCount++];

    proc->pid = nextPID++;
    proc->priority = priority;
    proc->arrivalTime = arrivalTime;
    proc->isCreated = 0;
    strcpy(proc->filename, filename);
    
    printf("Process registered: PID = %d, arrivalTime = %d, priority = %d, file = %s\n",
           proc->pid, proc->arrivalTime, proc->priority, proc->filename);
}


char* getVariableValue(PCB* pcb, const char* varName) {
    for (int i = pcb->memoryLowerBound; i <= pcb->memoryUpperBound; i++) {
        if (strcmp(memory[i].name, varName) == 0) {
            return memory[i].data;
        }
    }
    return NULL;  // If the variable is not found
}
void setVariableValue(PCB* pcb, const char* varName, const char* value) {
    for (int i = pcb->memoryLowerBound; i <= pcb->memoryUpperBound; i++) {
        if (strcmp(memory[i].name, varName) == 0) {
            strcpy(memory[i].data, value);
            return;
        }
    }
}

char* readFromFile(const char* filename) {
    static char buffer[100];  // Static buffer to hold the file data
    FILE* file = fopen(filename, "r");  // Open the file for reading
    if (file == NULL) {
        printf("Error: Could not open file %s for reading.\n", filename);
        return NULL;  // Return NULL if the file could not be opened
    }

    // Read the first line of the file into the buffer
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Remove the newline character at the end of the line if present
        buffer[strcspn(buffer, "\n")] = 0;
    } else {
        printf("Error: Could not read from file %s.\n", filename);
        fclose(file);
        return NULL;
    }

    // Close the file
    fclose(file);

    return buffer;  // Return the data read from the file
}

void writeToFile(const char* data, const char* filename) {
    // Open the file for writing (create if doesn't exist)
    FILE* file = fopen(filename, "a");  // "a" for append mode
    if (file == NULL) {
        printf("Error: Could not open file %s for writing.\n", filename);
        return;
    }

    // Write the data to the file
    fprintf(file, "%s\n", data);

    // Close the file
    fclose(file);
}


void executeOneInstruction(PCB* pcb) {
    int pc = pcb->programCounter;

    // Step 1: Check if there is a valid instruction to execute
    if (pc > pcb->memoryUpperBound) {
        char output[256];
        snprintf(output, sizeof(output), "PID %d: No instruction to execute at memory[%d].\n", pcb->pid, pc);
        append_to_output(output);
        return;
    }

    // Check if the current memory entry is a valid instruction
    if (strcmp(memory[pc].name, "Instruction") != 0 || memory[pc].isInstruction != 1) {
        char output[256];
        snprintf(output, sizeof(output), "PID %d: Invalid or missing instruction at memory[%d]: name='%s', isInstruction=%d\n",
                 pcb->pid, pc, memory[pc].name, memory[pc].isInstruction);
        append_to_output(output);
        return;
    }

    // Step 2: Retrieve and print the instruction being executed
    char* instr = memory[pc].data;
    char output[512];
    snprintf(output, sizeof(output), "Executing Instruction #%d: %s | PID: %d\n", pc - pcb->memoryLowerBound + 1, instr, pcb->pid);
    append_to_output(output); // Append to the general output section

    // Step 3: Tokenize the instruction for further processing
    char* tokens[3];
    char buffer[100];
    strcpy(buffer, instr);  // Copy the instruction into a buffer for tokenization
    char* token = strtok(buffer, " ");
    int i = 0;
    while (token && i < 3) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }

    // Step 4: Handle different types of instructions
    if (strcmp(tokens[0], "print") == 0) {
        // Example: print x
        trim(tokens[1]);
        trim(tokens[0]);

        // Retrieve the value of the variable to print
        char* value = getVariableValue(pcb, tokens[1]);
        if (value) {
            snprintf(output, sizeof(output), "Output: %s = %s\n", tokens[1], value);
            append_to_user_output(output); // Append to the user output section
        } else {
            snprintf(output, sizeof(output), "Error: Variable %s not found.\n", tokens[1]);
            append_to_user_output(output); // Append error to the user output section
        }
    } else if (strcmp(tokens[0], "assign") == 0) {
        // Step 1: Tokenize the full instruction safely
        char* fullInstr = memory[pc].data;
        char buffer[100];
        strcpy(buffer, fullInstr);
        char* token = strtok(buffer, " ");
        char* assignTokens[4] = {NULL};
    
        int j = 0;
        while (token != NULL && j < 4) {
            assignTokens[j++] = token;
            token = strtok(NULL, " ");
        }
    
        // Check for at least 3 tokens (assign x y) or 4 (assign x readFile y)
        if (assignTokens[1] == NULL || assignTokens[2] == NULL) {
            printf("PID %d: Malformed assign instruction.\n", pcb->pid);
            return;
        }
    
        trim(assignTokens[1]); // variable name
        trim(assignTokens[2]); // value or nested instruction
    
        if (strcmp(assignTokens[2], "input") == 0) {
            if (frontendInputHandler) {
                char *input = frontendInputHandler("Please enter a value:");
                if (input) {
                    setVariableValue(pcb, tokens[1], input);
                    snprintf(output, sizeof(output), "Assigned %s = %s\n", tokens[1], input);
                    append_to_output(output); // Append to the general output section
                    g_free(input);
                } else {
                    append_to_output("Error: No input provided.\n");
                }
            } else {
                append_to_output("Error: No input handler set for frontend input.\n");
            }
        }else if (strcmp(tokens[0], "printFromTo") == 0) {
            // Example: printFromTo x y (where x and y are variable names or numbers)
            trim(tokens[1]);
            trim(tokens[2]);
        
            // Retrieve the values of x and y
            int from = atoi(getVariableValue(pcb, tokens[1]));
            int to = atoi(getVariableValue(pcb, tokens[2]));
        
            // Generate the output header
            char output[512];
            snprintf(output, sizeof(output), "Output: Printing numbers from %d to %d:\n", from, to);
            append_to_user_output(output); // Append the header to the user output section
        
            // Append the range of numbers to the user output
            for (int i = from; i <= to; i++) {
                snprintf(output, sizeof(output), "%d ", i);
                append_to_user_output(output);
            }
        
            // Add a newline at the end
            append_to_user_output("\n");
        }

        else if (strcmp(assignTokens[2], "readFile") == 0) {
            if (assignTokens[3] == NULL) {
                printf("PID %d: Missing argument for readFile in assign.\n", pcb->pid);
                return;
            }
            trim(assignTokens[3]);
            char* fileName = getVariableValue(pcb, assignTokens[3]);
            if (fileName == NULL) {
                printf("PID %d: Variable %s not found for readFile.\n", pcb->pid, assignTokens[3]);
                return;
            }
            char* val = readFromFile(fileName);
            setVariableValue(pcb, assignTokens[1], val);
        } else {
            // Regular assignment from a variable or literal
            char* val = getVariableValue(pcb, assignTokens[2]);
            setVariableValue(pcb, assignTokens[1], val);
        }
    }
     else if (strcmp(tokens[0], "writeFile") == 0) {
        // Example: writeFile x y
        writeToFile(getVariableValue(pcb, tokens[2]), getVariableValue(pcb, tokens[1]));
    } else if (strcmp(tokens[0], "readFile") == 0) {
        // Example: readFile x
        char* val = readFromFile(getVariableValue(pcb, tokens[1]));
        setVariableValue(pcb, "x", val); // Store read value in variable 'x'
    
    } else if (strcmp(tokens[0], "semWait") == 0) {
        
        // Example: semWait semaphoreName
        
        if ((semWait(tokens[1], pcb)==1)) { 
            printf("Process %d is blocked on semaphore %s\n", pcb->pid, tokens[1]);
             }
             
         // Block process if semaphore is taken
    } else if (strcmp(tokens[0], "semSignal") == 0) {
        
        // Example: semSignal semaphoreName
        semSignal(tokens[1]);
        
    }else if (strcmp(tokens[0], "printFromTo") == 0) {
        // Example: printFromTo x y (where x and y are variable names or numbers)
        trim(tokens[1]);
        trim(tokens[2]);
    
        // Retrieve the values of x and y
        int from = atoi(getVariableValue(pcb, tokens[1]));
        int to = atoi(getVariableValue(pcb, tokens[2]));
    
        // Generate the output header
        char output[512];
        snprintf(output, sizeof(output), "Output: Printing numbers from %d to %d:\n", from, to);
        append_to_user_output(output); // Append the header to the user output section
    
        // Append the range of numbers to the user output
        for (int i = from; i <= to; i++) {
            snprintf(output, sizeof(output), "%d ", i);
            append_to_user_output(output);
        }
    
        // Add a newline at the end
        append_to_user_output("\n");
    }
     else {
        printf("Unknown instruction: %s\n", tokens[0]);
    }
    
    // Step 5: Increment the program counter
    pcb->programCounter++;
}


int isProcessFinished(PCB* pcb) {
    return pcb->programCounter >= pcb->memoryUpperBound+1;
}
void freeMemory(PCB* pcb) {
    // Reset the memory cells allocated for the finished process
    for (int i = pcb->memoryLowerBound; i <= pcb->memoryUpperBound; i++) {
        // Clear the memory cell by resetting its fields to default values
        memory[i].name[0] = '\0';  // Clear the name
        memory[i].data[0] = '\0';  // Clear the data
        memory[i].isInstruction = 0;  // Mark it as no instruction
    }

    printf("Memory freed for PID %d from memory[%d] to memory[%d]\n", 
           pcb->pid, pcb->memoryLowerBound, pcb->memoryUpperBound);
}
void stepFirstComeFirstServe() {
    // Step 1: Check if any process is currently running
    if (runningProcess == NULL) {
        // If no process is running, start the first process in the ready queue
        if (!isEmpty(&readyQueue)) {
            runningProcess = dequeue(&readyQueue);  // Dequeue the first process (FCFS)
        } else {
            return; // No process to run
        }
    }

    // Step 2: Execute one instruction of the current running process
    executeOneInstruction(runningProcess);
    
    // Increment program counter to the next instruction
    

    // Step 3: Check if the process has finished
    if (isProcessFinished(runningProcess)) {
        freeMemory(runningProcess);  // Free up memory occupied by the process
        runningProcess = NULL;  // Process finished, clear the running process
    }
    
    // No quantum handling is required in FCFS since it's non-preemptive
    // The process continues executing until it's finished.
}

void stepRoundRobin() {
    // Step 1: If no process is running, fetch the next one from the ready queue
    if (runningProcess == NULL) {
        if (!isEmpty(&readyQueue)) {
            runningProcess = dequeue(&readyQueue);
            quantumCounter = 0;
        } else {
            return; // No process to run
        }
    }

    // Step 2: Execute one instruction of the current running process
    executeOneInstruction(runningProcess);
    
    quantumCounter++;

    // Step 3: Check if the process is finished
    if (isProcessFinished(runningProcess)) {
        freeMemory(runningProcess);
        runningProcess = NULL;
        quantumCounter = 0;
    }
    // Step 4: If quantum is reached, preempt and re-enqueue
    else if (quantumCounter == quantum) {
        enqueue(&readyQueue, runningProcess);
        runningProcess = NULL;
        quantumCounter = 0;
    }
}


// Define these globally or within your MLFQ file

// Helper function to remove a process from its queue
void removeProcessFromQueue(Queue* queue, PCB* process) {
    int i;
    // Search for the process in the queue
    for (i = queue->front; i != queue->rear; i = (i + 1) % MAX_QUEUE_SIZE) {
        if (queue->items[i] == process) {
            // Shift the items to the left
            while (i != queue->rear) {
                int nextIndex = (i + 1) % MAX_QUEUE_SIZE;
                queue->items[i] = queue->items[nextIndex];
                i = nextIndex;
            }
            // Clear the rear (after shifting)
            queue->rear = (queue->rear - 1 + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
            queue->size--;
            break;
        }
    }
}

void step_MLFQ() {
    static int executedInQuantum = 0;
    static int lastPID = -1;
    static PCB* lastUsedL4Process = NULL;  // For round robin at level 4

    // If there's no running process, pick one
    if (runningProcess == NULL) {
        for (int level = 1; level <= 4; level++) {
            Queue* q = getReadyQueueByLevel(level);
            if (!isQueueEmpty(q)) {
                if (level == 4) {
                    // Round Robin: rotate until we find the next process
                    int size = q->size;
                    for (int i = 0; i < size; i++) {
                        PCB* candidate = dequeue(q);
                        if (lastUsedL4Process == NULL || candidate->pid != lastUsedL4Process->pid) {
                            runningProcess = candidate;
                            break;
                        }
                        enqueue(q, candidate);  // rotate
                    }
                    if (runningProcess == NULL) {
                        // If not found (only 1 process?), just pick the front
                        runningProcess = dequeue(q);
                    }
                    lastUsedL4Process = runningProcess;
                } else {
                    runningProcess = dequeue(q);
                }
                runningProcess->state = RUNNING;
                executedInQuantum = 0;
                lastPID = runningProcess->pid;
                break;
            }
        }
    }

    if (runningProcess == NULL) return;

    // Reset counter if new process is running
    if (runningProcess->pid != lastPID) {
        executedInQuantum = 0;
        lastPID = runningProcess->pid;
    }

    int level = runningProcess->priority;
    int currentQuantum = 1 << (level - 1);  // 1, 2, 4, 8

    executeOneInstruction(runningProcess);
    executedInQuantum++;

    // Check if the process is finished
    if (isProcessFinished(runningProcess)) {
        // Remove finished process from the ready queue
        Queue* q = getReadyQueueByLevel(runningProcess->priority);
        removeProcessFromQueue(q, runningProcess);

        runningProcess = NULL;
        executedInQuantum = 0;
        return;
    }

    // If process became BLOCKED or TERMINATED
    if (runningProcess->state == BLOCKED || runningProcess->state == TERMINATED) {
        // Remove terminated or blocked process from the ready queue
        Queue* q = getReadyQueueByLevel(runningProcess->priority);
        removeProcessFromQueue(q, runningProcess);

        if (runningProcess->state == BLOCKED && runningProcess->priority < 4)
            runningProcess->priority++;  // Promote blocked process

        runningProcess = NULL;
        executedInQuantum = 0;
        return;
    }

    // If quantum expired
    if (executedInQuantum >= currentQuantum) {
        if (runningProcess->priority < 4)
            runningProcess->priority++;
        runningProcess->state = READY;

        // For L4, re-enqueue to rotate Round Robin
        Queue* targetQueue = getReadyQueueByLevel(runningProcess->priority);
        enqueue(targetQueue, runningProcess);

        runningProcess = NULL;
        executedInQuantum = 0;
    }
}



void insertToMemory(int index, const char* name, const char* data, int isInstruction) {
    strcpy(memory[index].name, name);
    strcpy(memory[index].data, data);
    memory[index].isInstruction = isInstruction;
}

char* intToString(int num) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}

void checkArrivals(int clockCycle) {
    for (int i = 0; i < pendingCount; i++) {
        ProcessRegistration* proc = &pendingProcesses[i];
        if (proc->arrivalTime == clockCycle && proc->isCreated == 0) {
            printf("Creating process PID %d at clock %d\n", proc->pid, clockCycle);
            int start = memoryPointer; // Lower bound

            // ───── Create a new PCB ─────
            PCB* newPCB = (PCB*)malloc(sizeof(PCB));  // Dynamically allocate memory for PCB
            newPCB->pid = proc->pid;
            newPCB->state = New;
            newPCB->priority = proc->priority;
            newPCB->programCounter = 9+start;  // Placeholder for PC
            newPCB->memoryLowerBound = start;
            newPCB->memoryUpperBound = -1;  // To be updated later

            // ───── PCB Entries in memory ─────
            insertToMemory(memoryPointer++, "PID", intToString(newPCB->pid), 0);
            insertToMemory(memoryPointer++, "State", "New", 0);
            insertToMemory(memoryPointer++, "Priority", intToString(newPCB->priority), 0);
            insertToMemory(memoryPointer++, "PC", "0", 0); // placeholder for program counter
            insertToMemory(memoryPointer++, "LowerBound", intToString(start), 0);
            
            int upperBoundIndex = memoryPointer++; // reserve UpperBound slot

            // ───── Variables ─────
            insertToMemory(memoryPointer++, "a", "", 0);
            insertToMemory(memoryPointer++, "b", "", 0);
            insertToMemory(memoryPointer++, "c", "", 0);

            // ───── Instructions ─────
            FILE* f = fopen(proc->filename, "r");
            if (!f) {
                printf("Error: Cannot open file %s\n", proc->filename);
                free(newPCB);  // Free allocated PCB memory if file cannot be opened
                return;
            }

            int pcAddress = memoryPointer;

            char line[128];
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = 0;
                insertToMemory(memoryPointer++, "Instruction", line, 1);
            }

            fclose(f);

            int end = memoryPointer - 1;

            // Update PC to point to first instruction address
            sprintf(memory[start + 3].data, "%d", pcAddress); // index 3 is PC
            
            // Update UpperBound now that end is known
            sprintf(memory[upperBoundIndex].data, "%d", end);
            newPCB->memoryUpperBound = end;
            
            // Finalize PCB and enqueue it
            newPCB->state = READY;
            if(strcmp(schedulerName, "mlfq") == 0){
                enqueueAtLevel(newPCB, 1); // Enqueue to the first level
            }else{  
                enqueue(&readyQueue, newPCB); // Enqueue to the ready queue
            }
         

            proc->isCreated = 1;
            
            printf("Process PID %d loaded into memory from [%d to %d] and enqueued to ready queue.\n", newPCB->pid, start, end);
            
            // Remove the created process from pendingProcesses
            for (int j = i; j < pendingCount - 1; j++) {
                pendingProcesses[j] = pendingProcesses[j + 1];
            }
            pendingCount--;  // Decrease the number of pending processes
            i--;  // Adjust the loop counter to recheck the new process at the same index
        }
    }
}


void printMemory() {
    printf("\n📦 Memory Contents:\n");
    for (int i = 0; i < memoryPointer; i++) {
        const char* nameToPrint = strlen(memory[i].name) > 0 ? memory[i].name : "<empty>";
        printf("[%02d] name: %-12s | data: %-24s | instr: %d\n",
               i, nameToPrint, memory[i].data, memory[i].isInstruction);
    }
}

void printReadyQueue() {
    printf("\n🟩 Ready Queue:\n");
    if (readyQueue.size == 0) {  // Check if the queue is empty based on size
        printf(" (empty)\n");
        return;
    }    
    int idx = readyQueue.front;
    for (int i = 0; i < readyQueue.size; i++) {
        PCB* currentPCB = readyQueue.items[idx];
        printf("  → PID %d at memory[%d]\n", currentPCB->pid, currentPCB->memoryLowerBound);
        idx = (idx + 1) % MAX_QUEUE_SIZE;
    }
}



void printPendingProcesses() {
    printf("\n📋 Pending Processes:\n");
    for (int i = 0; i < pendingCount; i++) {
    ProcessRegistration* proc = &pendingProcesses[i];
    printf(" PID %d | arrival: %d | priority: %d | file: %-12s | created: %s\n",
    proc->pid, proc->arrivalTime, proc->priority,
    proc->filename, proc->isCreated ? "yes" : "no");
    }
    }

void passTurn(){
    clockCycle++;
    printf("\n⏱️ Clock Cycle: %d\n", clockCycle);
    // 1. Check and create new processes if their arrival time is met
    checkArrivals(clockCycle);
    
//printQueue(&globalBlockedQueue);
//printReadyQueue();
// printQueue(L1);
// printQueue(L2);
// printQueue(L3);
// printQueue(L4);

// 2. Call the appropriate scheduler for this clock tick
if (strcmp(schedulerName, "firstComeFirstServe") == 0) {
    stepFirstComeFirstServe();
} else if (strcmp(schedulerName, "roundRobin") == 0) {
    stepRoundRobin();
} else if (strcmp(schedulerName, "mlfq") == 0) {
    step_MLFQ(clockCycle); // Optional: define this when ready
} else {
    printf("⚠️  Unknown scheduler: %s\n", schedulerName);
}
//printPendingProcesses();
//printMemory();
//printReadyQueue();

printMLFQToOutput();
// 3. Increment global clock

}
    

void printSchedulerName() {
    printf("🔄 Current Scheduler: %s\n", schedulerName);
}

// You might want to create a function to set the scheduler name
// This can be called whenever you change scheduling algorithms
void setSchedulerName(const char* name) {
    strncpy(schedulerName, name, sizeof(schedulerName) - 1);
    schedulerName[sizeof(schedulerName) - 1] = '\0';
}
void printAllProcesses() {
    printf("\n=== PROCESS LIST ===\n");
    printf("%-10s %-10s %-10s %-10s %-15s %-15s\n", 
           "PID", "State", "Priority", "Memory Lower", "Mempory Upper", "PC");
    printf("-------------------------------------------------------\n");
    
    int processFound = 0;
    
    // Print currently running process (if any)
    if (runningProcess != NULL) {
        printf("%-10d %-15s %-10d %-10d-%-12d %-15d\n", 
               runningProcess->pid, 
               "Running", 
               runningProcess->priority, 
               runningProcess->memoryLowerBound, 
               runningProcess->memoryUpperBound,
               runningProcess->programCounter);
        processFound = 1;
    }
    
    // Print processes in ready queue
    if (readyQueue.size > 0) {
        int index = readyQueue.front;
        int count = 0;
        
        while (count < readyQueue.size) {
            PCB* process = readyQueue.items[index];
            printf("%-10d %-15s %-10d %-10d-%-12d %-15d\n", 
                   process->pid, 
                   "Ready", 
                   process->priority, 
                   process->memoryLowerBound, 
                   process->memoryUpperBound,
                   process->programCounter);
            
            index = (index + 1) % MAX_QUEUE_SIZE; // Circular queue navigation
            count++;
            processFound = 1;
        }
    }
    
    // Print processes in blocked queue
    if (globalBlockedQueue.size > 0) {
        int index = globalBlockedQueue.front;
        int count = 0;
        
        while (count < globalBlockedQueue.size) {
            PCB* process = globalBlockedQueue.items[index];
            printf("%-10d %-15s %-10d %-10d-%-12d %-15d\n", 
                   process->pid, 
                   "Blocked", 
                   process->priority, 
                   process->memoryLowerBound, 
                   process->memoryUpperBound,
                   process->programCounter);
            
            index = (index + 1) % MAX_QUEUE_SIZE; // Circular queue navigation
            count++;
            processFound = 1;
        }
    }
    
    if (!processFound) {
        printf("No active processes found.\n");
    }
    
    printf("===================\n\n");
}

    

void create_gui();  // Forward declaration

int main()
{
    // strcpy(schedulerName, "mlfq");  // Set the scheduler name

    initQueue(&readyQueue);  // ✅ Initialize the queue first
    initQueue(&globalBlockedQueue);  // ✅ Initialize the global blocked queue
    initializeMutexes();  // ✅ Initialize mutexes

    // Initialize the MLFQ levels
    L1 = createQueue();
    L2 = createQueue();
    L3 = createQueue();
    L4 = createQueue();
    

    //   // ✅ Then register the process
    //   initializeProcess("Program_1.txt", 1, 1);
    //   initializeProcess("Program_2.txt", 1, 3);
    //   initializeProcess("Program_3.txt", 1, 0);
    // printf("memory[9]: %s\n", memory[9].data);  // or any appropriate data type


    printSchedulerName();
    printPendingProcesses();
  
    create_gui(); 
 
    
   
    // Launch GTK dashboard
    
    return 0;
}