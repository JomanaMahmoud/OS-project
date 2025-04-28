#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "readyQueue.h"

// Function to perform FCFS scheduling
void startFCFS(JobPool* jobPool, ReadyQueue* readyQueue);

// Function to perform Round Robin scheduling (not implemented yet)
void startRoundRobin(JobPool* jobPool, ReadyQueue* readyQueue, int quantum);

// Function to perform Multilevel Feedback Queue scheduling (not implemented yet)
void mlfq_schedule(JobPool* jobPool, ReadyQueue* queues[4]);

#endif
