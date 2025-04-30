#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "readyQueue.h"

// Function to perform FCFS scheduling
void startFCFS(ReadyQueue* readyQueue);

// Function to perform Round Robin scheduling (not implemented yet)
void startRoundRobin(ReadyQueue* readyQueue, int quantum);

// Function to perform Multilevel Feedback Queue scheduling (not implemented yet)
void mlfq_schedule(ReadyQueue* queues[4]);

#endif
