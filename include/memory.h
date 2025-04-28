#ifndef MEMORY_H
#define MEMORY_H
#define MEMORY_SIZE 60

typedef struct {
    char name[20];    // e.g., "State", "x", or "Instruction"
    char data[40];    // e.g., "READY", "5", or "print x"
    int isInstruction; // 1 = instruction, 0 = variable or PCB
} MemoryCell;

extern MemoryCell memory[MEMORY_SIZE];  // Declaration of memory array

void initialize_memory();
void write_to_memory(int index, const char* name, const char* data, int isInstruction);
MemoryCell read_from_memory(int index);

#endif
