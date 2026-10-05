#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/multiboot.h>
#include <arch/i386/vmm.h>

#include <drivers/terminal.h>
#include <drivers/pci.h>
#include <drivers/e1000.h>

#include <lib/kprintf.h>

#include <mm/pmm.h>
#include <mm/heap.h>

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
	

	init_heap();
	init_pci();


	while(1){};
}
