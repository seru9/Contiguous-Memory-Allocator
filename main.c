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
    last_block->process[0] = '\0';
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
        if (sscanf(input_bufer, "%15s", request) != 1) {
            continue;
        }

        if(strcmp(request, "RQ") == 0){
            if (sscanf(input_bufer, "%15s %15s %zd %c", request, process, &bytes_number, &alghorithm) != 4) {
                fprintf(stderr, "Invalid RQ command: %s", input_bufer);
                continue;
            }
            printf("Request: %s Process: %s bytes_number: %zd algo: %c \n", request, process, bytes_number, alghorithm);
            MemoryBlock* existing = find_process_block(last_block, process);
            void* allocated_ptr = request_block(bytes_number, last_block, alghorithm, process);
            if (!allocated_ptr) {
                fprintf(stderr, "Allocation failed for process %s\n", process);
                continue;
            }
            if (existing) {
                release_block(existing);
            }
            continue;
        }

        if(strcmp(request, "STAT") == 0){
            report(last_block);
            continue;
        }

        if(strcmp(request, "RL") == 0){
            if (sscanf(input_bufer, "%15s %15s", request, process) != 2) {
                fprintf(stderr, "Invalid RL command: %s", input_bufer);
                continue;
            }
            MemoryBlock* block = find_process_block(last_block, process);
            if (!block) {
                fprintf(stderr, "Process not found: %s\n", process);
                continue;
            }
            release_block(block);
            continue;
        }

        if(strcmp(request, "C") == 0){
            compact(last_block, allocated_size);
            continue;
        }

        fprintf(stderr, "Unknown command: %s\n", request);

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