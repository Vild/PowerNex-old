#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <powernex/powernex.h>

#ifndef THREAD_H_ // Work around circular dependencies
typedef struct thread thread_t;
#endif

typedef struct thread_list {
	struct thread * thread;
	struct thread_list * next;
} thread_list_t;

void scheduler_init(thread_t * initial);
void scheduler_isReady(thread_t * t);
void scheduler_isNotReady(thread_t * t);
void scheduler_schedule();

#include <powernex/cpu/thread.h>

#endif
