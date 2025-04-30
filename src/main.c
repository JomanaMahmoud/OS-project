#include <stdio.h>
#include <stdlib.h>
#include "../include/process.h"
#include "../include/memory.h"
#include "../include/interpreter.h"


int main() {
    initializeProcessFromFile("../Program_1.txt", 1, generalReadyQueue);
    initializeProcessFromFile("../Program_2.txt", 2, generalReadyQueue);
    initializeProcessFromFile("../Program_3.txt", 3, generalReadyQueue);
    for (int i = 0; i < 60; i++) {
        if (memory[i].name[0] != '\0') {  
            printf("%s\n", memory[i].data);  
        }
    }
    printf("\n"); 
}
