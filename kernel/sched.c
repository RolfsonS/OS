#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/heap.h>

#include <kernel/sched.h>

/* Global TCB representing the current thread. 
 * This thread is used primarily in switch_to_task. */
task_control_block* current_TCB = NULL;

/* Signature for assembly stub method. */
extern void switch_to_task(task_control_block* next_task);

static void test_thread1() {
	/* Print, then switch task */
	for(size_t i = 0; i < 5; i ++) {
		kprintf("Inside Task 1\n");
		switch_to_task(current_TCB -> next);
	}
}

static void test_thread2() {

	for(size_t i = 0; i < 5; i++) {
		kprintf("Inside Task 2\n");
		switch_to_task(current_TCB -> next);
	}
}

/* Initialize a TCB */
static task_control_block* init_TCB((*func)(void)) {
	/* kmalloc a new TCB struct */
	task_control_block* tcb = (task_control_block*) kmalloc(sizeof(task_control_block));

	/* When we switch from the main thread into a different thread,
	 * we need to ensure the top of the stack contains the instruction 
	 * pointer that we want. 
	 *
	 * At intialization, also ensure that the first four 4 bytes values 
	 * are 0, as we pop callee saved registers.
	 *
	 * The first step is to obviously allocate a memory region for the new
	 * stack. */
	tcb -> esp = kmalloc(4096);
	*(uint32_t*) tcb -> esp = func;
	*(((uint32_t*) tcb -> esp) + 1) = 0x0;
	*(((uint32_t*) tcb -> esp) + 2) = 0x0;
	*(((uint32_t*) tcb -> esp) + 3) = 0x0;
	*(((uint32_t*) tcb -> esp) + 4) = 0x0;

	tcb -> CR3 = ;

	return tcb;
} 


/* This function will be used to start multitasking */
void init_multitasking() {
	kprintf("Hello World\n");
	

	task_control_block* t1 = init_TCB(test_thread1);
	task_control_block* t2 = init_TCB(test_thread2);

	
}




