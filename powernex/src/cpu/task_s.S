	.global task_switch
	.extern current_thread
	.extern task_exit

task_switch:
	mov (task_current), %eax // Loads current thread

	mov %esp,  0(%eax) // Saves regs
	mov %ebp,  4(%eax)
	mov %ebx,  8(%eax)
	mov %esi, 12(%eax)
	mov %edi, 16(%eax)

	mov 4(%esp), %eax // Loads the next thread :D, System V i386 Call convertions FTW
	mov %eax, (task_current)

	mov  0(%eax), %esp // Restores regs
	mov  4(%eax), %ebp
	mov  8(%eax), %ebx
	mov 12(%eax), %esi
	mov 16(%eax), %edi

	ret // This will jump to the new code because of the register swap
	
