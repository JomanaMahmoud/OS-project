#ifndef MUTEX_H
#define MUTEX_H
#include "process.h"
#include "blockedQueue.h"

// Mutex structure
typedef struct {
    int isLocked;               // 0 = unlocked, 1 = locked
    PCB* owner;                 // Current process using the mutex
    BlockedQueue blockedQueue;  // Priority queue of blocked processes
} Mutex;

// Declare all mutexes
extern Mutex mutexFile;
extern Mutex mutexUserInput;
extern Mutex mutexUserOutput;

// Initializes all mutexes
void initMutexes();

// Semaphore operations
void semWait(Mutex* mutex, PCB* process, BlockedQueue* generalBlockedQueue);
PCB* semSignal(Mutex* mutex, BlockedQueue* generalBlockedQueue);

#endif
