#include <stdint.h>
#include <stddef.h>

#include <arch/i386/vmm.h>

#include <mm/heap.h>
#include <mm/pmm.h>

#include <lib/kprintf.h>
#include <lib/string.h>

/* 11/15/2026 -- Rewriting this for a SLAB allocator.
 * I don't expect this to be super similar to Linux (obviously), but the current "bump" allocator 
 * is not sufficient.
 *
 * kmalloc acts as a wrapper for a bunch of kmem_caches, which each contain slabs.
 * As a result, existing kmalloc functionality should be maintained. 
 * */

/* Redefinition of kmem_cache for use inside kmem_slab. */
typedef struct kmem_cache_t;

/* Current number of kmem_caches that exist. */
#define KMEM_CACHES		0x8

/* Starting location of the Kernel Heap.
 * For now, this space is 0xC1000000 - 0xD0000000.*/
#define KERNEL_HEAP_START 	0xC1000000u
#define MAX_PAGES 		0x16800u
#define PAGE_SIZE 	 	0x1000

/* Page Bitmap we use for allocations. */
static uint8_t page_bitmap[MAX_PAGES];

/* Enum used to track the states of a cache. 
 * This is helpful as it allows us to distinguish between a 
 * newly allocated slab and an used, now empty slab.
 *
 * This is help, as if we want a thread to clean/deallocate a slab,
 * we need to be able to tell the difference. */
typedef enum slab_state_t {
	NEW = 0x0,
	IN_USE = 0x1,
	USED = 0x2
} slab_state;


/* kmem_slabs are placed at the start of an allocated slab. 
 * A free list is used to maintain what to allocate.
 * If all setups occur at kernel boot time, this is very cheap. */
typedef struct kmem_slab_t {
	/* A pointer to the first free memory location. */
	uint32_t* free;

	/* Current state of this slab. */
	slab_state state;

	/* Pointer to the kmem_cache that owns this slab. */
	struct kmem_cache_t* parent_cache;

	/* Pointer to the next and previous slabs.
	 * If we have to delete one, just remove from doubly linked list. */
	struct kmem_slab_t* next;
	struct kmem_slab_t* prev;

} kmem_slab;

/* kmem_cache represents the memory cache which we select free chunks from 
 * based on whatever slab is free.
 * */
typedef struct kmem_cache_t {
	/* Name of the cache. */
	const char* cache_name;

	/* The size of this cache level, meaning an allocated chunk takes up
	 * this much memory */
	uint16_t size;

	/* Pointers to empty, full, and partially used slabs. */
	kmem_slab* empty; 
	kmem_slab* full;
	kmem_slab* partial;

} kmem_cache;

/* kmem_caches is an array of kmem_cache objects. 
 * */
static kmem_cache kmem_caches[] = {
	{"kmem_cache_8", 8},
	{"kmem_cache_16", 16},
	{"kmem_cache_32", 32},
	{"kmem_cache_64", 64},
	{"kmem_cache_128", 128},
	{"kmem_cache_256", 256},
	{"kmem_cache_512", 512},
	{"kmem_cache_1024", 1024},
};

/* FIXME: For now, these stay, until I fix the allocator. */
uint32_t heap_start 	= 0xC1000000; 
uint32_t heap_curr	= 0xC1000000;
uint32_t heap_end 	= 0xC1000000;

/* Bitmap specfic helper functions. */

/* Intialize the page bitmap to all 0's meaning */
static void init_page_bitmap() {
	memset(page_bitmap, 0xFF, MAX_PAGES);
}

/* Print the allocation/free status of a set of 8 pages via bitmap. */
static void print_byte(uint32_t page) {
	uint32_t bitmap_byte 	= page / 0x8000;
	uint8_t info_byte = page_bitmap[bitmap_byte];

	kprintf("BYTE: %b\n", info_byte);
}

