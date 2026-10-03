.global gdt_setup


gdt_setup: 
	mov 4(%esp), %eax
	lgdt (%eax)

.gdt_flush:
	jmp $0x08, $.reload_cs

.reload_cs:
	mov $0x10, %ax
   	mov %ax, %ds
	mov %ax, %es
    	mov %ax, %fs
    	mov %ax, %gs
    	mov %ax, %ss
    	ret
	
