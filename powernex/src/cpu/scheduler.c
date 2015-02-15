#include <powernex/cpu/scheduler.h>
#include <powernex/mem/heap.h>

static thread_list_t * ready_queue = NULL;
thread_list_t * current_thread = NULL;

void scheduler_init(thread_t * initial) {
	current_thread = (thread_list_t *)kmalloc(sizeof(thread_list_t));
	current_thread->thread = initial;
	current_thread->next = NULL;
	ready_queue = NULL;
}

void scheduler_isReady(thread_t * t) {
	thread_list_t * item = (thread_list_t *)kmalloc(sizeof(thread_list_t));
	item->thread = t;
	item->next = NULL;
	
	if (!ready_queue)
		ready_queue = item;
	else {
		thread_list_t * iterator = ready_queue;
		while (iterator->next)
			iterator = iterator->next;

		iterator->next = item;
	}
}

void scheduler_isNotReady(thread_t * t) {
	thread_list_t * iterator = ready_queue;

	if (!iterator)
		return;
	
	if (iterator->thread == t) {
		ready_queue = iterator->next;
		kfree(iterator);
		return;
	}
	
	while (iterator->next) {
		if (iterator->next->thread == t) {
			thread_list_t * tmp = iterator->next;
			
			iterator->next = tmp->next;
			kfree(tmp);
			return;
		}
		iterator = iterator->next;
	}
}

void scheduler_schedule() {
	if (!ready_queue)
		return;

	thread_list_t * iterator = ready_queue;
	while (iterator->next)
		iterator = iterator->next;

	iterator->next = current_thread;
	current_thread->next = NULL;
	
	thread_list_t * newThread = ready_queue;
	ready_queue = ready_queue->next;
	thread_switch(newThread);
}
