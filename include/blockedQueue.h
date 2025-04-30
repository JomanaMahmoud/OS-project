#ifndef BLOCKED_QUEUE_H
#define BLOCKED_QUEUE_H

#include "process.h"  // For PCB* definition

#define MAX_BLOCKED 3  // Max number of blocked processes

typedef struct {
    PCB* processes[MAX_BLOCKED];
    int size;
} BlockedQueue;

// Initializes a blocked queue
void initBlockedQueue(BlockedQueue* queue);

// Enqueues a process into the blocked queue
void enqueueBlocked(BlockedQueue* queue, PCB* process);

// Dequeues the first process (for general blocked queue)
PCB* dequeueGeneral(BlockedQueue* queue);

// Dequeues the highest-priority process (for resource-specific queues)
PCB* dequeuePriority(BlockedQueue* queue);

// Removes a specific process from the blocked queue
void removeFromQueue(BlockedQueue* queue, PCB* process);

#endif
