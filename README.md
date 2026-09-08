# Contiguous memory allocator - Project
We'll be using mmap instead of malloc, because
mmap guarantee me a raw page-aligned block of virtual memory with total control over every byte. Where malloc doesnt even guarantees contiguous block of memory and its internal headers might be oversaved by compaction.
Compaction is much safer than in mmap.

### Todo
1. implement:
	1. request for contigugous block of memory
	2. release of conitguous block of memory
	3. compact
	4. report