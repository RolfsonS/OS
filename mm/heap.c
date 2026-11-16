#include <stdint.h>
#include <stddef.h>

#include <arch/i386/vmm.h>

#include <mm/heap.h>
#include <mm/pmm.h>

#include <lib/kprintf.h>

/* 11/15/2026 -- Rewriting this for a SLAB allocator.
 * I don't expect this to be super similar to Linux (obviously), but the current "bump" allocator 
 * is not sufficient.
 *
 * kmalloc acts as a wrapper for a bunch of kmem_caches, which each contain slabs.
 * As a result, kmalloc functionality should be maintained. 
 * */

/* Redefinition of kmem_cache for use inside kmem_slab. */
typedef struct kmem_cache_t;

/* kmem_slabs are placed at the start of an allocated slab. 
 * A free list is used to maintain what to allocate.
 * If all setups occur at kernel boot time, this is very cheap. */
typedef struct kmem_slab_t {
	/* A pointer to the first free memory location. */
	uint32_t* free;
	
	/* Pointer to the kmem_cache that owns this slab. */
	struct kmem_cache_t* parent_cache;

	/* Pointer to the next and previous slabs.
	 * If we have to delete one, just remove from doubly linked list. */
	struct kmem_slab* next, prev;

} kmem_slab;

/* kmem_cache represents the memory cache which we select free chunks from 
 * based on whatever slab is free.
 * */
typedef struct kmem_cache_t {
	/* The size of this cache level, meaning an allocated chunk takes up
	 * this much memory */
	uint16_t size;

	/* Name of the cache. */
	const char* cache_name;

	/* Pointers to empty, full, and partially used slabs. */
	kmem_slab* empty, full, partial;

} kmem_cache;

/* kmem_caches is an array of kmem_cache objects. 
 * */
kmem_cache kmem_caches[] = {
		
}

uint32_t heap_start 	= 0xC1000000; 
uint32_t heap_curr	= 0xC1000000;
uint32_t heap_end 	= 0xC1000000;

void* kmalloc(size_t size) {
	if(size == 0) {
		return NULL;
	}
	uint8_t* return_heap = (uint8_t*)heap_curr;
	heap_curr += size;

	if(heap_curr >= heap_end) {
		map_page(heap_end, (uint32_t)allocate_page(), P | RW);
		heap_end += 0x1000;
	}

//	kprintf("SIZE ALLOC: %x, CURR HEAP: %x\n", size, return_heap);

	return return_heap;
}



void free(uint32_t* loc) {	
	// Doing nothing rn lol
	//
	*loc = 0x0;
}


void init_heap() {
	map_page(heap_start, (uint32_t)allocate_page(), P | RW);
	heap_end += 0x1000;
}
