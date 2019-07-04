#include "nes_cpu.h"

static u_int8_t read_byte(nes_cpu_t *cpu) {
    return cpu->memory[cpu->mregister.PC++];
}

static u_int8_t addr_imm(nes_cpu_t *cpu) {
    return read_byte(cpu);
}

static u_int8_t addr_zp(nes_cpu_t *cpu) {
    return cpu->memory[read_byte(cpu)];
}

static u_int8_t addr_zpx(nes_cpu_t *cpu) {
    return cpu->memory[(read_byte(cpu) + cpu->mregister.X) % 0xff];
}

static u_int8_t addr_indx(nes_cpu_t *cpu) {
    return cpu->memory[cpu->memory[(read_byte(cpu) + cpu->mregister.X) % 0xff]];
}

static u_int8_t addr_indy(nes_cpu_t *cpu) {
    return cpu->memory[cpu->memory[(read_byte(cpu) + cpu->mregister.Y) % 0xff]];
}

static u_int8_t addr_absx(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_byte(cpu);
    addr <<= 8;
    addr |= read_byte(cpu);
    addr += cpu->mregister.X;
    return cpu->memory[addr];
}

static u_int8_t addr_absy(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_byte(cpu);
    addr <<= 8;
    addr |= read_byte(cpu);
    addr += cpu->mregister.Y;
    return cpu->memory[addr];
}

static u_int8_t addr_abs(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_byte(cpu);
    addr <<= 8;
    addr |= read_byte(cpu);
    return cpu->memory[addr];
}

static void logic(nes_cpu_t *cpu, u_int8_t opcode) {
    
}