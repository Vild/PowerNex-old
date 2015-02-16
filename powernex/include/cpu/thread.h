#ifndef THREAD_H_
#define THREAD_H_

#include <powernex/powernex.h>

#ifndef SCHEDULER_H_ // Work around circular dependencies
typedef struct thread_list thread_list_t;
#endif

typedef struct thread {
	uint32_t esp, ebp, ebx, esi, edi, eflags;
	uint32_t id;
} thread_t;

typedef int (*thread_func_f)(void *arg);

thread_t * thread_init();
thread_t * thread_create(thread_func_f fn, void * arg, uint32_t * stack);
void thread_switch(thread_list_t * next);

void thread_sleep(uint32_t time);

#endif
