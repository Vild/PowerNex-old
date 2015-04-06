#ifndef TASK_H_
#define TASK_H_

#include <powernex/powernex.h>

typedef struct thread {
	uint32_t esp, ebp, ebx, esi, edi;
	uint32_t id;
	bool dead; //Used only if the thread is currently running
	struct thread * next;
} task_t;

typedef int (*task_func_f)(void *arg);

extern task_t * task_current;

void task_init();
task_t * task_create(task_func_f fn, void * arg, uint32_t * stack);
void task_start(task_t * thread);
void task_stop(task_t * thread);

bool task_isRunning(task_t * thread);

void task_next();

void task_sleep(uint32_t time);

#endif
