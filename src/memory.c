#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"
#include "../include/memory.h"

#define MEMORY_SIZE 60

MemoryCell memory[MEMORY_SIZE];

void initialize_memory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory[i].name[0] = '\0';
        memory[i].data[0] = '\0';
        memory[i].isInstruction = 0;
    }
}

void write_to_memory(int index, const char* name, const char* data, int isInstruction) {
    if (index >= 0 && index < MEMORY_SIZE) {
        strncpy(memory[index].name, name, sizeof(memory[index].name) - 1);
        memory[index].name[sizeof(memory[index].name) - 1] = '\0';

        strncpy(memory[index].data, data, sizeof(memory[index].data) - 1);
        memory[index].data[sizeof(memory[index].data) - 1] = '\0';

        memory[index].isInstruction = isInstruction;
    }
}

MemoryCell read_from_memory(int index) {
    if (index >= 0 && index < MEMORY_SIZE) {
        return memory[index];
    }
    MemoryCell empty = { "", "", 0 };
    return empty;
}
