#include <stdint.h>
#include <stddef.h>

#include <arch/i386/vmm.h>

#include <mm/heap.h>
#include <mm/pmm.h>

#include <lib/kprintf.h>
#include <lib/string.h>

// #define DEBUG_HEAP_SLAB
// #define DEBUG_HEAP_PARTIAL
// #define DEBUG_HEAP_EMPTY
// #define DEBUG_HEAP_KMEMCA
// #define DEBUG_HEAP_FREE
// #define DEBUG_HEAP_KMEMCF
// #define DEBUG_HEAP_FFSLAB
// #define DEBUG_HEAP_FPSLAB
// #define DEBUG_HEAP_FREEAD

#ifdef DEBUG_HEAP_SLAB 
	#define heap_debug_slab(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_slab(...) do { } while (0)
#endif

#ifdef DEBUG_HEAP_PARTIAL 
	#define heap_debug_partial(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_partial(...) do { } while (0)
#endif

#ifdef DEBUG_HEAP_EMPTY
	#define heap_debug_empty(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_empty(...) do { } while (0)
#endif

#ifdef DEBUG_HEAP_KMEMCA
	#define heap_debug_kmemca(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_kmemca(...) do { } while (0)
#endif

#ifdef DEBUG_HEAP_FREE
	#define heap_debug_free(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_free(...) do {} while (0)
#endif 

#ifdef DEBUG_HEAP_KMEMCF
	#define heap_debug_kmemcf(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_kmemcf(...) do {} while (0)
#endif 

#ifdef DEBUG_HEAP_FFSLAB
	#define heap_debug_ffslab(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_ffslab(...) do {} while (0)
#endif 

#ifdef DEBUG_HEAP_FPSLAB
	#define heap_debug_fpslab(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_fpslab(...) do {} while (0)
#endif

#ifdef DEBUG_HEAP_FREEAD
	#define heap_debug_freead(...) kprintf(__VA_ARGS__)
#else
	#define heap_debug_freead(...) do {} while (0)
#endif

/* 11/15/2026 -- Rewriting this for a SLAB allocator.
 * I don't expect this to be super similar to Linux (obviously), but the current "bump" allocator 
 * is not sufficient.
 *
 * kmalloc acts as a wrapper for a bunch of kmem_caches, which each contain slabs.
 * As a result, existing kmalloc functionality should be maintained. 
 * */

/* Redefinition of kmem_cache for use inside kmem_slab. */
typedef struct kmem_cache_t;

/* Current number of kmem_caches that exist, for small and large caches. */
#define KMEM_CACHES		0x6
#define KMEM_CACHES_L 		0x5

/* Starting location of the Kernel Heap.
 * For now, this space is 0xC1000000 - 0xD0000000.*/
