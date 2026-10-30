/* Assembly stub for saving switching to a new task.
 * Use these for accessing fields of the task_control_block,
 * defined in sched.h. */

.set TCB_ESP, 0
.set TCB_CR3, 4

/* void switch_to_task(thread_control_block* next_thread) */
.global switch_to_task

/* uint32_t fetch_CR3() */
.global fetch_CR3

switch_to_task:
	/* Save the previous threads registers on its own stack. */
	push %ebx
	push %esi
	push %edi
	push %ebp

	/* Save the previous thread's (or current thread, we have not 
	 * switched the stack yet) stack pointer */
	movl current_TCB, %edi
	movl %esp,TCB_ESP(%edi)

	/* Update the current_task_TCB pointer... to be this one.
	 * Go up 20 bytes considering 4 GPRs and EIP (pushed onto stack via
	 * call). */
	movl 20(%esp), %esi 
	movl %esi, current_TCB

	/* Load new stack pointer and prepare for TLB comparison */
	movl TCB_ESP(%esi), %esp
	movl TCB_CR3(%esi), %eax

	/* Get the previous task address space, compare it with new one. 
	 * If different, TLB will flush. However, these should all be the 
	 * same for kernel threads right now. */
	movl %cr3, %ecx
	cmp %ecx, %eax
	/* If the same, jump */
	je .SAMEVM
	movl %eax, %cr3

.SAMEVM:
	/* We are now back on the original stack, so restore all 
	 * callee saved registers from last context switch. */
	 pop %ebp
	 pop %edi
	 pop %esi
	 pop %ebx

	 /* Calling return restores the IP. */
	 ret


fetch_CR3:
	movl %cr3, %eax
	ret
