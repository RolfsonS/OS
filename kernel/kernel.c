#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "idt.h"
#include "terminal.h"
#include "kprintf.h"
#include "pmm.h"
#include "multiboot.h"
#include "vmm.h"
#include "pci.h"
#include "heap.h"

// Check if the compiler things you are targeting the wrong operating system
#if defined(__linux__)
#error "You are not using a cross compiler"
#endif

// Only work for the 32-bit ix86 targets
#if !defined(__i686__)
#error  "This needs to be compiled with something else"
#endif


void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
	
	terminal_initialize();

	setup_gdt_descriptors();
	setup_idt_descriptors();

	mbd = (multiboot_info_t*)((uint32_t)mbd + 0xC0000000);
	pmm_init(mbd, magic);
	
//	for(size_t i = 0; i < 5; i++){
//		allocate_page();
//	}

//	map_page(0x0, 0x0, 0x0);
//	map_page(0x0, 0x0, 0x0);

//	map_page(0x400000, (uint32_t)allocate_page(), 0x1);
//	map_page(0x400000, 0x0, 0x0);
	
//	map_page(0x401000, (uint32_t)allocate_page(), 0x1);
//	map_page(0x401000, 0x0, 0x0);

//	map_page(0xC03FE000, 0x0, 0x0);

	init_heap();
	init_pci();

	while(1){};
}