#define KERNEL_HEAP_START 	0xC1000000u
#define MAX_PAGES 		0x16800u
#define PAGE_SIZE 	 	0x1000
#define INIT_LARGE_SLABS 	0x4

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
	
	/* For tracking how many allocated and maximum memory chunks are in a slab. */
	uint16_t curr_amnt, max_amnt;

	/* Bitmap for tracking larger allocations.
	 * Originally, Bonwick's paper mentioned a kmem_bufctl object.
	 * This is only used for larger allocations and should be ignored for 
	 * allocations under 256 B */
	uint8_t alloc_bitmap;
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
	{"kmem_cache_2048", 2048},
	{"kmem_cache_4096", 4096},
	{"kmem_cache_8192", 8192},
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
	slab -> curr_amnt = 0;
	slab -> max_amnt = 0;

	/* Find the first 8 byte aligned address to begin chopping the block up at. */
	uint8_t starting_free_offset = ((sizeof(kmem_slab) + 7) / 8) * 8;
	uint32_t starting_free_addr = starting_free_offset + (uint32_t) slab_address;

	/* Set the first free block in the slab. */
	slab -> free = (uint32_t*) starting_free_addr;
	heap_debug_slab("Start: %x \n", slab -> free);

	/* Compute the final allocatable address for this slab. */
	uint32_t final_addr = (PAGE_SIZE + (uint32_t) slab_address) - (cache -> size << 1);

	/* Essentially, left of <=, we do not want to potentially overwrite the next page in 
	 * virtual memory. To the right, we know the size of a 4KB page (4KB) and where this slab starts,
	 * so don't go past that. We then increment by our required allocation size. */
	uint32_t curr_addr = starting_free_addr;
	for(;curr_addr <= final_addr; curr_addr += cache -> size) {
		/* Cast current address to a pointer. This makes more intuitive sense. 
		 * Then, we can dereference it, and put a memory location there, that is,
		 * a pointer to the next free adress. */
		uint32_t* curr_free = (uint32_t*) curr_addr;
		*curr_free = curr_addr + cache -> size;
		(slab -> max_amnt)++;
	}

	/* Allocate the final block with the value 0xDEADBEEF. */
	*((uint32_t*) curr_addr) = 0xDEADBEEF;
	(slab -> max_amnt)++;

	/* Add this slab to the end parent caches empty queue.
	 * NOTE: cache == slab -> parent_cache here! 
	 *
	 * One may also believe that only one empty could exist at a time, 
	 * however, we must consider the case where memory is freed! */
	if(!(cache -> empty)) {
		cache -> empty = slab;
		slab -> next = slab;
		slab -> prev = slab;
	} else {
		/* l_empty == last empty slab in empty queue */
		kmem_slab* l_empty = cache -> empty -> prev;
		l_empty -> next = slab;
		slab -> next = cache -> empty;
		slab -> prev = l_empty;
		cache -> empty -> prev = slab;
	}
	slab -> state = NEW;
}		

/* Similar to above, but we add a slab to the larger caches. */
static void add_slab_l(kmem_cache* cache) {
	/* Use our smaller allocator to give use a cache address. */
	kmem_slab* curr_slab = (kmem_slab*) fake_kmalloc(sizeof(kmem_slab));

	curr_slab -> free = NULL;
	curr_slab -> parent_cache = cache;
	curr_slab -> next = NULL;
	curr_slab -> prev = NULL;
	curr_slab -> curr_amnt = 0;
	curr_slab -> max_amnt = 0;
	
	/* Determine how much memory to request.
	 * TODO: Add a method that gives us */

}


/* Init all the caches. 
 * This involves requesting and mapping memory, then carving the slabs into each of these pages.
 * Each kmem_cache will intially start with only a single slab. 
 *
 * After this, if request can not be satisfied, additional pages are requested and mapped. */
static void init_kmem_caches() {
	/* Zero the bitmap. */
	init_page_bitmap();
	
	size_t cache_num = 0;
	kmem_cache* curr_cache = NULL;
	/* Interate through all kmem_caches, initializing the first slabs. */
	for(; cache_num < KMEM_CACHES; cache_num++) {
		/* Initialize the first fields of the cache. 
		 * size and cache_name are already intialized.*/
		curr_cache = kmem_caches + cache_num;
		curr_cache -> empty = NULL;
		curr_cache -> full = NULL;
		curr_cache -> partial = NULL;

		/* Initialize the first slab, which belongs to this caches empty by default. */
		add_slab(curr_cache);
	}

	/* Initialize the larger memory caches. 
	 * Each slab points to some other buffer, not a free memory location on the slab. */
	for(; cache_num < KMEM_CACHES + KMEM_CACHES_L; cache_num++){
		curr_cache = kmem_caches + cache_num;
		curr_cache -> empty = NULL;
		curr_cache -> full = NULL;
		curr_cache -> partial = NULL;

		/* Initially allocated 4 slabs of varying sizes seems "fair".
		 * Decent amount RTG, and do not have to worry about slow allocs right away. */
		for(size_t inc = 0; inc < INIT_LARGE_SLABS; inc++) {
			/* add_slab LARGE. Still adds the slab to the empty queue. */
			add_slab_l(curr_cache);
		}
	}
}

/* Scan the partial slab, looking for a free address to return. 
 * Also, consider the case where we have to add this slab to the full list. */
