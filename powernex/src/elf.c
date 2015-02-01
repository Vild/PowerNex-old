#include <powernex/elf.h>
#include <powernex/io/textmode.h>
#include <string.h>

elf_t kernel_elf;

void elf_init(multiboot_elf_section_header_table_t * header) {
	elf_sectionHeader_t * sh = (elf_sectionHeader_t *)(header->addr);

	uint32_t shstrtab = sh[header->shndx].addr;
	for (uint32_t i = 0; i < header->num; i++) {
		const char * name = (const char *)(shstrtab + sh[i].name);
		if (!strcmp(name, ".strtab")) {
			kernel_elf.strtab = (const char *) sh[i].addr;
			kernel_elf.strtabsz = sh[i].size;
		} else if (!strcmp(name, ".symtab")) {
			kernel_elf.symtab = (elf_symbol_t *) sh[i].addr;
			kernel_elf.symtabsz = sh[i].size;
		}
	}
}

const char * elf_lookupSymbol(uint32_t addr, elf_t * elf) {
	for (uint32_t i = 0; i < (elf->symtabsz/sizeof(elf_symbol_t)); i++) {
		if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2)
			continue;

		if (addr >= elf->symtab[i].value &&
				addr < (elf->symtab[i].value + elf->symtab[i].size))
			return (const char *)((uint32_t)elf->strtab + elf->symtab[i].name);
	}
	return NULL;
}



void elf_printStackTrace() {
  uint32_t *ebp, *eip;
  __asm__ volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = ebp + 1;
	ebp = (uint32_t*) *ebp;
  while (ebp) {
    eip = ebp + 1;
    kprintf("   [0x%x] %s\n", *eip, elf_lookupSymbol(*eip, &kernel_elf));
    ebp = (uint32_t*) *ebp;
  }
}

