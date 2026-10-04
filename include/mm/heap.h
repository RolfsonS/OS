#ifndef HEAP_H
#define HEAP_H

void* kmalloc(size_t size);

void free(uint32_t* loc);

void init_heap();
#endif
