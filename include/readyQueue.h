#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "process.h"

#define MAX_PROCESSES 10

typedef struct {
    PCB queue[MAX_PROCESSES];
    int front;
    int rear;
    int count;
} ReadyQueue;

void initReadyQueue(ReadyQueue* q);
int isReadyQueueEmpty(ReadyQueue* q);
int isReadyQueueFull(ReadyQueue* q);
void enqueueReady(ReadyQueue* q, PCB pcb);
PCB* dequeueReady(ReadyQueue* q);

#endif
