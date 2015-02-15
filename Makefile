MODULES := powernex

#PREFIX	:= cc/Linux/bin/x86_64-pc-elf-
PREFIX	:= cc/Linux/bin/i686-pc-elf-
AR	:= $(PREFIX)ar
AS	:= $(PREFIX)as
CC	:= $(PREFIX)gcc
LD	:= $(PREFIX)ld


ARFLAGS := rcs
ASFLAGS := -march=generic32 --32 --nocompress-debug-sections -D -g --gdwarf-2
CFLAGS  := -std=c11 -O0 -g -m32 -nostdlib -fno-bounds-check -mno-red-zone -mno-mmx -mno-3dnow -Iincludes -Wall -Werror -pedantic -fsigned-bitfields -funsigned-bitfields
LDFLAGS :=
# Global ARFLAGS.

.PHONY: clean mrproper $(MODULES) bochs

# Add whatever should be your default / global target.
all: powernex.iso

run: bochs


powernex: bin/powernex.krl

iso: powernex.iso

powernex.iso: bin/powernex.krl
	cp bin/powernex.krl iso/boot
	grub-mkrescue -d /usr/lib/grub/i386-pc -o powernex.iso iso

bochs: bochsrc.txt powernex.iso
	bochs -f bochsrc.txt -q || true



###################################################################
# What follows are several templates (think "functions"), which are
# later instantiated for each registered module ($(1) being the
# module name).
###################################################################

.PHONY: FORCE
FORCE:

buildinfo.c: FORCE
	./buildinfo.sh


# Including a module's build.mk
define MK_template
include $(1)/build.mk
endef

# Setting a module's build rules for object files in <module>/obj.
define RULES_template
$(1)/obj/buildinfo.o: buildinfo.c
	@mkdir -p $$(dir $$@)
	@echo Compiling $$<
	@$$(CC) $$(CFLAGS) $$(CFLAGS_$(1)) -Iincludes/$(2) -c $$< -o $$@

$(1)/obj/%.o: $(1)/src/%.c
	@mkdir -p $$(dir $$@)
	@echo Compiling $$<
	@$$(CC) $$(CFLAGS) $$(CFLAGS_$(1)) -Iincludes/$(2) -c $$< -o $$@
$(1)/obj/%.o: $(1)/src/%.S
	@mkdir -p $$(dir $$@)
	@echo Compiling $$<
	@$$(AS) $$(ASFLAGS) $$(ASFLAGS_$(1)) -Iincludes/$(2) -c $$< -o $$@
endef

# Setting a module's build rules for executable targets.
# (Depending on its sources' object files and any libraries.)
# Also adds a module's dependency files to the global list.
define PROGRAM_template
#DEPENDENCIES := $(DEPENDENCIES) $(patsubst %,$(2)/obj/%.d,$(basename $($(1)_SOURCES)))
bin/$(1): $(2)/obj/buildinfo.o $(patsubst %,$(2)/obj/%.o,$(basename $($(1)_SOURCES))) $(foreach library,$($(1)_LIBRARIES),lib/$(library))
	@mkdir -p $$(dir $$@)
	@echo Linking $$@
	@$$(LD) $$(LDFLAGS) $$(LDFLAGS_$(2)) $$^ -o $$@
endef

# Setting a module's build rules for archive targets.
# (Depending on its sources' object files.)
define ARCHIVE_template
#DEPENDENCIES := $(DEPENDENCIES) $(patsubst %,$(2)/obj/%.d,$(basename $($(1)_SOURCES)))
lib/$(1): $(2)/obj/git-version.o $(patsubst %,$(2)/obj/%.o,$(basename $($(1)_SOURCES)))
	@mkdir -p $$(dir $$@)
	@$$(AR) $$(ARFLAGS) $$@ $$?
endef

# Linking a module's global includes into the global include directory
# (where they will be available as <module>/filename.h).
define INCLUDE_template
ifeq ($(wildcard includes/$(1)),)
    $$(shell ln -s ../$(1)/include includes/$(1))
endif
endef

# Now, instantiating the templates for each module.
$(foreach module,$(MODULES),$(eval include $(module)/build.mk))
$(foreach module,$(MODULES),$(eval $(call RULES_template,$(module))))
$(foreach module,$(MODULES),$(eval $(foreach binary,$($(module)_PROGRAM),$(call PROGRAM_template,$(binary),$(module)))))
$(foreach module,$(MODULES),$(eval $(foreach library,$($(module)_ARCHIVE),$(call ARCHIVE_template,$(library),$(module)))))
$(foreach module,$(MODULES),$(eval $(call INCLUDE_template,$(module))))

# Include the dependency files (generated by GCC's -MMD option)
#-include $(sort $(DEPENDENCIES))

clean:
	$(RM) -rf $(foreach mod,$(MODULES),$(mod)/obj/) | true
	$(RM) includes/* bin/* lib/* | true

kclean:
	$(RM) -rf powernex/obj/ | true

#$(foreach mod,$(MODULES),$(mod)/obj/*.d)