static void* scan_partial_slab(kmem_cache* curr_cache) {
	/* Assume we can not find something to begin with. */
	uint32_t* partial_addr = NULL;
	/* partial_s = partial slab, not to be confused with the SET of partial slabS (with capital S).*/
	kmem_slab* partial_s = curr_cache -> partial;
	if(!partial_s) {
		return (void*) NULL;
	}
	partial_addr = partial_s -> free;

	/* In the case this was the last address, 
	 * remove from partial and add this to the full queue. */
	if(*partial_addr == 0xDEADBEEF) {
		/* Check to see if this is the only element in partial queue.
		 * We do not allocate a new page/slab here, that is done on demand. */
		if(partial_s -> next == partial_s) {
			heap_debug_partial("PARTIAL NOW EMPTY AFTER REMOVING\n");
			curr_cache -> partial = NULL;
		} else {
			/* Remove normally from partial cache. It is important to note,
			 * the first element in the partial cache always has a free slot, so 
			 * removal only ever comes from front. */
			heap_debug_partial("PARTIAL STILL NOT EMPTY AFTER REMOVING\n");
			curr_cache -> partial = partial_s -> next;
			curr_cache -> partial -> prev = partial_s -> prev;
			partial_s -> prev -> next = curr_cache -> partial;
		}

		/* Check if full is empty. */
		if(!(curr_cache -> full)) {
			heap_debug_partial("FULL IS EMPTY, ADDING\n");
			curr_cache -> full = partial_s;
			partial_s -> next = partial_s;
			partial_s -> prev = partial_s;
		} else {
			/* If not the first element, insert at the end
		 	* l_partial = last full. */
			heap_debug_partial("FULL IS NOT EMPTY, ADDING");
			kmem_slab* l_full = curr_cache -> full -> prev;
			l_full -> next = partial_s;
			curr_cache -> full -> prev = partial_s;
			partial_s -> next =  curr_cache -> full;
			partial_s -> prev = l_full;
		}
	}

	/* After that case has been checked, just set this slabs free to the address 
	 * that was contained in the allocated block. */
	partial_s -> free = (uint32_t*) *partial_addr;	
	(partial_s -> curr_amnt)++;

	heap_debug_partial("INSIDE PARTIAL, RETURNED ADDRESS: %x\n", partial_addr);
	heap_debug_partial("NEXT ADDRESS: %x\n", *partial_addr);

	return (void*) partial_addr;
}	

/* Take an address from an existing empty slab and move it to the partial queue, or 
 * create a new empty slab (also moving it). */
static void* scan_empty_slab(kmem_cache* curr_cache) {
	/* As far as I know right now, for small allocations, we should not have
	 * to worry about any weird issues in selecting a memory address from a empty slab.
	 * Things may get interesting after massive slabs... */

	/* If no empties exist, create one. */
	if(!(curr_cache -> empty)) {	
		kprintf("ALLOC EMPTY \n");
		add_slab(curr_cache);
	}

	/* TODO: We always assume we get memory here... that is obviously not the case. */
	kmem_slab* curr_slab = curr_cache -> empty;
	uint32_t* empty_addr = curr_slab -> free;
	
	/* Remove this slab from empty, and add it to the partial queue. */
	if(curr_slab -> next == curr_slab) {
		/* Only curr_slab belongs in empty queue, set it to NULL. */
		heap_debug_empty("EMPTY EMPTY AFTER DELETING\n");
		curr_cache -> empty = NULL;
	} else {
		/* Remove curr_slab from front of the empty queue. */
		heap_debug_empty("EMPTY EMPTY FULL AFTER DELETING\n");
		curr_cache -> empty = curr_slab -> next;
		curr_slab -> prev -> next = curr_slab -> next;
		curr_slab -> next -> prev = curr_slab -> prev;
	}

	if(!(curr_cache -> partial)) {
		/* Partial is empty case. */
		heap_debug_empty("PARTIAL IS EMPTY, ADDING\n");
		curr_cache -> partial = curr_slab;
		curr_slab -> next = curr_slab;
		curr_slab -> prev = curr_slab;
	} else {
		/* Add to end of partial queue. 
		 * l_partial == current last partial slab. */
		heap_debug_empty("PARTIAL IS NOT EMPTY, ADDING\n");
		kmem_slab* l_partial = curr_cache -> partial -> prev;
		l_partial -> next = curr_slab;
		curr_slab -> prev = l_partial;
		curr_slab -> next = curr_cache -> partial;
		curr_cache -> partial -> prev = curr_slab;
	}
	/* Also set the slabs new free to the next free. */
	curr_slab -> free = (uint32_t*) *empty_addr;
	(curr_slab -> curr_amnt)++;

	heap_debug_empty("INSIDE EMPTY, RETURNED ADDRESS: %x\n", empty_addr);
	heap_debug_empty("NEXT ADDRESS: %x\n", *empty_addr);
	
	/* Set the state of this slab and return mem addr. */
	curr_slab -> state = IN_USE;
	return (void*) empty_addr;
}

