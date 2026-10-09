#include <stdint.h>
#include <stddef.h>

#include <arch/i386/vmm.h>

#include <mm/pmm.h>

#include <lib/kprintf.h>

/* When we acquire physical blocks of RAM, we need to ensure the processor does
 * not PF when dereferencing to set the contents of this memory. This is a known 
 * Virtual address (kvaddr) that allows us to "cheat" and always have a reference to
 * whatever physical address we may need to edit.
 *
 * As kernel alreadty jumps to 0xC0000000, this is the 1022 page: 
 * 	0xC0000000 + 1022 * 4096 = 0xC03FE000
 * */
#define kvaddr 0xC03FE000

/* Extern PD and PT declare in boot.s */
extern uint32_t boot_page_directory[1024];
extern uint32_t boot_page_table1[1024];

/* Kernels initial page directory and page table (readability) */
uint32_t* kernel_pd = (uint32_t*)&boot_page_directory;
uint32_t* kernel_pt = (uint32_t*)&boot_page_table1;




/* Lower helper functions, declared static. 
 * Interface should only contain methods to Map/Unmap a page, 
 * preferably via kmalloc (if no direct mapping is required).
 */

/* Creates a page directory entry */
static uint32_t create_pde(uint32_t addr, uint32_t flags) {
	return (addr | flags);
}

/* Creates a page table entry */
static uint32_t create_pte(uint32_t addr, uint32_t flags) {
	return (addr | flags);
}

/* Returns the address of a PDE or PTE */
static uint32_t return_addr(uint32_t table_entry) {
	return table_entry & 0xFFFFF000;
}

/* Invalidates a TLB entry based on virtual address */
static inline void invlpg(uint32_t vaddr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr) : "memory");
}

/* Maps known entry to a physical location, meaning we can modify physical
 * memory using a known translation */
static void map_known(uint32_t phy) {
	boot_page_table1[1022] = create_pte(phy, P | RW);
	invlpg(kvaddr);
}



/* Heavy lifters for VMM. See include/arch/i386/vmm.h 
 *
 */

void map_page(uint32_t v_addr, uint32_t p_addr, uint32_t flags) {
	/* Locate indexes to PD and PTs */
	uint32_t page_dir 	= v_addr / 0x400000;
	uint32_t page_table 	= (v_addr % 0x400000) / 0x1000;
	uint32_t* known_addr	= (uint32_t*) kvaddr;

	/* Obtain the PDE at that entry, check present bit */
	uint32_t pde = kernel_pd[page_dir];

	if(!(pde & 0x1)) {
		/* Creates a PDE.
		 * It is important to note, the second a PDE is created all PTEs are created.
		 * However, they are marked NOT present. As a result, the large chunks of RAM 
		 * are not allocated! */ 
		uint32_t phy_addr = (uint32_t) allocate_page();
		kernel_pd[page_dir] = create_pde(phy_addr, P | RW);

		/* Note: 1024 PTEs per PDE -- INIT as not present*/
		map_known(phy_addr);
		uint32_t* known = known_addr;

		for(size_t i = 0; i < 1024; i++) {
			*known = create_pte(0x0, NP);
			known++;
		}
	} else {
		//kprintf("PDE present\n");
	}

	/* Now, check the present bit for an existing PDE (or newly created) */
	pde = kernel_pd[page_dir];
	uint32_t page_table_addr = return_addr(pde);
	map_known(page_table_addr);

	/* Calculate the offset into the page table, representing the correct page */
	uint32_t* known = known_addr;
	known += page_table;

	if(!(*known & 0x1)) {
		/* Create a valid PTE for this memory location */
		*known = create_pte(p_addr, flags);
	} else {
		//kprintf("Page in memory\n");
	}
}

void unmap_page(uint32_t v_addr) {
	uint32_t page_dir 	= v_addr / 0x400000;
	uint32_t page_table  	= (v_addr % 0x400000) / 0x1000;
	uint32_t* known 	= (uint32_t*) kvaddr;

	/* Access the Page Directory entry, extract the phy mem address */
	uint32_t page_dir_entry = kernel_pd[page_dir];
	uint32_t pt_loc = return_addr(page_dir_entry);

	/* Map this address to known VMA, add offset, extract memory*/
	map_known(pt_loc);
	known += page_table;
	uint32_t phy_loc = return_addr(*known);

	/* Reallocate this physical mem, set this entry to not valid */
	deallocate_page(phy_loc);
	*known = create_pte(0x0, NP);

	// Add scan for full set of PTE, so we can free this 4k block 
}

void massive_map_page(uint32_t v_addr, uint32_t p_addr, uint32_t flags, uint32_t size) {
	uint32_t num_of_page = size / 0x1000;
	
	/* Iterate through, creating pages memory mapped pages */
	for(size_t page = 0; page < num_of_page; page++) {
		map_page(v_addr, p_addr, flags);
		v_addr += 0x1000;
		p_addr += 0x1000;
	}
}

uint32_t walk(uint32_t v_addr) {
	/* Walks Pages to find the correct physical address. */
	uint32_t page_dir 	= v_addr / 0x400000;
	uint32_t page_table 	= (v_addr % 0x400000) / 0x1000;
	uint32_t* known 	= (uint32_t*) kvaddr;

	/* Find the lcation of the page directory entry.
	 * Using this, extract the physical address of the page table. */
	uint32_t page_dir_entry = kernel_pd[page_dir];
	uint32_t pt_loc 	= return_addr(page_dir_entry);
	kprintf("PT_LOC: %x", pt_loc);

	/* Map this to the known address, and repeat the above process. */
	map_known(pt_loc);
	known += page_table;
	uint32_t phy_loc = return_addr(*known);
	kprintf("INSIZE: %x \n", *known);
	/* Return this physical address PLUS the physical offset. */
	return (phy_loc + (v_addr % 0x1000));
}