/* Set a bit to 0 in the bitmap, indicating it has been allocated. */
static void set_bit_alloc(uint32_t page) {
	/* Divide page by 8 * 4 KB to find which byte to use. 
	 * Divide page by 4 KB, and % 8 which tells us the entry bit. */
	uint32_t bitmap_byte	 = page / 0x8000;
	uint8_t bitmap_bit 	 = (page / 0x1000) % 0x8;

	/* Base on the entry bit, shift 1000 0000 to the right, and OR bitmap. */
	uint8_t bitmap_mask 	 = 0x80u >> bitmap_bit;
	page_bitmap[bitmap_byte] &= ~bitmap_mask;
}

/* Set a bit to 1 in the bitmap, indicating it has been freed. */
static void set_bit_free(uint32_t page) {
	/* Repeat the same process as above, obviously this time setting to 0. */
	uint32_t bitmap_byte	 = page / 0x8000;
	uint8_t bitmap_bit 	 = (page / 0x1000) % 0x8;
	uint8_t bitmap_mask 	 = 0x80u >> bitmap_bit;
	page_bitmap[bitmap_byte] |= bitmap_mask;
}

/* Scans through the bitmap looking for a 1.
 * Returns the corresponding memory address. */
static uint32_t* find_free_page() {
	/* Loop through all the bytes in the bitmap. 
	 * It is important to remember each byte represents 8 * 4 KB = 32 KB,
	 * so logic at the bit level is required to find a specific page. */
	for(size_t curr_byte = 0; curr_byte < MAX_PAGES; curr_byte++) {
		uint8_t count = 0;
		
		/* Iterate throught the bits. */
		for(int8_t curr_bit = 7; curr_bit >= 0; curr_bit--) {
			
			/* Conditional implies free page. */
			if((page_bitmap[curr_byte]>> curr_bit) & 0x1) {
				/* Obtain the memeory address. */
				uint32_t free_address = (curr_byte * 0x8000);
				free_address += (count * 0x1000);
				
				/* Change out bitmap to mark this as allocated. */
				set_bit_alloc(free_address);

				/* Return free address (Pointer makes it more memory-y) */
				return (uint32_t*) free_address;
			}
			count++;
		}
	}
	/* If nothing found, return NULL. */
	return NULL;
}


/* General page allocation and deallocation. */

/* Return a "memory address" to the start of a page.
 * This function is used for allocated full pages. It is the only
 * interface the add_slab function will see. 
 * So, any additional functional that is later added to determine how the PMM and VMM 
 * allocate pages is done at this function and below. 
 *
 * A bitmap is used to determine a page to allocate, starting at the kernel heap address. */
static uint32_t* page_alloc() {
		
	/* Obtain a free virtual page location from bitmap and mark allocated.*/
	uint32_t* base_page_address = find_free_page();
	set_bit_alloc((uint32_t)base_page_address);

	/* Obtain correct virtual address and memory map. */
	uint32_t final_page_address = KERNEL_HEAP_START + (uint32_t) base_page_address;
	map_page(final_page_address, (uint32_t) allocate_page(), P | RW);
	return (uint32_t*)final_page_address;
}	


/* SLAB allocator heavy lifters. */

/* Add a slab to the empty queue for the repsective cache. 
 * We also chop up the entire slab.
 * TODO: BIG SLABS??? I like this entry from the paper I read:
 * "Therefore, for large objects the physical layout is identical to the logical layout.
 * The required slab and bufctl data structures come from their own (small-object!) caches"
 **/
