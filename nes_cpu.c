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

static u_int8_t addr_indexindiret(nes_cpu_t *cpu) {
    u_int8_t *memory = cpu->memory;
    u_int8_t X = cpu->mregister.X;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = ((u_int16_t)memory[(data + X) & 0xff]) + ((u_int16_t)memory[(data + X + 1) & 0xff]) << 8;
    return memory[addr];
}

static u_int8_t addr_indiretindex(nes_cpu_t *cpu) {
    u_int8_t *memory = cpu->memory;
    u_int8_t Y = cpu->mregister.Y;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = (u_int16_t)(memory[data]) + (u_int16_t)(memory[(data + 1) & 0xff]) << 8 + Y;
    return memory[addr];
}

static u_int8_t addr_absx(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    //addr <<= 8;
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;
    addr += cpu->mregister.X;
    return cpu->memory[addr];
}

static u_int8_t addr_absy(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.Y;
    return cpu->memory[addr];
}

static u_int8_t addr_abs(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    return cpu->memory[addr];
}

static u_int8_t *addr_zp_write(nes_cpu_t *cpu) {
    return &(cpu->memory[read_opcode_byte(cpu)]);
}

static u_int8_t *addr_zpx_write(nes_cpu_t *cpu) {
    return &(cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.X) % 0xff]);
}

static u_int8_t *addr_abs_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    return &(cpu->memory[addr]);
}

static u_int8_t *addr_absx_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.X;
    return &(cpu->memory[addr]);
}

static u_int8_t *addr_absy_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.Y;
    return &(cpu->memory[addr]);
}

static u_int8_t *addr_indexindiret_write(nes_cpu_t *cpu) {
    u_int8_t *memory = cpu->memory;
    u_int8_t X = cpu->mregister.X;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = ((u_int16_t)memory[(data + X) & 0xff]) + ((u_int16_t)memory[(data + X + 1) & 0xff]) << 8;
    return &(memory[addr]);
}

static u_int8_t *addr_indiretindex_write(nes_cpu_t *cpu) {
    u_int8_t *memory = cpu->memory;
    u_int8_t Y = cpu->mregister.Y;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = (u_int16_t)(memory[data]) + (u_int16_t)(memory[(data + 1) & 0xff]) << 8 + Y;
    return &(memory[addr]);
}

