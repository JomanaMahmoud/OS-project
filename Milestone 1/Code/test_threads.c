#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

struct ThreadTiming {
    struct timespec release_time;
    struct timespec start_time;
    struct timespec finish_time;
    struct timespec start_times[100];
    struct timespec finish_times[100];
    int slice_count;
};

struct ThreadTiming t1_timing, t2_timing, t3_timing;


void get_time(struct timespec *ts) {
    clock_gettime(CLOCK_MONOTONIC, ts);
}

double time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void *Thread1(void *arg) {
    usleep(10000); // Sleep for 10ms

      t1_timing.slice_count = 0;

    get_time(&t1_timing.start_times[t1_timing.slice_count]);
    t1_timing.slice_count++;
    get_time(&t1_timing.start_time); 

    char letter1, letter2;
    printf("Please enter 2 letters: ");
    scanf(" %c %c", &letter1, &letter2);

    if (letter1 > letter2) {
        char temp = letter1;
        letter1 = letter2;
        letter2 = temp;
    }

    printf("Thread 1 Output: ");
    for (char c = letter1; c <= letter2; c++) {
        printf("%c ", c);
    }
    printf("\n");

    get_time(&t1_timing.finish_times[t1_timing.slice_count - 1]);
    get_time(&t1_timing.finish_time); // Add this before exit


    pthread_exit(NULL);
}

void *Thread2(void *arg) {
    usleep(10000); // Sleep for 10ms

    t2_timing.slice_count = 0;
    get_time(&t2_timing.start_times[t2_timing.slice_count]); 
    get_time(&t2_timing.start_time);
    t2_timing.slice_count++;

    

    pthread_t tid2 = pthread_self();
    printf("Running with Thread ID: %lu\n", tid2);
    printf("This is print statement 1 \n");
    printf("This is print statement 2 \n");

    printf("\n");
    
    get_time(&t2_timing.finish_time); 

    get_time(&t2_timing.finish_times[t2_timing.slice_count - 1]);
    pthread_exit(NULL);
}


void *Thread3(void *arg) {
    usleep(10000); // Sleep for 10ms

    t3_timing.slice_count = 0;
    get_time(&t3_timing.start_times[t3_timing.slice_count]);
    t3_timing.slice_count++; 
    get_time(&t3_timing.start_time);

    int num1, num2, sum = 0, product = 1, count = 0;
    float avg;

    printf("Please enter 2 integers: ");
    scanf("%d %d", &num1, &num2);
    printf("\n");
    if (num1 > num2) {
        int temp = num1;
        num1 = num2;
        num2 = temp;
    }

    for (int i = num1; i <= num2; i++) {
        sum += i;
        product *= i;
        count++;
    }
    avg = (float)sum / count;

    printf("Sum: %d\n", sum);
    printf("Average: %.2f\n", avg);
    printf("Product: %d\n", product);

    get_time(&t3_timing.finish_times[t3_timing.slice_count - 1]);
    get_time(&t3_timing.finish_time); 

    pthread_exit(NULL);
}

void print_cpu_load() {
    double loadavg[3]; 

    if (getloadavg(loadavg, 3) != -1) {
        printf("CPU Load Averages: 1min=%.2f, 5min=%.2f, 15min=%.2f\n",
               loadavg[0], loadavg[1], loadavg[2]);
    } else {
        perror("getloadavg");
    }
}




int main() {
    

   
    double start_time, end_time;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    pthread_t t1, t2, t3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);

    get_time(&t1_timing.release_time);
    pthread_create(&t1, &attr, Thread1, NULL);

    get_time(&t2_timing.release_time);
    pthread_create(&t2, &attr, Thread2, NULL);

    get_time(&t3_timing.release_time);
    pthread_create(&t3, &attr, Thread3, NULL);

    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    pthread_attr_destroy(&attr);

    float execution1 = 0, execution2 = 0, execution3 = 0;
    for (int i = 0; i < t1_timing.slice_count; i++) {
        execution1 += time_diff(t1_timing.start_times[i], t1_timing.finish_times[i]);
    }
    for (int i = 0; i < t2_timing.slice_count; i++) {
        execution2 += time_diff(t2_timing.start_times[i], t2_timing.finish_times[i]);
    }
    for (int i = 0; i < t3_timing.slice_count; i++) {
        execution3 += time_diff(t3_timing.start_times[i], t3_timing.finish_times[i]);
    }

    printf("\nPerformance Metrics:\n");
   
    printf("\nThread 1:\n");