static void add_slab(kmem_cache* cache) {
	/* First, we request some memory and then cast this to a kmem_slab*. */
	uint32_t* slab_address = page_alloc();
	kmem_slab* slab = (kmem_slab*) slab_address;
	slab -> free = NULL;
	slab -> parent_cache = cache;
	slab -> next = NULL;
	slab -> prev = NULL;

	/* Find the first 8 byte aligned address to begin chopping the block up at. */
	uint8_t starting_free_offset = ((sizeof(kmem_slab) + 7) / 8) * 8;
	uint32_t starting_free_addr = starting_free_offset + (uint32_t) slab_address;

	/* Set the first free block in the slab. */
	slab -> free = (uint32_t*) starting_free_addr;
	kprintf("Start: %x \n", slab -> free);

	/* The condtion in this for loop is a little cumbersome.
	 * Essentially, left of <=, we do not want to potentially overwrite the next page in 
	 * virtual memory. To the right, we know the size of a 4KB page (4KB) and where this slab starts,
	 * so don't go past that. We then increment by our required allocation size. */
	for(uint32_t curr_addr = starting_free_addr;
			curr_addr + (cache -> size) <=        PAGE_SIZE + (uint32_t) slab_address;
			curr_addr += cache -> size) {
		/* Cast current address to a pointer. This makes more intuitive sense. 
		 * Then, we can dereference it, and put a memory location there, that is,
		 * a pointer to the next free adress. */
		uint32_t* curr_free = (uint32_t*) curr_addr;
		*curr_free = curr_addr + cache -> size;
	}

	/* For debugging -- print the first few cache lines. */
	/*for(size_t i = 0; i < 5; i++) {
		uint32_t *curr = (uint32_t*) starting_free_addr;
		kprintf("CURR: %x\n", *curr);

		starting_free_addr += cache -> size;
		curr = (uint32_t*) starting_free_addr;
	}*/

	/* Add this slab to the parent caches empty queue. */
	// FIXME: Change back to slab -> parent_cache -> empty = slab!
	slab -> parent_cache -> partial = slab;
	slab -> state = NEW;
}		

/* Init all the caches. 
 * This involves requesting and mapping memory, then carving the slabs into each of these pages.
 * Each kmem_cache will intially start with only a single slab. 
 *
 * After this, if request can not be satisfied, additional pages are requested and mapped. */
static void init_kmem_caches() {
	/* Zero the bitmap. */
	init_page_bitmap();

	/* Interate through all kmem_caches, initializing the first slabs. */
	for(size_t cache_num = 0; cache_num < KMEM_CACHES; cache_num++) {
		/* Initialize the first fields of the cache. 
		 * size and cache_name are already intialized.*/
		kmem_cache* curr_cache = kmem_caches + cache_num;
		curr_cache -> empty = NULL;
		curr_cache -> full = NULL;
		curr_cache -> partial = NULL;

		/* Initialize the first slab, which belongs to this caches empty by default. */
		add_slab(kmem_caches + cache_num);
	}
}

/* Scan the partial slab, looking for a free address to return. 
 * Also, consider the case where we have to add this slab to the full list. */
static void* scan_partial_slab(kmem_cache* curr_cache) {
	/* Assume we can not find something to begin with. */
	uint32_t* partial_addr = NULL;
	kmem_slab* partial = curr_cache -> partial;
	if(partial) {
		partial_addr = partial -> free;
	}
	
	/* TODO: Two issues I imagine:
	 * 	1. Checking to verify the last address is allocated 
	 * 		- If so, need to create a new slab.
	 * 	2. Checking to verify the last address in a block is allocated (but could be in middle). 
	/* Logic for removing this slab from the partial slab if that slab becomes full. */
	//if(partial_addr) {
	//	if(*partial_addr 
	kprintf("ADDR: %x", *partial_addr);

	return (void*) NULL;
}	

/* Return the first memory segment from the current kmem_cache. */
static void* kmem_cache_alloc(kmem_cache* curr_cache) {
	/* If the current cache has a partial queue, prefer that.
	 * CPU caches will treat us a little better. */
	(void) scan_partial_slab(curr_cache);

	return (void*) NULL;
}


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


/* Fake function we are using for now... this will eventually replace kmalloc. */
void* fake_kmalloc(size_t size) {
	/* If size == 0, they asked for no memory, so return NULL. */
	if(size == 0) {
		return NULL;
	}

	/* Find the cache with the smallest chunk that fits this request. */
	for(size_t cache_num = 0; cache_num < KMEM_CACHES; cache_num++) {
		kmem_cache* curr_cache = kmem_caches + cache_num;
		if(curr_cache -> size >= size) {
			return kmem_cache_alloc(curr_cache);
		}
	}

	return (void*) NULL;
}

void free(uint32_t* loc) {	
	// Doing nothing rn lol
	//
	*loc = 0x0;
}



void init_heap() {
	map_page(heap_start, (uint32_t)allocate_page(), P | RW);
	heap_end += 0x1000;

	init_kmem_caches();
	(void*) fake_kmalloc(8);
}

		



