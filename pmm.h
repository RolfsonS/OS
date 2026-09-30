#ifndef PMM_H
#define PMM_H

#include "multiboot.h"

/* Initializes the physical memory mapping, which is passed to us via the bootloader.
 * For more info, see https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 *
 * Param: multiboot_info_t* mbd, Upon entry to the OS, the %ebx register contains the physical address
 *        of a multiboot information data structure. This is how the boot loader communicates to the OS.
 *
 * Param: uint32_t magic, Indicates the OS was loaded in by a multiboot compliant loader.
 */ 
void pmm_init(multiboot_info_t* mbd, uint32_t magic);

/* Sets the entire bit map to all zeros.
 */
void zero_bitmap();

/* Scan through memory, intialize the contents of the bitmap.
 *
 * Param: multiboot_info_t* mbd, Upon entry to the OS, the %ebx register contains the physical address
 *        of a multiboot information data structure. This is how the boot loader communicates to the OS.
 */
void init_bitmap(multiboot_info_t* mbd);

/* Allocates a 1 to the corresponding bitmap entry, indicating the page is free.
 * 
 * Param: uint32_t page, the location in memory to mark as free
 */
void* free_pmm(uint32_t page);


/* Allocates a 0 to the corresponding bitmap entry, indicating the page is reserved.
 *
 * Param: uint32_t page, the location in memory to mark as take
 */
void* allocate_pmm(uint32_t page);

/* Prints the represented byte at a memory location.
 *
 * Param: uint32_t memory_loc, the location in memory to print
 */
void print_byte(uint32_t memory_loc);

/* Used to find the start of the nearest page.
 *
 * Param: uint32_t curr_addr, the curr_addr, used to round up
 */
uint32_t find_nearest_page(uint32_t curr_addr);

/* Allocates a page */
void* allocate_page(void);

/* Deallocates a page, acts as a wrapper for free_pmm.
 *
 * Param: uint32_t addr, the location of which to deallocate.
 */
void deallocate_page(uint32_t addr);

/* Reserves the memory the kernel sits in, until paging is activated. */
void reserve_kernel();
#endif
