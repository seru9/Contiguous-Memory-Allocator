#include <stdio.h>
#define REQUEST_SIZE 16
#define INPUT_BUFFER_SIZE 256

void program(){
    char   input_bufer[INPUT_BUFFER_SIZE];
    char   request[REQUEST_SIZE]; // RQ - request, RL - release, STAT - statistics, C - Compaction
    char   process[REQUEST_SIZE];
    size_t bytes_number;
    char   alghorithm; // F - First Fit, B - Best Fit, W - Worst Fit
    int  shall_exit = 0;
    while(!shall_exit){
        fgets(input_bufer, INPUT_BUFFER_SIZE, stdin); // for now only \n at the end
        sscanf(input_bufer, "%15s %15s %zd %c", request, process, &bytes_number, &alghorithm);
        printf("Request: %s Process: %s bytes_number: %zd algo: %c \n", request, process, bytes_number, alghorithm);
    }
}
int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <size> \n", argv[0]);
        return 1;
    }   
    size_t allocated_size = (size_t)argv[1];
    program();
    return 0;
}