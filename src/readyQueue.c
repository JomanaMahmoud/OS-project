#include "readyQueue.h"
#include <stdio.h>

void initReadyQueue(ReadyQueue* q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

int isReadyQueueEmpty(ReadyQueue* q) {
    return q->count == 0;
}

int isReadyQueueFull(ReadyQueue* q) {
    return q->count == MAX_PROCESSES;
}

void enqueueReadySortedByArrival(ReadyQueue* q, PCB pcb) {
    if (isReadyQueueFull(q)) {
        printf("Ready Queue is full. Cannot add process %d\n", pcb.pid);
        return;
    }

    // Find correct position based on arrival time
    int i = q->rear;
    int insertPos = (q->rear + 1) % MAX_PROCESSES;

    // Shift elements to the right to make space
    while (q->count > 0) {
        int prevIndex = (i + MAX_PROCESSES) % MAX_PROCESSES;
        if (q->queue[prevIndex].arrival_time <= pcb.arrival_time) {
            break;
        }
        q->queue[insertPos] = q->queue[prevIndex];
        insertPos = prevIndex;
        i--;
    }

    q->queue[insertPos] = pcb;
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->count++;
}

PCB* dequeueReady(ReadyQueue* q) {
    if (isReadyQueueEmpty(q)) {
        printf("Ready Queue is empty.\n");
        return NULL;  // Return NULL if the queue is empty
    }
    PCB* p = &q->queue[q->front];  // Return a pointer to the first PCB in the queue
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->count--;
    return p;  // Return the pointer to the PCB
}

