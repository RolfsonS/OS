// declare constants for the multiboot header
.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

// Declare a multoboot header that marks the program as a kernel
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

// Multiboot standard does not define the value of the stack pointer register and it is up to the kernel to provide a stack. 
.section .stack
.align 16
stack_bottom:
.skip 16384
stack_top:

/* Initial PD and PT */
.section .bss
.align 4096
.global boot_page_directory
.global boot_page_table1

boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096

/* Initialize everything */
.section .multiboot.text, "aw"
.global _start
.type _start, @function
_start:
	// Physical address of boot page directory 1
	movl $(boot_page_directory - 0xC0000000), %edi
	movl $0x0, %esi
	movl $1023, %ecx
1:
	movl %esi, (%edi)
	addl $4, %edi
	loop 1b

	// Physical address of boot page table 1
	movl $(boot_page_table1 - 0xC0000000), %edi
	movl $0, %esi
	movl $1023, %ecx

2:
	cmpl $_kernel_start, %esi
	jl 3f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 4f

3:
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)
	addl $4096, %esi
	addl $4, %edi
	loop 2b

4:
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4
	movl $(boot_page_table1 - 0xC0000000 + 0x03), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x03), boot_page_directory - 0xC0000000 + 768 * 4

	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	lea 5f, %ecx
	jmp *%ecx
	

// Linker script specifies _start as the entry point to the kernel and the bootloader will jump to this position once the kernel has been loaded
.section .text

5: 
	movl $0, boot_page_directory + 0 

	movl %cr3, %ecx
	movl %ecx, %cr3
	
	// Set the stack
	mov $stack_top, %esp
	
	push %eax
	push %ebx

	call kernel_main

	cli
1: 	hlt
	jmp 1b



