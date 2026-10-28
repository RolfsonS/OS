#ifndef SCHED_H
#define SCHED_H

/* Task scheduling */

/* Thread Control Block */
typedef struct task_control_block_t {
	/* Stack Top. */
	void* esp;

	/* CR3 -- holds page directory (Virtual Address Space). */
	void* CR3;
	
	/* Pointer to next task. */
	task_control_block* next;

	/* Current state of this task. */
	uint8_t state;
} task_control_block;

/* Intialize our kernel boot code.
 * kmalloc and create a new task_struct. */
void init_multitasking();



#endif
