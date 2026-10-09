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

/* Acts as a wrapper for map_page.
 * Typically used for MMIO access mappings, where calling a map_page 
 * a bunch of times would be largely impractical.
 *
 * Param: uint32_t v_addr, Virtual address to map
 * Param: uint32_t p_addr, Physical address to map
 * Param: uint32_t flags, Flags to apply to Pages
 * Param: uint32_t size, Size to allocate (must be 4k aligned)
 */
void massive_map_page(uint32_t v_addr, uint32_t p_addr, uint32_t flags, uint32_t size);

uint32_t walk(uint32_t v_addr);
#endif
