# CFLAGS for module 'powernex'
AFLAGS_powernex := -m32 -Iincludes/powernex -ffreestanding -fno-builtin -O0 -Wall -Wextra
CFLAGS_powernex := -m32 -Iincludes/powernex -ffreestanding -fno-builtin -O0 -Wall -Wextra

LDFLAGS_powernex := -T powernex/link.ld -nostdlib -nostdinc

# Executable to build in module 'powernex'
powernex_PROGRAM := powernex.krl

# Libraries that the executable depends on:
powernex.krl_LIBRARIES :=

# Sources for the executable 'powernex' (without headers)
powernex.krl_SOURCES := $(shell cd powernex/src && find -type f -name '*.c' -o -name '*.S' | sed 's/\.\///g')
