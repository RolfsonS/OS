#ifndef HEAP_H
#define HEAP_H

void* kmalloc(size_t size);

void free(uint32_t* loc);

void* fake_kmalloc(size_t size);

void init_heap();
#endif
