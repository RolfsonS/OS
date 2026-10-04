#include <stdint.h>
#include <stddef.h>

#include <arch/i386/vmm.h>

#include <mm/heap.h>
#include <mm/pmm.h>

#include <lib/kprintf.h>

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
