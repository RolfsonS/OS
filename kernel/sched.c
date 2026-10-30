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
extern uint32_t fetch_CR3();

/* Based on some stack frame pointer, esp, print the first num_of_elements 
 * on the stack */
static void print_stack(uint32_t esp, uint32_t num_of_elements) {
	/* Cast stack pointer to a real pointer, easier referencing */
	uint32_t* sp = (uint32_t*) esp;

	/* Print everything, STACK GROWS DOWN -- so we look up! */
	for(size_t i = 0; i < num_of_elements; i++) {
		kprintf("%x: %x \n", sp, *sp);
		sp++;
	}
}

/* Allows the current task to preempt itself. */
void yield() {
	switch_to_task(current_TCB -> next);
}

static void test_thread1() {
	/* Print, then switch task */
	for(size_t i = 0; i < 5; i ++) {
		kprintf("Inside Task 1\n");
		yield();
	}

	kprintf("Task 1 Completed \n");
	yield();
}

static void test_thread2() {

	for(size_t i = 0; i < 5; i++) {
		kprintf("Inside Task 2\n");
		yield();
	}

	kprintf("Task 2 completed \n");
	yield();
}

/* Initialize a TCB.
 *
 * FIXME: Refactor; I think the work with pointers can be cleaned
 * up a lot.*/
static task_control_block* init_TCB(void (*func)(void)) {
	/* Cheat to 16 byte align the stack */
	uint32_t mem = (uint32_t) kmalloc(0x1);
	while ((mem & 0xF) != 0xF) mem = (uint32_t) kmalloc(0x1); 

	/* kmalloc a new TCB struct */
	task_control_block* tcb = (task_control_block*) kmalloc(sizeof(task_control_block));

	/* When we switch from the main thread into a different thread,
	 * we need to ensure the top of the stack contains the instruction 
	 * pointer that we want. 
	 *
	 * At intialization, also ensure that the first four 4 bytes values 
	 * are 0, as we pop callee saved registers.The first step is to obviously 
	 * allocate a memory region for the new stack. 
	 *
	 * I originally made two major mistakes -- stack "grows" down, 
	 * and you need to set esp to the "top" of the stack. */
	uint32_t* bottom_of_stack  = (uint32_t*) kmalloc(4096);
	uint32_t* local_sp = bottom_of_stack + 1023;
		
	*local_sp = (uint32_t) func;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;

	tcb -> esp = (uint32_t) local_sp;

	/* To create a process thread, fetching/creating the Page Directory
	 * Entry is an involved process. For kernel threads, we make the
	 * (safe) assumption that each Kthread uses the same virtual mem. 
	 *
	 * In this case, we can just load the value of the CR3 register 
	 * into this task's TCB. */
	tcb -> CR3 = fetch_CR3();

	return tcb;
} 


/* This function will be used to start multitasking */
void init_multitasking() {
	kprintf("Hello World\n");
	
	/* Create something easy for the main thread, i.e. the kernel boot
	 * code (what we are in now... FIXME later) */
	task_control_block* kthread_main = (task_control_block*) kmalloc(sizeof(task_control_block));
	kthread_main -> CR3 = fetch_CR3();

	/* Set current task to kthread_main and build out list.*/
	current_TCB = kthread_main;

	task_control_block* t1 = init_TCB(test_thread1);
	task_control_block* t2 = init_TCB(test_thread2);

	kthread_main -> next = t1;
	t1 -> next = t2;
	t2 -> next = kthread_main;

	for(size_t i = 0; i < 5; i++) {
		kprintf("Inside main thread \n");
		yield();
	}
	yield();
	kprintf("ALL DONE\n");
}



