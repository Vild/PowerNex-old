	.global thread_switch
	.extern current_thread
	.extern thread_exit

thread_switch:
	mov (current_thread), %eax // Loads current thread
	mov (%eax), %eax

	mov %esp,  0(%eax) // Saves regs
	mov %ebp,  4(%eax)
	mov %ebx,  8(%eax)
	mov %esi, 12(%eax)
	mov %edi, 16(%eax)

	pushf
	pop %ecx
	mov %ecx, 20(%eax) // Saves EFlags

	mov 4(%esp), %eax // Loads the next thread :D, System V i386 Call convertions FTW
	mov %eax, (current_thread)
	mov (%eax), %eax

	mov  0(%eax), %esp // Restores regs
	mov  4(%eax), %ebp
	mov  8(%eax), %ebx
	mov 12(%eax), %esi
	mov 16(%eax), %edi

	mov 20(%eax), %eax // Restores EFlags
	push %eax
	popf

	ret // This will jump to the new code because of the register swap
	
