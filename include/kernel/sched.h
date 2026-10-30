#ifndef SCHED_H
#define SCHED_H

/* Task scheduling */

/* State of a task */
typedef enum task_state {
	RUNNING,
	READY,
	BLOCKED,
	FINISH,
} task_state;

/* Thread Control Block */
typedef struct task_control_block_t {
	/* Stack Top. */
	uint32_t esp;

	/* CR3 -- holds page directory (Virtual Address Space). */
	uint32_t CR3;
	
	/* Pointer to next task. */
	struct task_control_block_t* next;

	/* Current state of this task. */
	task_state state;

} task_control_block;

/* Intialize our kernel boot code.
 * kmalloc and create a new task_struct. */
void init_multitasking();



#endif
