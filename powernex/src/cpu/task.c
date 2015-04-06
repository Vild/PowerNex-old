#include <powernex/cpu/task.h>

#include <powernex/mem/heap.h>
#include <powernex/string.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/pit.h>

static uint32_t task_counter = 1;

void task_switch();

static void task_exit();
task_t * task_current = NULL;
static task_t * queue = NULL;

void task_init() {
	task_t * thread = kmalloc(sizeof(task_t));
	memset(thread, 0, sizeof(task_t));
	thread->id = task_counter++;
	thread->dead = false;
	thread->next = NULL;

	queue = task_current = thread;
}

task_t * task_create(task_func_f fn, void * arg, uint32_t * stack) {
	task_t * thread = kmalloc(sizeof(task_t));
	memset(thread, 0, sizeof(task_t));
	thread->id = task_counter++;
	thread->dead = false;
	thread->next = NULL;
	
	*--stack = (uint32_t)arg;
	*--stack = (uint32_t)&task_exit;
	*--stack = (uint32_t)fn;
	
	thread->esp = (uint32_t)stack;
	thread->ebp = 0;
	
	return thread;
}

static void task_exit() {
	register uint32_t val __asm__("eax");

	kprintf("Thread exited with value %d\n", val);
	task_stop(task_current);
	while (true)
		task_next();
}

void task_start(task_t * thread) {
	task_t * last = queue;
	while (last->next)
		last = last->next;

	last->next = thread;
}

void task_stop(task_t * thread) {
	task_t * cur = queue;
	
	while (cur->next && cur->next != thread)
		cur = cur->next;

	if (cur->next)
		thread->dead = true;
	else
		kfree(thread);
}

bool task_isRunning(task_t * thread) {
	task_t * cur = queue;
	if (cur == thread) //Kernel is always running
		return true;

	while (cur->next && cur->next != thread)
		cur = cur->next;

	if (cur->next)
		return !thread->dead;
	else
		return false;
}

void task_next() {
	task_t * newThread = task_current->next;
	if (!newThread)
		newThread = queue;

	if (newThread->dead) {
		task_t * tmp = newThread;
		task_t * cur = queue;

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

	task_switch(newThread);
}


void task_sleep(uint32_t time) {
	uint32_t tmp = pit_tick+time;
	while (pit_tick < tmp);
}
