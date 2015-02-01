#include <powernex/cpu/idt.h>
#include <powernex/io/textmode.h>

#include <string.h>

extern void idt_flush(idt_ptr_t * ptr);

static void idt_setGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;
idt_interruptHandler_t interruptHandlers[256];

void idt_init() {
	memset(&interruptHandlers, 0, sizeof(idt_interruptHandler_t) * 256);

	idt_ptr.limit = sizeof(idt_entry_t)*256 -1;
	idt_ptr.base = (uint32_t) &idt_entries;

	memset(idt_entries, 0, sizeof(idt_entry_t) * 255);

  idt_setGate( 0, (uint32_t)isr0 , 0x08, 0x8E);
  idt_setGate( 1, (uint32_t)isr1 , 0x08, 0x8E);
  idt_setGate( 2, (uint32_t)isr2 , 0x08, 0x8E);
  idt_setGate( 3, (uint32_t)isr3 , 0x08, 0x8E);
  idt_setGate( 4, (uint32_t)isr4 , 0x08, 0x8E);
  idt_setGate( 5, (uint32_t)isr5 , 0x08, 0x8E);
  idt_setGate( 6, (uint32_t)isr6 , 0x08, 0x8E);
  idt_setGate( 7, (uint32_t)isr7 , 0x08, 0x8E);
  idt_setGate( 8, (uint32_t)isr8 , 0x08, 0x8E);
  idt_setGate( 9, (uint32_t)isr9 , 0x08, 0x8E);
  idt_setGate(10, (uint32_t)isr10, 0x08, 0x8E);
  idt_setGate(11, (uint32_t)isr11, 0x08, 0x8E);
  idt_setGate(12, (uint32_t)isr12, 0x08, 0x8E);
  idt_setGate(13, (uint32_t)isr13, 0x08, 0x8E);
  idt_setGate(14, (uint32_t)isr14, 0x08, 0x8E);
  idt_setGate(15, (uint32_t)isr15, 0x08, 0x8E);
  idt_setGate(16, (uint32_t)isr16, 0x08, 0x8E);
  idt_setGate(17, (uint32_t)isr17, 0x08, 0x8E);
  idt_setGate(18, (uint32_t)isr18, 0x08, 0x8E);
  idt_setGate(19, (uint32_t)isr19, 0x08, 0x8E);
  idt_setGate(20, (uint32_t)isr20, 0x08, 0x8E);
  idt_setGate(21, (uint32_t)isr21, 0x08, 0x8E);
  idt_setGate(22, (uint32_t)isr22, 0x08, 0x8E);
  idt_setGate(23, (uint32_t)isr23, 0x08, 0x8E);
  idt_setGate(24, (uint32_t)isr24, 0x08, 0x8E);
  idt_setGate(25, (uint32_t)isr25, 0x08, 0x8E);
  idt_setGate(26, (uint32_t)isr26, 0x08, 0x8E);
  idt_setGate(27, (uint32_t)isr27, 0x08, 0x8E);
  idt_setGate(28, (uint32_t)isr28, 0x08, 0x8E);
  idt_setGate(29, (uint32_t)isr29, 0x08, 0x8E);
  idt_setGate(30, (uint32_t)isr30, 0x08, 0x8E);
  idt_setGate(31, (uint32_t)isr31, 0x08, 0x8E);

	idt_flush(&idt_ptr);
}

void idt_registerHandler(uint8_t n, idt_interruptHandler_t handle) {
	interruptHandlers[n] = handle;
}

static void idt_setGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt_entries[num].base_lo = base & 0xFFFF;
  idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

  idt_entries[num].sel     = sel;
  idt_entries[num].always0 = 0;
  // We must uncomment the OR below when we get to using user-mode.
  // It sets the interrupt gate's privilege level to 3.
  idt_entries[num].flags   = flags /* | 0x60 */;
}

void idt_handler(registers_t * regs) {
	if (interruptHandlers[regs->int_no])
		interruptHandlers[regs->int_no](regs);
	else
		kprintf("Unhandled interrupt: %d\n", regs->int_no);
}
