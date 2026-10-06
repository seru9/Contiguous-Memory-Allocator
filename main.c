typedef struct MemoryBlock MemoryBlock;
#include "common.h"
#include "memory.h"
#include <signal.h>
#include <string.h>

volatile sig_atomic_t last_signal = 0;
void SIGINT_handler(int sig){
    last_signal = sig;
}
int sethandler(void (*f)(int), int sigNo) {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}
void program(size_t allocated_size){
    void*  ptr = request_memory(allocated_size);
    MemoryBlock* last_block = (MemoryBlock*)ptr;
    last_block->ptr = (u8*)ptr + sizeof(MemoryBlock);
    last_block->blockSize = allocated_size - sizeof(MemoryBlock);
    last_block->is_free = true;
    last_block->next = NULL;
    last_block->prev = NULL;
    printf("Memory starts at %p size: %zd \n", ptr, allocated_size);
    char   input_bufer[INPUT_BUFFER_SIZE];
    char   request[REQUEST_SIZE]; // RQ - request, RL - release, STAT - statistics, C - Compaction
    char   process[REQUEST_SIZE];
    size_t bytes_number;
    char   alghorithm; // F - First Fit, B - Best Fit, W - Worst Fit
    int    shall_exit = 0;
    while(last_signal != SIGINT && !shall_exit){
        char* s = fgets(input_bufer, INPUT_BUFFER_SIZE, stdin); // for now only \n at the end
        if(s == NULL || strcmp(s, "\n") == 0 || strcmp(s, "\r") == 0){
            if(last_signal == SIGINT)
                break;
            ERR("fgets"); // to change
        }
        sscanf(input_bufer, "%15s %15s %zd %c", request, process, &bytes_number, &alghorithm);
        printf("Request: %s Process: %s bytes_number: %zd algo: %c \n", request, process, bytes_number, alghorithm);
        if(strcmp(request, "RQ") == 0){
            request_block(bytes_number, last_block);
        }
        if(strcmp(request, "STAT") == 0){
            report(last_block);
        }
        if(strcmp(request, "RL") == 0){
            release_block(last_block);
        } 

    }
    release_memory(ptr, allocated_size);
    printf("Releasing memory starting at %p of a size of %zd \n", ptr, allocated_size);
}
int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <size> <number of processes (approximately) write -1 if don't know> \n", argv[0]);
        return 1;
    }   
    sethandler(SIGINT_handler, SIGINT);
    int number_of_processes = atoi(argv[2]);
    size_t processes_size = number_of_processes == -1 ? sizeof(MemoryBlock) * APROX_PROCESS_COUNT : sizeof(MemoryBlock) * number_of_processes;
    size_t allocated_size = ALIGN_UP_POW2((size_t)atoi(argv[1]) + processes_size, MEMORY_ALIGN);
    program(allocated_size);
    return 0;
}