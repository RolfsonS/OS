#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/heap.h>

#include <kernel/sched.h>

/* Pointers the front and back of the ready queue */
task_control_block* start = NULL;
task_control_block* end = NULL;


/* Global TCB representing the current thread. 
 * This thread is used primarily in switch_to_task. */
task_control_block* current_TCB = NULL;


/* Signature for assembly stub method. */
extern void switch_to_task(task_control_block* next_task);
extern uint32_t fetch_CR3();
extern void irq_return();
extern void disable_interrupts();
extern void enable_interrupts();

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


/* schedule() will define the general logic for task scheduling.
 * yield() gives the currently running process a chance to preempt itself.
 *
 * Self preemption is the only form of process switching right now,
 * but in the future, schedule() will be called on a timer interrupt. */
void schedule() {
	/* current_TCB == NULL implies threads have not yet been initialized.
	 * current_TCB -> next == current_TCB implies only one thread in READY queue (init task). 
	 * Then there is no need to context switch. */
	if(current_TCB == NULL || current_TCB -> next == current_TCB) {
		return;
	}
	kprintf("Switch");
	switch_to_task(current_TCB -> next);
}

static void yield() {
	schedule();
}

static void test_thread1() {
	/* Print, then switch task */
	for(size_t i = 0; i < 100; i++) {
		kprintf("1");
	}

	kprintf("Task 1 Completed \n");

	finish();
}

static void test_thread2() {

	for(size_t i = 0; i < 100; i++) {
		kprintf("2");
	}

	kprintf("Task 2 completed \n");

	finish();
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
	 * are 0, as we pop all callee saved registers.The first step is to obviously 
	 * allocate a memory region for the new stack. 
	 *
	 * I originally made two major mistakes -- stack "grows" down, 
	 * and you need to set esp to the "top" of the stack.
	 *
	 * This has been modified to accomodate for interrupts. */
	uint32_t* bottom_of_stack  = (uint32_t*) kmalloc(4096);
	uint32_t* local_sp = bottom_of_stack + 1023;
	
	/* CFLAGS set to allow interrupts again (See Intel SD Manual, pg 74).*/
	uint32_t cflags = 0x202;
	/* CS (code segment) -- Kernel Mode Code Segment */
	uint32_t cs = 0x08;
	/* "Recovering" from the interrupt take us to the correct func */
	*local_sp = cflags;
	*--local_sp = cs;
	*--local_sp = (uint32_t) func;

	/* popal expects all 8 GPRs to have value 0. */
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;
	*--local_sp = 0x0;

	/* Now we perform the operations switch_to_task expects to undo. */
	*--local_sp = (uint32_t) irq_return;
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

/* Add a thread to the ready queue. */
void add_ready(task_control_block* ready) {
	/* Be aware, we are modifying the ready queue here.
	 * A timer interrupt may hit this in a incomplete state if we do not
	 * disable interrupts. */
	ready -> state = READY;
	disable_interrupts();
	
	/* Add this to the end of the READY queue */
	end -> next = ready;
	ready -> next = start;
	ready -> prev = end;
	end = ready;

	/* Resume interrupts.
	 * Task has now been added to READY Queue. */
	enable_interrupts();
}

/* A task calls this itself, adds itself to the FINISH queue. 
 *
 * TODO: add a low priority thread that reaps finish queue. */
void finish() {
	/* Set state to FINISH, add to finish queue (in future). */
	current_TCB -> state = FINISH;

	/* FIXME: Thinking ahead to future... schedule requires -> next to exist
	 * for current TCB. If we add this task to FINISH queue... and edit next, 
	 * schedule will could potentially call a finished task.*/

	/* Disable interrupts, remove from READY queue. */
	disable_interrupts();
	current_TCB -> prev -> next = current_TCB -> next;
	current_TCB -> next -> prev = current_TCB -> prev;

	/* If current TCB is end, set previous to end. */
	if (current_TCB == end) end = current_TCB -> prev;
	enable_interrupts();
	
	/* Wait until a switch occurs. */
	for(;;);
}
void init_dummy() {
	task_control_block* t1 = (task_control_block*) kmalloc(sizeof(task_control_block));
	task_control_block* t2 = (task_control_block*) kmalloc(sizeof(task_control_block));

	t1 = init_TCB(test_thread1);
	t2 = init_TCB(test_thread2);

	add_ready(t1);
	add_ready(t2);
}


/* This function will be used to start multitasking */
void init_multitasking() {
	kprintf("Hello World\n");
	
	/* The main boot stack task will be the first kernel task.
	 * This thread is where immediate other threads are spawned from.
	 *
	 * As a result, this thread represents the first and last thread of READY queue.
	 * It always remains the first (TODO: for now...). */
	task_control_block* task_main = (task_control_block*) kmalloc(sizeof(task_control_block));
	task_main -> CR3  = fetch_CR3();
	task_main -> state = READY;
	start = task_main;
	end = task_main;
	current_TCB = start;

	/* INIT the pointers of task_main */
	task_main -> next = task_main;
	task_main -> prev = task_main;

	/* Initializing two dummy threads */
	init_dummy();
	
	kprintf("ALL DONE\n");
}



