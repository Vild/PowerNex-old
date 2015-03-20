#ifndef POWERNEX_H_
#define POWERNEX_H_

#define UNUSED __attribute__((unused))
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <buildinfo.h>

void panic(const char * str, ...); //Located in textmode.c

#define DEBUG() kprintf("%s:%s@%i\n", __FILE__, __FUNCTION__, __LINE__); __asm__ volatile("xchgw %bx, %bx")

#define BITS(s, name) unsigned int name : s

#endif
