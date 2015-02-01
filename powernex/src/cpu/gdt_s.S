	.global gdt_flush
gdt_flush:
	mov 4(%ESP), %eax
	lgdt (%EAX)

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ljmp $0x08, $flush_ret
flush_ret:
	ret
	
