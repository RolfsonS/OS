#ifndef VMM_H
#define VMM_H

/* (P) Present vs Not Present */
#define P		0x1
#define NP		0x0

/* (R/W) Read/Write Permissions */
#define RW 		(0x1 << 1)
#define R		0x0

/* (U/S) User/Supervisor Permissions */
#define ALL		(0x1 << 2)
#define SUP 		0x0

/* (PWT) Write through abilities of cache */
#define RT		(0x1 << 3)
#define RB 		0x0

/* (PCD) Page caching abilities */
#define C 		(0x0)
#define NC		(0x1 << 4)

/* (A) Accessed */
#define A		(0x1 << 5)
#define NA		0x0

/* (D) Dirty */
#define D		(0x1 << 6)
#define ND 		0x0

/* (PS) Page Sizes --- Only for Directory. PS1 for 4 MB, PS0 for 4K PT. */
#define PS1 		(0x1 << 7)
#define PS0		0x0

/* (G) Global - Do not Invalidate TLB entries */
#define GL 		(0x1 << 8)
#define NGL 		0x0

/* (PAT -- PDE) Allows for further caching combinations */
#define PAT_PDE 	(0x1 << 12)
#define NPAT_PDE 	0x0

/* (PAT -- PTE) Allows for further caching combinations */
#define PAT_PTE 	(0x1 << 7)
#define NPAT_PTE 	0x0



/* Creates a page directory entry in a page directory.
 *
 * Param: uint32_t addr, The physical address of the Page Table.
 * Param: uint32_t flags, The flags set in the lower half of this PD entry.
 *
 * Returns: uint32_t, a created Page Directory Entry
 */
uint32_t create_pde(uint32_t addr, uint32_t flags);

/* Creates a page table entry in a page directory.
 *
 * Param: uint32_t addr, The physical address of the PHYSICAL PAGE.
 * Param: uint32_t flags, The flags set in the lower half of this PT entry.
 *
 * Returns: uint32_t, a created Page Table Entry
 */
uint32_t create_pte(uint32_t addr, uint32_t flags);

/* Returns the first 20 bits of a page directory or table entry, thus,
 * where the next "walk" is located in memory.
 *
 * Param: uint32_t table_entry, the full table entry
 *
 * Returns: uint32_t, physical address of next entry in walk
 */
uint32_t return_addr(uint32_t table_entry);

/* Maps a physical address to a known virtual address.
 * Translation is known. Only change entry of the final PTE
 *
 * Param: uint32_t p_addr, the physical address to map
 */
void map_known(uint32_t p_addr);

/* Maps a virtual address to a physical address ("Allocates a page"). 
 *
 * Param: uint32_t v_addr, the virtual address to map
 * Param: uint32_t p_addr, the physically allocated memory
 * Param: uint32_t flags, the flags to apply to PTE
 */
void map_page(uint32_t v_addr, uint32_t p_addr, uint32_t flags);

/* Unmap a virtual address (Must be 4k aligned)
 *
 * Param: uint32_t v_addr, the virtual address to unmap
 */
void unmap_page(uint32_t v_addr);
#endif