static u_int8_t get_oper(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_indexindiret, addr_zp, addr_imm, addr_abs,
        addr_indiretindex, addr_zpx, addr_absy, addr_absx
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

static u_int8_t *get_operp(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *(*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_indexindiret_write, addr_zp_write, NULL, addr_abs_write,
        addr_indiretindex_write, addr_zpx_write, addr_absy_write, addr_absx_write
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
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

//加
static void cpu_math_adc(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t result = 0;
    result += cpu->mregister.A;
    result += get_oper(cpu, opcode);
    result += cpu->flags.C;
    if(result > 0xff) {
        cpu->flags.C = 1;
        cpu->flags.V = 1;
    } else {
        cpu->flags.C = 0;
        cpu->flags.V = 0;
    }

    cpu->mregister.A = result & 0xff;
    setZN(cpu, cpu->mregister.A);
}

//减法
static void cpu_math_sbc(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t result = cpu->mregister.A;
    result -= get_oper(cpu, opcode);
    result -= cpu->flags.C;
    if(result > 0xff) {
        cpu->flags.C = 1;
        cpu->flags.V = 1;
    } else {
        cpu->flags.C = 0;
        cpu->flags.V = 0;
    }

    cpu->mregister.A = result & 0xff;
    setZN(cpu, cpu->mregister.A);
}

//比较
static void cpu_flow_cmp(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t result = cpu->mregister.A;
    result -= get_oper(cpu, opcode);
    if(result > 0xff) {
        cpu->flags.C = 1;
    } else {
        cpu->flags.C = 0;
    }
    setZN(cpu, result);
}

//送累加器
static void cpu_mem_lda(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A = get_oper(cpu, opcode);
    setZN(cpu, cpu->mregister.A);
}

//存累加器
static void cpu_mem_sta(nes_cpu_t *cpu, u_int8_t opcode) {
    *get_operp(cpu, opcode) = cpu->mregister.A;
    //setZN(cpu, cpu->mregister.A);
}

//类型一 01
static void cpu_type01(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t type = opcode >> 5;
    void (*type_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_logic_ora, cpu_logic_and, cpu_logic_eor,
        cpu_math_adc, cpu_mem_sta, cpu_mem_lda,
        cpu_flow_cmp, cpu_math_sbc
    };
    (*(type_funs[type]))(cpu, opcode);
}

//空操作
static void cpu_type10_nop(nes_cpu_t *cpu, u_int8_t opcode) {

}

//存储器加1， 空操作
static void cpu_type10_inc(nes_cpu_t *cpu, u_int8_t opcode) {
    if(((opcode >> 2)& 0x07) == 0x02) {
        cpu_type10_nop(cpu, opcode);
    } else {
        u_int8_t *result = get_operp(cpu, opcode);
        ++*result;
        setZN(cpu, result);
    }
}

//存储器减一，寄存器X减一
static void cpu_type10_dec(nes_cpu_t *cpu, u_int8_t opcode) {
    if(((opcode >> 2) & 0x07) == 0x02) {
        cpu->mregister.X--;
        setZN(cpu, cpu->mregister.X);
    } else {
        u_int8_t *result = get_operp(cpu, opcode);
        --*result;
        setZN(cpu, result);
    }
}

static u_int8_t *get_operp_register_A(nes_cpu_t *cpu, u_int8_t opcode) {
    return &(cpu->mregister.A);
}

static u_int8_t *get_operp_shift(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *(*get_oper_fun[])(nes_cpu_t *cpu) = {
        get_operp_register_A, addr_zp_write, NULL, addr_abs_write,
        NULL, addr_zpx_write, NULL, addr_absx_write
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//左移
static void cpu_shift_asl(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *result = get_operp_shift(cpu, opcode);
    cpu->flags.C = (*result) >> 7;
    (*result) <<= 1;
    setZN(cpu, (*result));
}

//右移
static void cpu_shift_lsr(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *result = get_operp_shift(cpu, opcode);
    cpu->flags.C = (*result) & 0x01;
    (*result) >>= 1;
    setZN(cpu, (*result));
}

//循环左移
static void cpu_shift_rol(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *result = get_operp_shift(cpu, opcode);
    u_int8_t temp_C = cpu->flags.C;
    cpu->flags.C = (*result) >> 7;
    (*result) <<= 1;
    (*result) |= temp_C;
    setZN(cpu, (*result));
}

//循环右移
static void cpu_shift_ror(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *result = get_operp_shift(cpu, opcode);
    u_int8_t temp_C = cpu->flags.C;
    cpu->flags.C = (*result) & 0x01;
    (*result) >>= 1;
    (*result) |= temp_C << 7;
    setZN(cpu, (*result));
}

static u_int8_t addr_zpy(nes_cpu_t *cpu) {
    return &(cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.Y) % 0xff];
}

static u_int8_t get_oper_ldx(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_imm, addr_zp, NULL, addr_abs,
        NULL, addr_zpx, NULL, addr_absy
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//A -> X
static void cpu_type10_tax(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.X = cpu->mregister.A;
    setZN(cpu, cpu->mregister.X);
}

//SP -> X
static void cpu_type10_tsx(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.X = cpu->mregister.SP;
    setZN(cpu, cpu->mregister.X);
}

//加载数据到寄存器X
static void cpu_type10_ldx(nes_cpu_t *cpu, u_int8_t opcode) {
    if(((opcode >> 2) & 0x07) == 0x02) {
        cpu_type10_tax(cpu, opcode);
        //setZN(cpu, cpu->mregister.X);
    }else if(((opcode >> 2) & 0x07) == 0x06) {
        cpu_type10_tsx(cpu, opcode);
    } else {
        u_int8_t result = get_oper_ldx(cpu, opcode);
        cpu->mregister.X = result;
        setZN(cpu, result);
    }
}

//X -> A
static void cpu_type10_txa(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A = cpu->mregister.X;
    setZN(cpu, cpu->mregister.A);
}

//X -> SP
static void cpu_type10_tsx(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.SP = cpu->mregister.X;
}

static u_int8_t *addr_zpy_write(nes_cpu_t *cpu) {
    return &(cpu->memory[(read_opcode_byte(cpu) + cpu->mregister.Y) % 0xff]);
}


static u_int8_t *get_operp_stx(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *(*get_oper_fun[])(nes_cpu_t *cpu) = {
        NULL, addr_zp_write, NULL, addr_abs_write,
        NULL, addr_zpy_write, NULL, NULL
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//X -> M
static void cpu_type10_stx(nes_cpu_t *cpu, u_int8_t opcode) {
    if(((opcode >> 2) & 0x07) == 0x02) {
        cpu_type10_txa(cpu, opcode);
        //setZN(cpu, cpu->mregister.X);
    }else if(((opcode >> 2) & 0x07) == 0x06) {
        cpu_type10_txs(cpu, opcode);
    } else {
        u_int8_t *result = get_operp_stx(cpu, opcode);
        *result = cpu->mregister.X;
        setZN(cpu, *result);
    }
}

//类型10
static void cpu_type10(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t type = opcode >> 5;
    void (*type_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_shift_asl, cpu_shift_rol, cpu_shift_lsr,
        cpu_shift_ror, cpu_type10_stx, cpu_type10_ldx,
        cpu_type10_dec, cpu_type10_inc
    };
    (*(type_funs[type]))(cpu, opcode);
}

//软件中断
static void cpu_type00_brk(nes_cpu_t *cpu, u_int8_t opcode) {

}

//处理器状态压入堆栈。
static void cpu_type00_php(nes_cpu_t *cpu, u_int8_t opcode) {
    
}