#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/heap.h>

#include <kernel/sched.h>

#define READY_QUEUES 32

/* Pointer to all the 32 task priority level's READY queues. 
 *
 * We also use a bitmap for indicating a task exist at that priority. */
ready_queue* ready[READY_QUEUES];
uint32_t priorities = 0x0;

/* Global TCB representing the current thread. 
 * This thread is used primarily in switch_to_task. */
task_control_block* current_TCB = NULL;


/* Signature for assembly stub method. */
extern void switch_to_task(task_control_block* next_task);
extern uint32_t fetch_CR3();
extern void irq_return();
extern void disable_interrupts();
extern void enable_interrupts();
extern uint32_t highest_bit(uint32_t);

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

/* Extract the position of the highest priority forom the ready bitmap. */
static uint32_t get_highest_priority(uint32_t priorities) {
	return highest_bit(priorities);
}

/* schedule() will define the general logic for task scheduling.
 * yield() gives the currently running process a chance to preempt itself.
 *
 * Self preemption is the only form of process switching right now,
 * but in the future, schedule() will be called on a timer interrupt. */
void schedule() {
	/* current_TCB == NULL implies threads have not yet been initialized. */
	if(current_TCB == NULL) {
		return;
	}

	/* We then check to see if a higher priority task was added to the READY queues. */
	uint32_t highest = get_highest_priority(priorities);
	if(current_TCB -> priority == highest) {

		/* In this case, there is only one task at this priority, so no switch required. */
		if(current_TCB -> next == current_TCB) return;
		
		/* If we remain in this priority, but not the only task, then switch. */
		switch_to_task(current_TCB -> next);
	} else {
		/* Switch to the higher priority task. */
		switch_to_task(ready[highest] -> head);
	}
}

static void yield() {
	schedule();
}

static void test_thread3() {

	for(size_t i = 0; i < 100; i++) {
		kprintf("3");
	}

	kprintf("Task 3 completed \n");

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

static void test_thread1() {
	/* Print, then switch task */
	for(size_t i = 0; i < 100; i++) {
		kprintf("1");
	}
	task_control_block* t3 = (task_control_block*) kmalloc(sizeof(task_control_block));

	t3 = init_TCB(test_thread3);

	add_ready(t3, 26);

	yield();

	kprintf("Task 1 Completed \n");

	finish();
}

/* Toggle a 1 in the priorities bitmap. */
static void set_priority_bm(uint8_t priority) {
	uint32_t p_val = 1u << priority;
	priorities |= p_val;
}

/* Reset a priority back to 0. */
static void reset_priority_bm(uint8_t priority) {
	uint32_t p_val = 1u << priority;
	priorities &= ~p_val; 
}

/* Add a thread to the ready queue.
 *
 * The ready uint32_t is the bitmap used for priority. This is set
 * if we add a task to a READY queue that is currently empty. 
 *
 * It is also important to note that priorities are 0 - 31. */
void add_ready(task_control_block* ready_task, uint8_t priority) {
	/* Be aware, we are modifying the ready queue here.
	 * A timer interrupt may hit this in a incomplete state if we do not
	 * disable interrupts. */
	ready_task -> state = READY;
	ready_task -> priority = priority;
	disable_interrupts();
	
	/* Pointer to the READY queue for this priority. */
	ready_queue* r_queue = ready[priority];

	/* Add this to the end of this priorities READY queue.
	 *
	 * In the case r_queue -> head == NULL, then this list is empty. 
	 * So, just add that as the head.*/
	if(r_queue -> head == NULL) {
		r_queue -> head = ready_task;
		ready_task -> next = ready_task;

		/* This being NULL also implies that the bit has not been set. */
		set_priority_bm(priority);
	} else {
		r_queue -> tail -> next = ready_task;
		ready_task -> next = r_queue -> head;
		ready_task -> prev = r_queue -> tail;
	}
	r_queue -> tail = ready_task;

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
	
	/* Pointer to the READY queue for this priority. */
	ready_queue* r_queue = ready[current_TCB -> priority];

	/* The case where current_TCB -> next == current_TCB 
	 * implies there is only one task at this priority level, so just
	 * reset the head and tail pointers to NULL and fix bitmap.
	 *
	 * The other case is a little more involved. */
	if(current_TCB -> next == current_TCB) {
		r_queue -> head = NULL;
		r_queue -> tail = NULL;
		reset_priority_bm(current_TCB -> priority);
	} else {
		current_TCB -> prev -> next = current_TCB -> next;
		current_TCB -> next -> prev = current_TCB -> prev;

		/* If current TCB is end, set previous to end. */
		if (current_TCB == r_queue -> tail) {
			r_queue -> tail = current_TCB -> prev;
		}
	}
	enable_interrupts();
	
	/* Wait until a switch occurs. */
	for(;;);
}


void init_dummy() {
	task_control_block* t1 = (task_control_block*) kmalloc(sizeof(task_control_block));
	task_control_block* t2 = (task_control_block*) kmalloc(sizeof(task_control_block));

	t1 = init_TCB(test_thread1);
	t2 = init_TCB(test_thread2);

	add_ready(t1, 4);
	add_ready(t2, 2);
}

/* Initialize the ready queues for each priority. 
 * This involves just setting the head and tail pointers to NULL. */
static void init_ready_queues() {
	for(size_t pos = 0; pos < READY_QUEUES; pos++) {
		ready[pos] = (ready_queue*) kmalloc(sizeof(ready_queue));
		ready[pos] -> head = NULL;
		ready[pos] -> tail = NULL;
	}
}

/* This function will be used to start multitasking */
void init_multitasking() {
	kprintf("Hello World\n");

	/* Initialize all of the ready queues. */
	init_ready_queues();

	/* The main boot stack task will be the first kernel task.
	 * This thread is where immediate other threads are spawned from.
	 *
	 * As a result, this thread represents the first and last thread of READY queue.
	 * It always remains the first (TODO: for now...). */
	task_control_block* task_main = (task_control_block*) kmalloc(sizeof(task_control_block));
	task_main -> CR3  = fetch_CR3();
	add_ready(task_main, 0);
	current_TCB = task_main;

	/* Initializing two dummy threads */
	init_dummy();
	
	kprintf("ALL DONE\n");
}



