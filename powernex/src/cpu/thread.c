#include <powernex/cpu/thread.h>

#include <powernex/mem/heap.h>
#include <powernex/string.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/scheduler.h>
#include <powernex/cpu/pit.h>

static uint32_t thread_counter;

static void thread_exit();

thread_t * thread_init() {
	thread_t * thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = thread_counter++;

	return thread;
}

thread_t * thread_create(thread_func_f fn, void * arg, uint32_t * stack) {
	thread_t * thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = thread_counter++;
	DEBUG();
	*--stack = (uint32_t)arg;	DEBUG();	
	*--stack = (uint32_t)&thread_exit;	DEBUG();
	*--stack = (uint32_t)fn;	DEBUG();
	DEBUG();
	thread->esp = (uint32_t)stack;
	thread->ebp = 0;
	thread->eflags = 0x200; // Interrupts enabled, TODO: lookup real names
	scheduler_isReady(thread);
	DEBUG();
	return thread;
}

static void thread_exit() {
	register uint32_t val __asm__("eax");

	kprintf("Thread exited with value %d\n", val);
	while (true);
}

void thread_sleep(uint32_t time) {
	uint32_t tmp = pit_tick+time;
	while (pit_tick < tmp);
}
