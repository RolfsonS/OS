#ifndef HEAP_H
#define HEAP_H

void* kmalloc(size_t size);

void free(void* loc);

void init_heap();
#endif
