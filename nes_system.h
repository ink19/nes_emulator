#ifndef NES_SYSTEM_H__
#define NES_SYSTEM_H__

#include "nes_cpu.h"
#include "nes_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    nes_cpu_t cpu;
    u_int8_t prg_n;
    u_int8_t chr_n;
    u_int8_t mapper;
    u_int8_t mirror_type;
    u_int8_t ram_n;
    u_int8_t video_type;
} nes_emulator_t;

int load_game(nes_emulator_t *emu, char *filename);

#endif