#include <powernex/powernex.h>
#include <powernex/elf.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>
#include <powernex/cpu/pit.h>
#include <powernex/cpu/task.h>
#include <powernex/io/port.h>
#include <powernex/io/keyboard.h>
#include <powernex/mem/heap.h>
#include <powernex/mem/paging.h>
#include <powernex/fs/fs.h>
#include <powernex/fs/initrd.h>
#include <powernex/string.h>
#include <stdarg.h>

static void step(const char * msg, ...);
static void setup(multiboot_info_t * multiboot);
static void welcome();
static char * readline(int size, char echochar);

static int thread1(void * arg);
static int thread2(void * arg);

static uint32_t thread1stack[0x1000];
static uint32_t thread2stack[0x1000];

int kmain(UNUSED int multiboot_magic, multiboot_info_t * multiboot) {
	setup(multiboot);
	kputc('\n');
	
	welcome();
	kputc('\n');

	char * user;
	char * pass;
	while(true) { //Login
		kprintf("PowerNex login: ");
		user = readline(64, 0);
		kprintf("Password: ");
		pass = readline(64, '*');
		bool success = (!strcmp(user, "root") && !strcmp(pass, "root"));

		kfree(user);
		kfree(pass);

		if (success)
			break;
		
		kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
		kprintf("Sorry, try again.\n\n");
		kputcolor(DEFAULT_COLOR);
	}
	
	kputc('\n');

	
	kprintf("Thread starting...\n");
	task_t * t1 = task_create(thread1, (void *)"Mr Green", thread1stack+(0x1000/sizeof(uint32_t)));
	task_t * t2 = task_create(thread2, (void *)"Mr Red", thread2stack+(0x1000/sizeof(uint32_t)));
	task_start(t1);
	task_start(t2);
	while(task_isRunning(t1) || task_isRunning(t2));

	kputcolor(DEFAULT_COLOR);
	kprintf("Thread done\n");
	while (true) {
		kprintf("root@PowerNex# ");
		while (true) {
			char c = kb_getc();
			if (c)
				kputc(c);
			if (c == '\n')
				break;
		}
		kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
		kprintf("NOT IMPLEMENTED YET\n");
		kputcolor(DEFAULT_COLOR);
	}

	
	return 0xDEADBEEF;
}


static int thread1(void * arg) {
	kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	kprintf("PID IS: %d NAME: %s\n", task_current->id, (char*)arg);
	return task_current->id;
}

static int thread2(void * arg) {
	kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
	kprintf("PID IS: %d NAME: %s\n", task_current->id, (char*)arg);
	return task_current->id;
}

static void step(const char * str, ...) {
	kputc('[');
	kputcolor(makecolor(COLOR_MAGENTA, COLOR_BLACK));
	kputc('*');
	kputcolor(DEFAULT_COLOR);
	kputc(']');
	kputc(' ');
	va_list va;
	va_start(va, str);
	kprintf_va(str, va);
	va_end(va);
	kputc('\n');
}

static void welcome() {
	const char * line1 = "Welcome to PowerNex!";
	const char * line2 = "Power for the Next generation";
	const char * line3 = "Version: ";
	const char * line3_ = build_git_version;
	const char * line4 = "Created by: Dan Printzell";
	const char * line5 = "License: Mozilla Public License, version 2.0";

	
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	kputc(' '   );kputc(' '   );kputc('\xB3');kputc(' '   );kputc(' '   );
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line1);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xC9');kputc('\xCD');kputc('\xB3');kputc('\xCD');kputc('\xBB');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line2);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xBA');kputc(' '   );kputc('\xB3');kputc(' '   );kputc('\xBA');
	kputcolor(DEFAULT_COLOR);kprintf(" %s%s", line3, line3_);kputc('\n');
	kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xBA');kputc(' '   );kputc(' '   );kputc(' '   );kputc('\xBA');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line4);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xC8');kputc('\xCD');kputc('\xCD');kputc('\xCD');kputc('\xBC');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line5);kputc('\n');
}

static void setup(multiboot_info_t * multiboot) {
	//Textmode
	textmode_clear(); // Also initalizes textmode
	
	//GDT
	step("Initializing GDT...");
	gdt_init();
	step("Initializing IDT...");
	idt_init();

	//Memory	
	step("Initializing paging with %d MB...", (multiboot->mem_lower + multiboot->mem_upper)/1024);
	paging_init(multiboot);

	//Debuging
	step("Initializing Backtrace...");
	elf_init(&(multiboot->u.elf_sec));

	//Exceptions
	step("Initializing enabling Exceptions...");
	__asm__ volatile("sti");

	//Multithreading
	step("Initializing Multithreading...");
	task_init();

	//Hardware
	step("Initializing PIT with %d HZ...", 100);
	pit_init(100/*HZ*/);

	//Register keyboard
	step("Initializing Keyboard driver...");
	kb_init();

	step("Initializing Initrd...");
	if (multiboot->mods_count == 0)
		panic("No initrd defined in boot.cfg!");
	
	uint32_t initrd_location = *((uint32_t *)multiboot->mods_addr);
  // uint32_t initrd_end = *(uint32_t *)(multiboot->mods_addr+4);
	fs_root = initrd_init(initrd_location);
}

static char * readline(int size, char echoChar) {
	char * str = kmalloc(size);
	int count = 0;
	while (true) { //Readline user
		char c = kb_getc();
		if (c) {
			if (c == '\n') {
				kputc('\n');
				break;
			} else if (c == '\b') {
				if (count > 0) {
					--count;
					str[count] = '\0';
					kputc('\b');
				}
			} else {
				if (echoChar)
					kputc(echoChar);
				else
					kputc(c);
				if (count < size - 1)
					str[count++] = c;
			}
		}
	}
  str[count++] = '\0';
  return str;
}
