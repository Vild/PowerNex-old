#ifndef THREAD_H_
#define THREAD_H_

#include <powernex/powernex.h>

typedef struct thread {
	uint32_t esp, ebp, ebx, esi, edi;
	uint32_t id;
	bool dead; //Used only if the thread is currently running
	struct thread * next;
} thread_t;

typedef int (*thread_func_f)(void *arg);

extern thread_t * thread_current;

void thread_init();
thread_t * thread_create(thread_func_f fn, void * arg, uint32_t * stack);
void thread_start(thread_t * thread);
void thread_stop(thread_t * thread);

bool thread_isRunning(thread_t * thread);

void thread_next();

void thread_sleep(uint32_t time);

#endif
