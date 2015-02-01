OUTPUT_FORMAT("elf32-i386")
ENTRY(start)
SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		code = .; _code = .; __code = .;
		*(.multiboot)
		*(.text)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .; _data = .; __data = .;
		*(.data)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
		*(.shstrtab)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		bss = .; _bss = .; __bss = .;
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

  end = .; _end = .; __end = .;
}