/* Return the first memory segment from the current kmem_cache. */
static void* kmem_cache_alloc(kmem_cache* curr_cache) {
	/* If the current cache has a partial queue, prefer that.
	 * CPU caches will treat us a little better. */
	uint32_t* free_addr;
	if(free_addr = (uint32_t*) scan_partial_slab(curr_cache)) {
		heap_debug_kmemca("USE PARTIAL ADDR: %x\n", free_addr); 
		return (void*) free_addr;
	}

	/* If we can not get a partial address, we have to take from a new slab.
	 * I say take and not create, as it may be the case that a new slab already exist,
	 * but nothing has been allocated from it just yet. */
	if(free_addr = (uint32_t*) scan_empty_slab(curr_cache)) {
		heap_debug_kmemca("USE EMPTY ADDR: %x\n", free_addr);
		return (void*) free_addr;
	}

	/* Nothing found, force the calling func to deal with it. */
	return (void*) NULL;
}

/* Free a memory address, but do not touch the slab's state. */
static void free_slab(kmem_slab* curr_slab, void* free_addr) {
	/* Obtain the memory address at the start of the free list. */
	uint32_t* first_free = curr_slab -> free;	
	heap_debug_freead("FREEING ADDRESS: %x\n", (uint32_t*) free_addr);

	/* Cast free address into a uint32_t*, storing this newly caculated address. */
	uint32_t* free_addr_nvoid = (uint32_t*) free_addr;
	*free_addr_nvoid = (uint32_t) first_free;
	
	/* Set the first free address to the newly freed address in slab. */
	curr_slab -> free = free_addr;
	(curr_slab -> curr_amnt)--; 
}

/* Free a memory address and add this slab to the caches partial queue. */
static void free_full_slab(kmem_cache* curr_cache, kmem_slab* curr_slab, void* free_addr) {
	/* Remove from the slab from the current full queue. */

	/* This check has been performed a few times by this point. 
	 * First condition checks checks to see if we are only slab in full queue. */
	if(curr_slab -> next == curr_slab) {
		curr_cache -> full = NULL;
	} else {
		/* For allocation involving empty -> partial or partial->full, 
		 * we are guaranteed to be working at the head of the queue. 
		 * This logic does not apply for the full queues transition to 
		 * partial, so additional check it required */
		if(curr_slab == (curr_cache -> full)) {
			curr_cache -> full = curr_slab -> next;
		}
		/* Remove the current slab from where ever it may be in the full queue. */
		curr_slab -> next -> prev = curr_slab -> prev;
		curr_slab -> prev -> next = curr_slab -> next;
	}
	heap_debug_ffslab("FREEING FULL -> PARTIAL\n");
	/* Add this slab back into the partial queue, first checking if partial is empty. */
	if(curr_cache -> partial == NULL) {
		curr_cache -> partial = curr_slab;
		curr_slab -> next = curr_slab;
		curr_slab -> prev = curr_slab;
	} else {
		/* Add this slab to end of partial queue. */
		kmem_slab* l_partial = curr_cache -> partial -> prev;
		curr_cache -> partial -> prev = curr_slab;
		curr_slab -> next = curr_cache -> partial;
		curr_slab -> prev = l_partial;
		l_partial -> next = curr_slab;
	}

	/* Free this memory in the slab. */
	free_slab(curr_slab, free_addr);
}

