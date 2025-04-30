
#include <stdlib.h>
#include "blockedQueue.h"

// Initialize the queue

void initBlockedQueue(BlockedQueue* queue) {
    queue->size = 0;
}

// Adds a process to the end of the queue
void enqueueBlocked(BlockedQueue* queue, PCB* process) {
    if (queue->size < MAX_BLOCKED) {
        queue->processes[queue->size++] = process;
        process->state = BLOCKED;
    }
}

// Dequeues the first process (used for general blocked queue)
PCB* dequeueGeneral(BlockedQueue* queue) {
    if (queue->size == 0) return NULL;

    PCB* first = queue->processes[0];
    for (int i = 1; i < queue->size; ++i)
        queue->processes[i - 1] = queue->processes[i];
    queue->size--;
    return first;
}

// Dequeues the highest-priority process (used for resource queues)
PCB* dequeuePriority(BlockedQueue* queue) {
    if (queue->size == 0) return NULL;

    int highestIndex = 0;
    for (int i = 1; i < queue->size; ++i) {
        if (queue->processes[i]->priority < queue->processes[highestIndex]->priority) {
            highestIndex = i;
        }
    }

    PCB* selected = queue->processes[highestIndex];
    for (int i = highestIndex + 1; i < queue->size; ++i)
        queue->processes[i - 1] = queue->processes[i];
    queue->size--;
    return selected;
}

void removeFromQueue(BlockedQueue* queue, PCB* process) {
    int foundIndex = -1;
    for (int i = 0; i < queue->size; ++i) {
        if (queue->processes[i] == process) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        for (int i = foundIndex + 1; i < queue->size; ++i) {
            queue->processes[i - 1] = queue->processes[i];
        }
        queue->size--;
    }
}

