.global idt_setup
.global general_idt

.global irq_0 				# Timer interrupt request
.global irq_1

idt_setup:
	mov 4(%esp), %eax
	lidt (%eax)
	sti
	ret

general_idt:
	pushal 				# push GPRs onto the stack
	cld 				# C code following ABI requires DF to be clear
	call general_idt_handler
	popal 				# pop GPRs from the stack
	iret

irq_0:
	pushal
	cld
	call irq_0_handler
	popal
	iret

irq_1:
	pushal
	cld
	call irq_1_handler
	popal
	iret

