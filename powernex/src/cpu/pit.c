#include <powernex/cpu/pit.h>
#include <powernex/cpu/idt.h>
#include <powernex/io/port.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/scheduler.h>


uint32_t tick = 0;
static void pit_callback(UNUSED registers_t * regs) {
	tick++;
	scheduler_schedule();
	//kprintf("\rTick: %d", tick);
}

void pit_init(uint32_t frequency) {
	idt_registerHandler(IRQ(0), &pit_callback);

	uint32_t divisor = 1193180 / frequency;

	outb(0x43, 0x36);

	uint32_t l = (uint8_t)(divisor & 0xFF);
	uint32_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40, l);
	outb(0x40, h);
}