float turn1 = time_diff(t1_timing.release_time, t1_timing.finish_time);
float waiting1 = turn1 - execution1;
float cpu_useful_work_Thread1 = execution1/(execution1+waiting1);
float response_time_1 = time_diff(t1_timing.release_time,t1_timing.start_time);

printf("Release Time: %.6f sec\n", time_diff((struct timespec){0}, t1_timing.release_time));
printf("Start Time: %.6f sec\n", time_diff((struct timespec){0}, t1_timing.start_time));
printf("Finish Time: %.6f sec\n", time_diff((struct timespec){0}, t1_timing.finish_time));
printf("Execution Time: %.6f sec\n", execution1);
printf("Response Time: %.6f sec\n", response_time_1);
printf("Turnaround Time: %.6f sec\n", turn1);
printf("Waiting Time: %.6f sec\n", waiting1);
printf("CPU useful work: %.6f\n", cpu_useful_work_Thread1);

printf("\nThread 2:\n");

float turn2 = time_diff(t2_timing.release_time, t2_timing.finish_time);
float waiting2 = turn2 - execution2;
float cpu_useful_work_Thread2 = execution2/(execution2+waiting2);
float response_time_2 = time_diff(t2_timing.release_time,t2_timing.start_time);
printf("Release Time: %.6f sec\n", time_diff((struct timespec){0}, t2_timing.release_time));
printf("Start Time: %.6f sec\n", time_diff((struct timespec){0}, t2_timing.start_time));
printf("Finish Time: %.6f sec\n", time_diff((struct timespec){0}, t2_timing.finish_time));
printf("Execution Time: %.6f sec\n", execution2);
printf("Response Time: %.6f sec\n", response_time_2);
printf("Turnaround Time: %.6f sec\n", turn2);
printf("Waiting Time: %.6f sec\n", waiting2);
printf("CPU useful work: %.6f\n", cpu_useful_work_Thread2);

printf("\nThread 3:\n");

float turn3 = time_diff(t3_timing.release_time, t3_timing.finish_time);
float waiting3 = turn3 - execution3;
float cpu_useful_work_Thread3 = execution3/(execution3+waiting3);
float response_time_3 = time_diff(t3_timing.release_time,t3_timing.start_time);
printf("Release Time: %.6f sec\n", time_diff((struct timespec){0}, t3_timing.release_time));
printf("Start Time: %.6f sec\n", time_diff((struct timespec){0}, t3_timing.start_time));
printf("Finish Time: %.6f sec\n", time_diff((struct timespec){0}, t3_timing.finish_time));
printf("Execution Time: %.6f sec\n", execution3);
printf("Response Time: %.6f sec\n", response_time_3);
printf("Turnaround Time: %.6f sec\n", turn3);
printf("Waiting Time: %.6f sec\n", waiting3);
printf("CPU useful work: %.6f\n", cpu_useful_work_Thread3);

float cpu_Average_Utilization = ((cpu_useful_work_Thread1 + cpu_useful_work_Thread2 + cpu_useful_work_Thread3)/3)*100;

 printf("CPU Average Utilization: %.6f%%\n", cpu_Average_Utilization);
 

 print_cpu_load();
 FILE *file = fopen("/proc/self/status", "r");
 if (file) {
     char line[256];
     while (fgets(line, sizeof(line), file)) {
         if (strncmp(line, "VmRSS:", 6) == 0) {  
             printf("Memory Consumption: %s", line);
             break;
         }
     }
     fclose(file);
 } else {
     printf("Memory Consumption: Unable to retrieve\n");
 }


printf("\nMain Thread: All threads finished execution.\n");
return 0;

}