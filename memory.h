#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"
void* request_memory(size_t size);
void  release_memory(void* addr, size_t size);
void  compact(void* addr);
void  report(void* addr);
#endif