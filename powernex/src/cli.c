#include <powernex/cli.h>
#include <powernex/io/port.h>
#include <powernex/io/textmode.h>
#include <powernex/io/keyboard.h>
#include <powernex/mem/heap.h>
#include <powernex/string.h>

static char * HOSTNAME = "powernex";

static void welcome();
static char * readline(int size, char echochar);
static void login();
static int cli_run();

static char * user = NULL;
static bool logout = false;

void cli_start() {
	while(true) {
		welcome();
		kputc('\n');
		login();
		kputc('\n');
		logout = false;
		while (!logout) {
			kprintf("%s@%s# ", user, HOSTNAME);
			char * line = readline(128, 0);
			kputcolor(makecolor(COLOR_WHITE, COLOR_BLACK));
			int ret = cli_run(line);
			kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
			if (ret)
				kprintf("Command returned: %d\n", ret);
			kputcolor(DEFAULT_COLOR);
			kfree(line);
		}

		textmode_clear();
	}
}

extern bool vbe_text_mode;
extern int vbe_pattern;

static int cli_run(char * line) {

	kprintf("Line is: '%s'\n", line);
	
	if (!strncmp(line, "logout", 6) || !strncmp(line, "exit", 4))
		logout = true;
	else if (!strncmp(line, "clear", 5))
		textmode_clear();
	else if (!strncmp(line, "whoami", 6))
		kprintf("%s\n", user);
	else if (!strncmp(line, "echo ", 5)) //HACK!
		kprintf("%s\n", line + 5);
	else if (!strncmp(line, "echo", 4))
		kprintf("\n");
	else if (!strncmp(line, "shutdown", 8)) {
		outw(0xB004, 0x0 | 0x2000); // Qemu / Boch
		kprintf("Shutdown failed!\n");
	} else if (!strncmp(line, "patt ", 5)) {
		vbe_text_mode = false;
		vbe_pattern = line[5] - '1';
	} else if (!strncmp(line, "1", 4))
		vbe_text_mode = true;
	else if (!strncmp(line, "help", 4))
		kprintf("Current commands are: clear, echo, exit, help, logout, whoami\n");
	else {
		kprintf("'%s' IS NOT IMPLEMENTED\n", line);
		return -1;
	}
	return 0;
}

static void welcome() {
	const char * line1 = "Welcome to PowerNex!";
	const char * line2 = "Power for the Next generation";
	const char * line3 = "Version: ";
	const char * line3_ = build_git_version;
	const char * line4 = "Created by: Dan Printzell";
	const char * line5 = "License: Mozilla Public License, version 2.0";

  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	kputc(' ');kputc(' ');kputc('|');kputc(' ');kputc(' ');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line1);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('|');kputc('-');kputc('|');kputc('-');kputc('|');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line2);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('|');kputc(' ');kputc('|');kputc(' ');kputc('|');
	kputcolor(DEFAULT_COLOR);kprintf(" %s%s", line3, line3_);kputc('\n');
	kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('|');kputc(' ');kputc(' ');kputc(' ');kputc('|');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line4);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('|');kputc('-');kputc('-');kputc('-');kputc('|');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line5);kputc('\n');
}

static void login() {
	char * pass;
	while(true) { //Login
		kprintf("PowerNex login: ");
		user = readline(64, 0);
		kprintf("Password: ");
		pass = readline(64, '*');
		bool success = (!strcmp(user, "root") && !strcmp(pass, "root"));

		//kfree(user);
		kfree(pass);

		if (success)
			break;
		else
			kfree(user);
		
		kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
		kprintf("Sorry, try again.\n\n");
		kputcolor(DEFAULT_COLOR);
	}
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
