typedef struct MemoryBlock MemoryBlock;

#include "memory.h"
#include <stdlib.h>
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

void* request_block(size_t size, MemoryBlock* first_block){
    MemoryBlock* returnBlock = first_fit(size, first_block);
    returnBlock->is_free = false;

    size_t aligned_request_size = ALIGN_UP_POW2(size, MEMORY_ALIGN);
    size_t returnBlockSize = returnBlock->blockSize;
    
    returnBlock->blockSize = aligned_request_size;

    if(returnBlockSize < sizeof(MemoryBlock)){
        goto return_ptr;
    }

    void* new_block_ptr = (u8*)(returnBlock->ptr) + aligned_request_size;
    MemoryBlock* newBlock = (MemoryBlock*)new_block_ptr;

    newBlock->ptr = (u8*)new_block_ptr + sizeof(MemoryBlock);
    newBlock->is_free = true;
    newBlock->blockSize = returnBlockSize - sizeof(MemoryBlock);
    newBlock->next = returnBlock->next == NULL ? NULL : returnBlock->next;
    newBlock->prev = returnBlock;

    returnBlock->next = newBlock;
    printf("Return Block addr: %p with the size %zd \n", returnBlock->ptr, returnBlock->blockSize);
    printf("New Block addr: %p with the size %zd \n", newBlock->ptr, newBlock->blockSize);
    return_ptr: 
        return returnBlock->ptr;
}
void release_block(MemoryBlock* block){ 
    if(!block) return;
    block->is_free = true;
    MemoryBlock* prevBlock = block->prev;
    MemoryBlock* nextBlock = block->next;
    
    if(prevBlock && prevBlock->is_free && nextBlock && nextBlock->is_free ){
        prevBlock->blockSize += sizeof(MemoryBlock) * 2 + block->blockSize + nextBlock->blockSize;
        prevBlock->next = nextBlock->next;
        if(nextBlock->next){
            nextBlock->next->prev = prevBlock;
        }
        memset(block, 0, sizeof(MemoryBlock));
        memset(nextBlock, 0, sizeof(MemoryBlock));
        printf("DONE1\n");
    }

    else if (prevBlock && prevBlock->is_free) {
        prevBlock->blockSize += sizeof(MemoryBlock) + block->blockSize;
        prevBlock->next = nextBlock;
        if (nextBlock) {
            nextBlock->prev = prevBlock;
        }
        memset(block, 0, sizeof(MemoryBlock));
        printf("DONE2\n");
    }

    else if (nextBlock && nextBlock->is_free) {
        block->blockSize += sizeof(MemoryBlock) + nextBlock->blockSize;
        block->next = nextBlock->next;
        if (nextBlock->next) {
            nextBlock->next->prev = block;
        }
        memset(nextBlock, 0, sizeof(MemoryBlock));
        printf("DONE3\n");
    }
}
void report(MemoryBlock* first_block){
    MemoryBlock* temp = first_block;
    while(temp != NULL){
        printf("Address: %p of the size %zd is_free: %d \n", temp->ptr, temp->blockSize, temp->is_free);
        temp = temp->next;
    }
}
MemoryBlock* first_fit(size_t size, MemoryBlock* block){
    MemoryBlock* temp = block;
    size_t wholeBlockSize = size + sizeof(MemoryBlock);
    while(temp != NULL){
        if(!temp->is_free || temp->blockSize < wholeBlockSize)
        {
            temp = temp->next;
            continue;
        }
        return temp;
    }
    return NULL;
}
MemoryBlock* best_fit(size_t size, MemoryBlock* head) {
    MemoryBlock* temp = head;
    MemoryBlock* best = NULL;

    while (temp != NULL) {
        if (temp->is_free && temp->blockSize >= size) {
            if (best == NULL || temp->blockSize < best->blockSize) {
                best = temp;
            }
        }
        temp = temp->next; 
    }
    return best;
}
MemoryBlock* worst_fit(size_t size, MemoryBlock* head) {
    MemoryBlock* temp = head;
    MemoryBlock* worst = NULL;

    while (temp != NULL) {
        if (temp->is_free && temp->blockSize >= size) {
            if (worst == NULL || temp->blockSize > worst->blockSize) {
                worst = temp;
            }
        }
        temp = temp->next; 
    }
    return worst;
}
void compact(MemoryBlock* firstBlock, size_t allocated_size) {
    if (!firstBlock) return;

    MemoryBlock* temp = firstBlock;
    u8* write_ptr = (u8*)firstBlock;
    MemoryBlock* last_moved = NULL;
    size_t total_active_bytes = 0;

    // Process all nodes in the linked list
    while (temp != NULL) {
        // Save temp->next before memmove potentially overwrites unvisited blocks
        MemoryBlock* next_temp = temp->next;

        if (!temp->is_free) {
            size_t total_size = temp->blockSize + sizeof(MemoryBlock);
            total_active_bytes += total_size;

            // Move header + payload to contiguous write position
            if ((u8*)temp != write_ptr) {
                memmove(write_ptr, temp, total_size);
            }

            MemoryBlock* moved_block = (MemoryBlock*)write_ptr;
            moved_block->ptr = write_ptr + sizeof(MemoryBlock);

            // Rebuild doubly-linked list connections correctly
            moved_block->prev = last_moved;
            if (last_moved != NULL) {
                last_moved->next = moved_block;
            }
            
            last_moved = moved_block;
            write_ptr += total_size;
        }

        temp = next_temp;
    }

    // Append the single merged free block at the end if space remains
    size_t remaining_size = allocated_size - total_active_bytes;
    if (remaining_size >= sizeof(MemoryBlock)) {
        MemoryBlock* freeBlock = (MemoryBlock*)write_ptr;
        freeBlock->ptr = write_ptr + sizeof(MemoryBlock);
        freeBlock->blockSize = remaining_size - sizeof(MemoryBlock);
        freeBlock->is_free = true;
        freeBlock->next = NULL;
        freeBlock->prev = last_moved;

        if (last_moved != NULL) {
            last_moved->next = freeBlock;
        }
    } else if (last_moved != NULL) {
        last_moved->next = NULL;
    }
}