static void free_partial_slab(kmem_cache* curr_cache, kmem_slab* curr_slab, void* free_addr) {
	/* Current slab is front of queue, so do nothing. */
	if(curr_slab -> next == curr_slab) {
		curr_cache -> partial = NULL;
	} else {
		/* As above, multiple partial queues may exist at once after a
		 * freeing operation, so be careful where you remove from. */
		if(curr_slab == (curr_cache -> partial)) {
			curr_cache -> partial = curr_slab -> next;
		}
		/* Remove the current slab from where ever it may be in the full queue. */
		curr_slab -> next -> prev = curr_slab -> prev;
		curr_slab -> prev -> next = curr_slab -> next;
	}
	heap_debug_fpslab("FREEING PARTIAL -> EMPTY\n");
	/* Add this slab back into the empty queue, first checking if empty is empty. */
	if(curr_cache -> empty == NULL) {
		curr_cache -> empty = curr_slab;
		curr_slab -> next = curr_slab;
		curr_slab -> prev = curr_slab;
	} else {
		/* Add this slab to end of empty queue. */
		kmem_slab* l_empty = curr_cache -> empty -> prev;
		curr_cache -> empty -> prev = curr_slab;
		curr_slab -> next = curr_cache -> empty;
		curr_slab -> prev = l_empty;
		l_empty -> next = curr_slab;
	}

	/* Free this memory in the slab. */
	free_slab(curr_slab, free_addr);
	
	/* Decrement the current amount of slabs and set this slab to USED. */
	curr_slab -> state = USED;
}

/* Free a memory address from a "small" slab (512 bytes MAX). 
 * This will not be used for larger allocations where an additional slab 
 * is used to point at the buffer. */
static void kmem_cache_free(kmem_cache* curr_cache, void* free_addr) {
	/* Check if the cache is full. If it is, we need to remove it from the 
	 * full cache and add it to the partial cache. */
	uint32_t f_slab = ((uint32_t)free_addr) & ~(0xFFF);	
	kmem_slab* curr_slab = (kmem_slab*) f_slab;
	volatile uint16_t curr_alloc = curr_slab -> curr_amnt;
	volatile uint16_t max_alloc = curr_slab -> max_amnt;
	heap_debug_free("CURR_ALLOC: %d, MAX_ALLOC %d\n", curr_alloc, max_alloc);
	/* If condition is true, the slab is obviously full */
	if(curr_alloc == max_alloc) {
		heap_debug_free("SLAB IS FULL\n");
		free_full_slab(curr_cache, curr_slab, free_addr);
		return;
	}
	
	/* If we only have one slab, then we become empty. 
	 * Else just free this address normally. */
	if(curr_alloc == 1) {
		free_partial_slab(curr_cache, curr_slab, free_addr);
	} else {
		free_slab(curr_slab, free_addr);
	}
	heap_debug_free("SLAB IS PARTIAL\n");
	/* Rely on callers to set additional meta data, i.e. curr_amnt and slab_state */
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
	for(size_t cache_num = 0; cache_num < KMEM_CACHES + KMEM_CACHES_L; cache_num++) {
		kmem_cache* curr_cache = kmem_caches + cache_num;
		if(curr_cache -> size >= size) {
			return kmem_cache_alloc(curr_cache);
		}
	}

	return (void*) NULL;
}

/* Fake free function, eventually replacig the real free(). */
void fake_free(void* free_addr) {
	/* TODO: As of right now, this function almost just acts as a wrapper for
	 * kmem_cache_free(). However, it will be more involved when larger allocations
	 * are made avialable. */

	/* Obtain the starting slab address. */
	uint32_t free_slab = ((uint32_t)free_addr) & ~(0xFFF);		
	heap_debug_free("FREEING ADDRESS: %x AT SLAB: %x\n", (uint32_t*)free_addr, free_slab);
	
	/* Do the work to free this address. */
	kmem_cache* parent_cache = ((kmem_slab*) free_slab) -> parent_cache;
	kmem_cache_free(parent_cache, free_addr);
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
	/*
	uint32_t* ptr1 = (uint32_t*) fake_kmalloc(1024);
	uint32_t* ptr2 = (uint32_t*) fake_kmalloc(1024);
	uint32_t* ptr3 = (uint32_t*) fake_kmalloc(1024);
	fake_free(ptr1);
	fake_free(ptr2);
	fake_free(ptr3);
	ptr1 = (uint32_t*) fake_kmalloc(1024);
	fake_free(ptr1);
	*/

}

		



