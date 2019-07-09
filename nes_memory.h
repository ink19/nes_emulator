#ifndef NES_MEMORY_H__
#define NES_MEMORY_H__

#include "nes_cpu.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern u_int8_t *get_memory(nes_cpu_t *cpu, u_int16_t addr);

#endif