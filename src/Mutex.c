#include "mutex.h"
#include "readyQueue.h"
#include <stdio.h>

Mutex mutexFile;
Mutex mutexUserInput;
Mutex mutexUserOutput;

void initMutexes() {
    mutexFile.isLocked = 0;
    mutexFile.owner = NULL;
    initBlockedQueue(&mutexFile.blockedQueue);

    mutexUserInput.isLocked = 0;
    mutexUserInput.owner = NULL;
    initBlockedQueue(&mutexUserInput.blockedQueue);

    mutexUserOutput.isLocked = 0;
    mutexUserOutput.owner = NULL;
    initBlockedQueue(&mutexUserOutput.blockedQueue);
}

void semWait(Mutex* mutex, PCB* process, BlockedQueue* generalBlockedQueue) {
    if (mutex->isLocked == 0) {
        // Mutex is available
        mutex->isLocked = 1;
        mutex->owner = process;
    } else {
        // Mutex is locked — block the process
        enqueueBlocked(&mutex->blockedQueue, process);
        enqueueBlocked(generalBlockedQueue, process);
        process->state = BLOCKED;
    }
}

PCB* semSignal(Mutex* mutex, BlockedQueue* generalBlockedQueue,ReadyQueue* ReadyQueue) {
    if (mutex->owner == NULL || mutex->isLocked == 0) {
        return NULL; // nothing to release
    }

    if (mutex->blockedQueue.size > 0) {
        // Unblock highest priority process
        PCB* nextProcess = dequeuePriority(&mutex->blockedQueue);
        removeFromQueue(generalBlockedQueue, nextProcess);
        nextProcess->state = READY;
        enqueueReadySortedByArrival(ReadyQueue, nextProcess);
        mutex->owner = nextProcess;
        return nextProcess;
    } else {
        // No one waiting — release mutex
        mutex->isLocked = 0;
        mutex->owner = NULL;
        return NULL;
    }
}
