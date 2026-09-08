#include "memory.h"
#include <unistd.h>
#include <sys/mman.h>
void* request_memory(size_t size){
    void* out = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(out == MAP_FAILED)
        return NULL;
    return out; 
}
void release_memory(void* ptr, size_t size){
    int out = munmap(ptr, size);
    if(out < 0)
        ERR("munmap");
}