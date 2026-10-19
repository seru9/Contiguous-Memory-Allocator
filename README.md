# Contiguous Memory Allocator

A educational contiguous memory allocator written in C. Instead of
relying on `malloc`, it reserves one region of virtual memory with `mmap` and
manages every byte of it by hand: allocating blocks, freeing them, coalescing
neighbours, and compacting the region on demand.

This is a learning project focused on how allocators work under the hood:
block headers, free lists, first/best/worst-fit placement, fragmentation, and
compaction.

## Why `mmap` instead of `malloc`

`mmap` returns a raw, page-aligned block of virtual memory that the allocator
has total control over. `malloc` does not guarantee a contiguous block and hides
its own bookkeeping headers inside the returned region, which would be corrupted
by compaction. Managing an `mmap` region directly makes the memory layout fully
transparent and compaction safe.

## Memory layout

The entire region is treated as an array of self-describing blocks laid out
consecutively:

```
 base                                    end
  |                                        |
  v                                        v
+----------------+----------------+------- +----------------+----------------+--------+
| MemoryBlock    | payload        | ...    | MemoryBlock    | payload        | free   |
| header         | (user data)    |        | header         | (user data)    | space  |
+----------------+----------------+--------+----------------+----------------+--------+
```

Each `MemoryBlock` header is stored inline in the region and holds:

```c
struct MemoryBlock {
    void*         ptr;        // start of the user payload
    size_t        blockSize;  // size of the payload in bytes
    bool          is_free;
    MemoryBlock*  next;       // next block in address order
    MemoryBlock*  prev;       // previous block in address order
};
```

Because the headers live inside the region (an intrusive doubly-linked list),
the allocator tracks the whole heap without any external bookkeeping.

## Features

- **Backing memory** – `request_memory` / `release_memory` wrap `mmap` / `munmap`.
- **Allocation** – `request_block` carves a block out of a free one, splitting off
  the remainder as a new free block.
- **Placement strategies** – `first_fit`, `best_fit`, and `worst_fit` all exist.
  The requested strategy is selected from the command input.
- **Freeing with coalescing** – `release_block` marks a block free and merges it
  with a free previous and/or next neighbour, keeping the list fragmented as
  little as possible.
- **Compaction** – `compact` slides every live block to the front of the region
  and merges all resulting free space into a single trailing block.
- **Reporting** – `report` walks the list and prints each block's address, size,
  and free flag.
- **Alignment** – every requested size is rounded up to a multiple of
  `sizeof(void*)` with `ALIGN_UP_POW2`.

## Project structure

| File        | Purpose                                                            |
| ----------- | ------------------------------------------------------------------ |
| `common.h`  | Shared includes, fixed-width typedefs, size macros, alignment helpers |
| `memory.h`  | `MemoryBlock` definition and allocator API                         |
| `memory.c`  | Allocator implementation (mmap, fit strategies, coalescing, compact) |
| `main.c`    | Interactive driver / command loop                                  |
| `Makefile`  | Build rules                                                        |

## Build

```sh
make
```

This produces the `main` executable. To remove build artifacts:

```sh
make clean
```

## Usage

```sh
./main <size> <number_of_processes>
```

- `<size>` – size in bytes of the region to manage (before header overhead).
- `<number_of_processes>` – roughly how many processes/blocks you expect. The
  allocator reserves `sizeof(MemoryBlock)` per process for headers. Pass `-1` if
  you don't know and it will use a default approximation.

The program then reads commands from standard input, one per line, and exits
cleanly on `Ctrl+C` (SIGINT).

### Commands

| Command                     | Description                                                |
| --------------------------- | ---------------------------------------------------------- |
| `RQ <process> <bytes> <algo>` | Request a block of `bytes` for `process`. `<algo>` is `F` (first fit), `B` (best fit), or `W` (worst fit). |
| `RL <process>`              | Release the block belonging to `process`.                  |
| `STAT`                      | Print the current block list (address, size, free flag).   |
| `C`                         | Compact the heap and move live blocks to the front.        |

### Memory Allocation 

You will always be assigned at least the amount of memory you ask (or not if there is no block big enough).
Assigned memory might be bigger if the size given is not the multiplication of the size(void*). On 64-bit systems it's 8 byte, on 32-bit systems it's 4 byte.

When allocating new (free) block, available memory is size of new block - size of structure with metadata of a block. 

### Example session

```text
$ ./main 4096 10
Memory starts at 0x7f... size: 4496
RQ p1 100 F
Request: RQ Process: p1 bytes_number: 100 algo: F
Return Block addr: 0x7f...5028 with the size 104
New Block addr: 0x7f...50b8 with the size 4416
STAT
Address: 0x7f...5028 of the size 104 is_free: 0
Address: 0x7f...50b8 of the size 4416 is_free: 1
Releasing memory starting at 0x7f... of a size of 4496
```

## Status / roadmap

- [x] Request a contiguous block of memory
- [x] Release a contiguous block of memory (with coalescing)
- [x] Compaction
- [x] Report / statistics
- [x] Hook up compaction to a command (`C`)
- [x] Select `best_fit` / `worst_fit` from the input algorithm flag
- [x] Track blocks per process so `RL <process>` releases the correct block
