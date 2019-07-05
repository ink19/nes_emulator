#include "nes_cpu.h"

static u_int8_t read_opcode_byte(nes_cpu_t *cpu) {
    return cpu->memory[cpu->mregister.PC++];
}

static u_int8_t addr_imm(nes_cpu_t *cpu) {
    return read_opcode_byte(cpu);
}

static u_int8_t addr_zp(nes_cpu_t *cpu) {
    return cpu->memory[read_opcode_byte(cpu)];
}

static u_int8_t addr_zpx(nes_cpu_t *cpu) {
    return cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.X) % 0xff];
}

static u_int8_t addr_indx(nes_cpu_t *cpu) {
    return cpu->memory[cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.X) % 0xff]];
}

static u_int8_t addr_indy(nes_cpu_t *cpu) {
    return cpu->memory[cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.Y) % 0xff]];
}

static u_int8_t addr_absx(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_opcode_byte(cpu);
    addr <<= 8;
    addr |= read_opcode_byte(cpu);
    addr += cpu->mregister.X;
    return cpu->memory[addr];
}

static u_int8_t addr_absy(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_opcode_byte(cpu);
    addr <<= 8;
    addr |= read_opcode_byte(cpu);
    addr += cpu->mregister.Y;
    return cpu->memory[addr];
}

static u_int8_t addr_abs(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_opcode_byte(cpu);
    addr <<= 8;
    addr |= read_opcode_byte(cpu);
    return cpu->memory[addr];
}

static u_int8_t *addr_zp_write(nes_cpu_t *cpu) {
    return &(cpu->memory[read_opcode_byte(cpu)]);
}

static u_int8_t *addr_zpx_write(nes_cpu_t *cpu) {
    return &(cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.X) % 0xff]);
}

static u_int8_t *addr_abs_write(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_opcode_byte(cpu);
    addr <<= 8;
    addr |= read_opcode_byte(cpu);
    return &(cpu->memory[addr]);
}

static u_int8_t *addr_absx_write(nes_cpu_t *cpu) {
    u_int16_t addr;
    addr = read_opcode_byte(cpu);
    addr <<= 8;
    addr |= read_opcode_byte(cpu);
    addr += cpu->mregister.X;
    return &(cpu->memory[addr]);
}

static u_int8_t get_oper(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_imm, addr_zp, addr_zpx, 
        addr_indx, addr_indy, addr_absx,
        addr_absy, addr_abs
    };
    return (*(get_oper_fun[opcode & 0x07]))(cpu);
}

static void setZ(nes_cpu_t *cpu, u_int8_t data) {
    cpu->flags.Z = (data == 0);
}

static void setN(nes_cpu_t *cpu, u_int8_t data) {
    cpu->flags.N = (data & 0x80 == 0x80);
}

static void setZN(nes_cpu_t *cpu, u_int8_t data) {
    setZ(cpu, data);
    setN(cpu, data);
}

//按位与
static void cpu_logic_and(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A &= get_oper(cpu, opcode);
    setZN(cpu, cpu->mregister.A);
}

//按位异或
static void cpu_logic_eor(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A ^= get_oper(cpu, opcode);
    setZN(cpu, cpu->mregister.A);
}

//按位或
static void cpu_logic_ora(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A |= get_oper(cpu, opcode);
    setZN(cpu, cpu->mregister.A);
}

//位测试
static void cpu_logic_bit(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t oper = get_oper(cpu, opcode);
    setZ(cpu, cpu->mregister.A & oper);
    cpu->flags.N = oper >> 7;
    cpu->flags.V = (oper >> 6) & 0x01;
}

//cpu的逻辑操作
static void cpu_logic(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_logic_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_logic_and,
        cpu_logic_eor,
        cpu_logic_ora,
        cpu_logic_bit
    };

    (*(cpu_logic_funs[(opcode & 0x18) >> 3]))(cpu, opcode);
}

//左移
static void cpu_shift_asl(nes_cpu_t *cpu, u_int8_t opcode) {

}

//右移
static void cpu_shift_lsr(nes_cpu_t *cpu, u_int8_t opcode) {
    
}

//循环左移
static void cpu_shift_rol(nes_cpu_t *cpu, u_int8_t opcode) {
    
}

//循环右移
static void cpu_shift_ror(nes_cpu_t *cpu, u_int8_t opcode) {
    
}
