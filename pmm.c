#include <stddef.h>
#include <stdint.h>
#include "pmm.h"
#include "multiboot.h"
#include "kprintf.h"
#include "string.h"

#define MAX_PAGES 4096

/* Global bit map for memory managment */
uint8_t bit_buffer[MAX_PAGES];
uint8_t* bitmap;

/* Defined via linker */
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void pmm_init(multiboot_info_t* mbd, uint32_t magic) {
	
	/* Verify the Magic Value is correct */
	if (magic != 0x2BADB002) {
//		kprintf("Invalid MAGIC VALUE");
	} else {
//		kprintf("Valid magic value: %x\n", magic);
	}

	/* Verify that bit 6 of the flags field is set. 
	 * Tells us whether certain memory map information is available.
	 */
	if(!(mbd -> flags >> 6 & 0x1)) {
//		kprintf("Invalid flag set");
	} else {
//		kprintf("Correct flag set: %b \n", (uint8_t) mbd->flags);
	}
		
	//kprintf("Start: %x, End: %x", &_kernel_start, &_kernel_end);
	/* Zero bitmap out, and init valid memory regions */
	zero_bitmap();
	init_bitmap(mbd);
	reserve_kernel();
}

void zero_bitmap() {
	bitmap = bit_buffer;
	memset(bitmap, 0, MAX_PAGES);	
}

void init_bitmap(multiboot_info_t* mbd) {
	/* Represents our current memory iteration */
	uint32_t curr_mem = 0x0;

	/* Iterate through all of the mmap sections individually */
	for(size_t i = 0; i < mbd -> mmap_length; i += sizeof(memory_map_t)) {

		/* Obtain the memory map pointer */
		memory_map_t* mmp = (memory_map_t*) ((uint32_t)(mbd -> mmap_addr + i) + 0xC0000000);
		uint32_t segment_low   = mmp -> base_addr_low;
		uint32_t segment_limit =  segment_low + (mmp -> length_low);

		/* Indicates we found a large gap between memory regions.
		 * We are also not going to use unmapped memory! */
		if (curr_mem < segment_low) {
			curr_mem = find_nearest_page(segment_low);
		}

		/* Iterate through all available 4K regions of that memory */
		for (; curr_mem < segment_limit; curr_mem += 0X1000) {
			/* If a region is marked as usable, mark the corresponding bit 1.
			 * However, be ware of a boundary region, we do not allocate these. */
			if(mmp -> type == 0x1 && (curr_mem + 0x1000) < segment_limit) {

				/* All memory locations are marked as reserved before this.
				 * So mark the correct ones free */
				free_pmm(curr_mem);
			}
		}
	}
}

void* free_pmm(uint32_t page) {

	uint32_t entry_byte = page / 0x8000;
	uint32_t entry_bit = (page / 0x1000) % 0x8;

	uint8_t comparator = (uint8_t) 0x80 >> entry_bit;
	bitmap[entry_byte] |= comparator;

	return (void*) page;
}

void* allocate_pmm(uint32_t page) {
	uint32_t entry_byte = page / 0x8000;
	uint32_t entry_bit = (page / 0x1000) % 0x8;

	uint8_t comparator = (uint8_t) 0x80 >> entry_bit;
	bitmap[entry_byte] ^= comparator;

	return (void*) page;
}

void print_byte(uint32_t memory_loc) {
	uint32_t entry_byte = memory_loc / 0x8000;
	uint32_t entry_bit = (memory_loc / 0x1000) % 0x8;
	 
	kprintf("Entry Byte: %u, Entry bit: %u \n", entry_byte, entry_bit);
	kprintf("Updated Bitmap Location: %b \n", bitmap[entry_byte]);
}

uint32_t find_nearest_page(uint32_t curr_addr) {
	curr_addr += 0xFFF;
	curr_addr /= 0x1000;
	curr_addr *= 0x1000;

	return curr_addr;
}

void* allocate_page(void) {
	/* Iterate through all possible pages, checking for next empty slot */
	size_t curr_byte = 0;
	while(curr_byte < MAX_PAGES) {
		/* Each bit is a page */
		uint8_t count = 0;
		for(size_t curr_bit = 7; curr_bit > 0; curr_bit--) {
			if((bitmap[curr_byte] >> curr_bit) && 0x1) {
				uint32_t memory_loc = (curr_byte * 0x8000) + (count * 0x1000);
				uint32_t* allocated = allocate_pmm(memory_loc);
//				kprintf("Phy mem allocated: %x\n", allocated);
				return allocated;
			}	
			count++;
		}
	}
//	kprintf("Broken");
	/* Nothing is found */
	return NULL;
}

void deallocate_page(uint32_t addr) {
	free_pmm(addr);
}

void reserve_kernel() {
//	kprintf("start: %x \n", &_kernel_start);
//	kprintf("end: %x \n", &_kernel_end);
	//uint32_t new_ker_start = _kernel_start + 0xC000000;
	for (size_t kernel_mem = (size_t)&_kernel_start; kernel_mem <= (size_t)&_kernel_end - 0xC0000000; kernel_mem += 0x1000) {
		allocate_pmm(kernel_mem);
	}
}

