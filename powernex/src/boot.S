.extern kmain
.extern start
.extern end

.set STACK_SIZE,        0x4000

.set MULTIBOOT_MAGIC,   0x1badb002
.set MULTIBOOT_ALIGN,    1<<0
.set MULTIBOOT_MEMINFO,  1<<1
.set MULTIBOOT_VIDINFO,  1<<2
.equ MULTIBOOT_FLAGS,  MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_VIDINFO
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

.section .multiboot
.align 4
multiboot_header:
  .long MULTIBOOT_MAGIC
  .long MULTIBOOT_FLAGS
  .long MULTIBOOT_CHECKSUM
  .long multiboot_header

	.long code
	.long bss
	.long end
	.long start

	.long 0
	.long 800
	.long 600
	.long 32
	//http://wiki.osdev.org/GUI
.section .text
.global start
start:
  cli

  mov $stack + STACK_SIZE, %esp
	mov $0, %ebp

  push %ebx
  push %eax

  call kmain

halt:
 // cli
 // hlt
  jmp halt

.section .bss
.align 16
stack:
        .space STACK_SIZE

