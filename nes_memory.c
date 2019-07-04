#include "nes_memory.h"

u_int8_t *get_memory(nes_cpu_t *cpu, u_int16_t addr) {
    if(addr < 0x0800) {
        return &(cpu->memory[addr]);
    } else if(addr < 0x2000) {
        return &(cpu->memory[addr % 0x0800]);
    } else if(addr < 0x2008) {
        return &(cpu->memory[addr]);
    } else if(addr < 0x4000) {
        return &(cpu->memory[0x2000 + (addr - 0x2000) % 8]);
    } else if(addr <= 0xffff) {
        return &(cpu->memory[addr]);
    }
}