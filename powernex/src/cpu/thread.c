#include <powernex/cpu/thread.h>

#include <powernex/mem/heap.h>
#include <powernex/string.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/pit.h>

static uint32_t thread_counter = 1;

void thread_switch();

static void thread_exit();
thread_t * thread_current = NULL;
static thread_t * queue = NULL;

void thread_init() {
	thread_t * thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = thread_counter++;
	thread->dead = false;
	thread->next = NULL;

	queue = thread_current = thread;
}

thread_t * thread_create(thread_func_f fn, void * arg, uint32_t * stack) {
	thread_t * thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = thread_counter++;
	thread->dead = false;
	thread->next = NULL;
	
	*--stack = (uint32_t)arg;
	*--stack = (uint32_t)&thread_exit;
	*--stack = (uint32_t)fn;
	
	thread->esp = (uint32_t)stack;
	thread->ebp = 0;
	
	return thread;
}

static void thread_exit() {
	register uint32_t val __asm__("eax");

	kprintf("Thread exited with value %d\n", val);
	thread_stop(thread_current);
	while (true)
		thread_next();
}

void thread_start(thread_t * thread) {
	thread_t * last = queue;
	while (last->next)
		last = last->next;

	last->next = thread;
}

void thread_stop(thread_t * thread) {
	thread_t * cur = queue;
	
	while (cur->next && cur->next != thread)
		cur = cur->next;

	if (cur->next)
		thread->dead = true;
	else
		kfree(thread);
}

bool thread_isRunning(thread_t * thread) {
	thread_t * cur = queue;
	if (cur == thread) //Kernel is always running
		return true;

	while (cur->next && cur->next != thread)
		cur = cur->next;

	if (cur->next)
		return !thread->dead;
	else
		return false;
}

void thread_next() {
	thread_t * newThread = thread_current->next;
	if (!newThread)
		newThread = queue;

	if (newThread->dead) {
		thread_t * tmp = newThread;
		thread_t * cur = queue;

		while (cur->next && cur->next != tmp)
			cur = cur->next;

		if (cur->next) {
			cur->next = tmp->next;
		}
		
		newThread = newThread->next;
		if (!newThread)
			newThread = queue;
		kfree(tmp);
	}

	thread_switch(newThread);
}


void thread_sleep(uint32_t time) {
	uint32_t tmp = pit_tick+time;
	while (pit_tick < tmp);
}
