#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"

struct MemoryBlock{
    void* ptr;
    size_t blockSize;
    bool is_free;
    MemoryBlock* next;
    MemoryBlock* prev;
};

void* request_memory(size_t size);
void  release_memory(void* addr, size_t size);
void* request_block(size_t size, MemoryBlock* first_block);
void  release_block(MemoryBlock* first_block);
MemoryBlock* first_fit(size_t size, MemoryBlock* block);
MemoryBlock* best_fit(size_t size, MemoryBlock* block);
MemoryBlock* worst_fit(size_t size, MemoryBlock* block);
void compact(MemoryBlock* firstBlock, size_t allocated_size);
void  report(MemoryBlock* first_block);
#endif