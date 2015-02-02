#ifndef PMM_H_
#define PMM_H_

#include <powernex/powernex.h>
#include <stdint.h>
#include <stdbool.h>

#define PMM_STACK_ADDR 0xFF000000


void pmm_init(uint32_t start);
uint32_t pmm_allocPage();
void pmm_freePage(uint32_t p);

extern bool pmm_pagingActive;

#endif
