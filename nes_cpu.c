#include "nes_cpu.h"

static void stack_push(nes_cpu_t *cpu, u_int8_t data) {
    *get_memory(cpu, cpu->mregister.SP + 0x0100) = data;
    cpu->mregister.SP--;
}

static void stack_pop(nes_cpu_t *cpu, u_int8_t *data) {
    cpu->mregister.SP++;
    u_int8_t *data = get_memory(cpu, cpu->mregister.SP + 0x0100);
}

static u_int8_t read_opcode_byte(nes_cpu_t *cpu) {
    return *get_memory(cpu, cpu->mregister.PC++);
}

static u_int8_t addr_imm(nes_cpu_t *cpu) {
    return read_opcode_byte(cpu);
}

static u_int8_t addr_zp(nes_cpu_t *cpu) {
    return *get_memory(cpu, read_opcode_byte(cpu));
}

static u_int8_t addr_zpx(nes_cpu_t *cpu) {
    return *get_memory(cpu, (read_opcode_byte(cpu) + cpu->mregister.X) & 0xff);
}

static u_int8_t addr_indexindiret(nes_cpu_t *cpu) {
    //u_int8_t *memory = cpu->memory;
    u_int8_t X = cpu->mregister.X;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = ((u_int16_t)*get_memory(cpu, (data + X) & 0xff)) + ((u_int16_t)*get_memory(cpu, (data + X + 1) & 0xff)) << 8;
    return *get_memory(cpu, addr);
}

static u_int8_t addr_indiretindex(nes_cpu_t *cpu) {
    //u_int8_t *memory = cpu->memory;
    u_int8_t Y = cpu->mregister.Y;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = (u_int16_t)(*get_memory(cpu, data)) + (u_int16_t)(*get_memory(cpu, (data + 1) & 0xff)) << 8 + Y;
    return *get_memory(cpu, addr);
}

static u_int8_t addr_absx(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    //addr <<= 8;
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;
    addr += cpu->mregister.X;
    return *get_memory(cpu, addr);
}

static u_int8_t addr_absy(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.Y;
    return *get_memory(cpu, addr);
}

static u_int8_t addr_abs(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    return *get_memory(cpu, addr);
}

static u_int8_t *addr_zp_write(nes_cpu_t *cpu) {
    return get_memory(cpu, read_opcode_byte(cpu));
}

static u_int8_t *addr_zpx_write(nes_cpu_t *cpu) {
    return get_memory(cpu, (read_opcode_byte(cpu) + cpu->mregister.X) % 0xff);
}

static u_int8_t *addr_abs_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    return get_memory(cpu, addr);
}

static u_int8_t *addr_absx_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.X;
    return get_memory(cpu, addr);
}

static u_int8_t *addr_absy_write(nes_cpu_t *cpu) {
    u_int16_t addr, addrl, addrh;
    addrl = read_opcode_byte(cpu);
    addrh = read_opcode_byte(cpu);
    addr = (addrh << 8) | addrl;

    addr += cpu->mregister.Y;
    return get_memory(cpu, addr);
}

static u_int8_t *addr_indexindiret_write(nes_cpu_t *cpu) {
    u_int8_t X = cpu->mregister.X;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = ((u_int16_t)*get_memory(cpu, (data + X) & 0xff)) + ((u_int16_t)*get_memory(cpu, (data + X + 1) & 0xff)) << 8;
    return get_memory(cpu, addr);
}

static u_int8_t *addr_indiretindex_write(nes_cpu_t *cpu) {
    u_int8_t Y = cpu->mregister.Y;
    u_int8_t data = read_opcode_byte(cpu);
    u_int16_t addr = (u_int16_t)(*get_memory(cpu, data)) + (u_int16_t)(*get_memory(cpu, (data + 1) & 0xff)) << 8 + Y;
    return get_memory(cpu, addr);
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
    return *get_memory(cpu, (read_opcode_byte(cpu) + cpu->mregister.Y) % 0xff);
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
    return get_memory(cpu, (read_opcode_byte(cpu) + cpu->mregister.Y) % 0xff);
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
    u_int8_t data = 0;
    data |= cpu->flags.C;
    data |= (cpu->flags.Z) << 1;
    data |= (cpu->flags.I) << 2;
    data |= (cpu->flags.D) << 3;
    data |= (cpu->flags.B) << 4;
    data |= (cpu->flags.V) << 6;
    data |= (cpu->flags.N) << 7;
    stack_push(cpu, data);
}

//N = 0分支
static void cpu_type00_bpl(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(cpu->flags.N) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//清进位标志C
static void cpu_type00_clc(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.C = 0;
}

//000
static void cpu_type00_000(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_000_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_brk, NULL, cpu_type00_php, NULL, 
        cpu_type00_bpl, NULL, cpu_type00_clc, NULL
    };
    (*cpu_type00_000_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//BIT 位测试
static void cpu_type00_bit(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t data;
    if((opcode >> 2) & 0x07 == 0x01) {
        data = addr_zp(cpu);
    } else if((opcode >> 2) & 0x07 == 0x03) {
        data = addr_abs(cpu);
    }
    setN(cpu, data);
    cpu->flags.V = (data >> 6) & 0x01;
    setZ(cpu, data & (cpu->mregister.A));
}

// 类似CALL
static void cpu_type00_jsr(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t temp_PC;
    stack_push(cpu, (cpu->mregister.PC + 1) & 0xff);
    stack_push(cpu, ((cpu->mregister.PC + 1) >> 8) & 0xff);
    temp_PC = read_opcode_byte(cpu);
    temp_PC |= ((u_int16_t)read_opcode_byte(cpu)) << 8;
    cpu->mregister.PC = temp_PC;
}

//从栈中弹回状态寄存器P
static void cpu_type00_plp(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t data = 0;
    stack_pop(cpu, &data);
    cpu->flags.C = data & 0x01;
    cpu->flags.Z = (data >> 1) & 0x01;
    cpu->flags.I = (data >> 2) & 0x01;
    cpu->flags.D = (data >> 3) & 0x01;
    cpu->flags.B = (data >> 4) & 0x01;
    cpu->flags.V = (data >> 6) & 0x01;
    cpu->flags.N = (data >> 7) & 0x01;
}

//N=1 分支
static void cpu_type00_bmi(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(!cpu->flags.N) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//C = 1
static void cpu_type00_sec(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.C = 1;
}

//001
static void cpu_type00_001(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_001_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_jsr, cpu_type00_bit, cpu_type00_plp, cpu_type00_bit, 
        cpu_type00_bmi, NULL, cpu_type00_sec, NULL
    };
    (*cpu_type00_001_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//V=0的分支
static void cpu_type00_bvc(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(cpu->flags.V) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//跳转(goto)
static void cpu_type00_jmp(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t jmp_pc = 0, temp_data = 0;
    if(opcode == 0x4c) {
        jmp_pc = read_opcode_byte(cpu);
        jmp_pc |= ((u_int16_t)read_opcode_byte(cpu)) << 8;
    } else if(opcode == 0x6c) {
        temp_data = read_opcode_byte(cpu);
        temp_data |= ((u_int16_t)read_opcode_byte(cpu)) << 8;
        jmp_pc = *get_memory(cpu, temp_data);
        jmp_pc = ((u_int16_t)(*get_memory(cpu, temp_data + 1))) << 8;
    }
    cpu->mregister.PC = jmp_pc;
}

//中断返回
static void cpu_type00_rti(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t data = 0;
    stack_pop(cpu, &data);
    cpu->flags.C = data & 0x01;
    cpu->flags.Z = (data >> 1) & 0x01;
    cpu->flags.I = (data >> 2) & 0x01;
    cpu->flags.D = (data >> 3) & 0x01;
    cpu->flags.B = (data >> 4) & 0x01;
    cpu->flags.V = (data >> 6) & 0x01;
    cpu->flags.N = (data >> 7) & 0x01;

    u_int16_t temp_PC;
    stack_pop(cpu, &data);
    temp_PC = ((u_int16_t)data) << 8;
    stack_pop(cpu, &data);
    temp_PC |= ((u_int16_t)data);
}

//I = 0
static void cpu_type00_cli(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.I = 0;
}

//累加器入栈
static void cpu_type00_pha(nes_cpu_t *cpu, u_int16_t opcode) {
    stack_push(cpu, cpu->mregister.A);
}

//010
static void cpu_type00_010(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_010_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_rti, NULL, cpu_type00_pha, cpu_type00_jmp, 
        cpu_type00_bvc, NULL, cpu_type00_cli, NULL
    };
    (*cpu_type00_010_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//V = 1 分支
static void cpu_type00_bvs(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(!(cpu->flags.V)) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//子程序返回
static void cpu_type00_rts(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t data = 0;

    u_int16_t temp_PC;
    stack_pop(cpu, &data);
    temp_PC = ((u_int16_t)data) << 8;
    stack_pop(cpu, &data);
    temp_PC |= ((u_int16_t)data);
}

//I = 1
static void cpu_type00_sei(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.I = 1;
}

//累加器出栈
static void cpu_type00_pla(nes_cpu_t *cpu, u_int8_t opcode) {
    stack_pop(cpu, &(cpu->mregister.A));
    setZN(cpu, cpu->mregister.A);
}

//011
static void cpu_type00_011(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_011_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_rts, NULL, cpu_type00_pla, cpu_type00_jmp, 
        cpu_type00_bvs, NULL, cpu_type00_sei, NULL
    };
    (*cpu_type00_011_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//寄存器X-1
static void cpu_type00_dex(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.X--;
    setZN(cpu, cpu->mregister.X);
}

//C = 0分支
static void cpu_type00_bcc(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(cpu->flags.C) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//Y -> M 10000100
static void cpu_type00_sty(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t *datap = get_operp(cpu, opcode);
    *datap = cpu->mregister.Y;
}

//Y -> A
static void cpu_type00_tya(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.A = cpu->mregister.Y;
    setZN(cpu, cpu->mregister.A);
}

//100
static void cpu_type00_100(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_100_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        NULL, cpu_type00_sty, cpu_type00_dex, cpu_type00_sty, 
        cpu_type00_bcc, cpu_type00_sty, cpu_type00_tya, NULL
    };
    (*cpu_type00_100_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//C = 1分支
static void cpu_type00_bcs(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(!(cpu->flags.C)) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//V = 0
static void cpu_type00_clv(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.V = 0;
}

//A -> Y
static void cpu_type00_tay(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.Y = cpu->mregister.A;
    setZN(cpu, cpu->mregister.Y);
}

static u_int8_t get_oper_ldy(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_imm, addr_zp, NULL, addr_abs,
        NULL, addr_zpx, NULL, addr_absx
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//Y = M
static void cpu_type00_ldy(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t data = get_oper_ldy(cpu, opcode);
    cpu->mregister.Y = data;
}

//101
static void cpu_type00_101(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_101_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_ldy, cpu_type00_ldy, cpu_type00_tay, cpu_type00_ldy, 
        cpu_type00_bcs, cpu_type00_ldy, cpu_type00_clv, cpu_type00_ldy
    };
    (*cpu_type00_101_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//Y++
static void cpu_type00_iny(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.Y++;
    setZN(cpu, cpu->mregister.Y);
}

//Z=0分支
static void cpu_type00_bne(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(cpu->flags.Z) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//D=0
static void cpu_type00_cld(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.D = 0;
}

static u_int8_t get_oper_cpy(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_imm, addr_zp, NULL, addr_abs,
        NULL, NULL, NULL, NULL
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//Y与M进行比较
static void cpu_type00_cpy(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t result = cpu->mregister.Y;
    result -= get_oper_cpy(cpu, opcode);
    if(result > 0xff) {
        cpu->flags.C = 1;
    } else {
        cpu->flags.C = 0;
    }
    setZN(cpu, result);
}

//110
static void cpu_type00_110(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_110_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_cpy, cpu_type00_cpy, cpu_type00_iny, cpu_type00_cpy, 
        cpu_type00_bne, NULL, cpu_type00_cld, NULL
    };
    (*cpu_type00_110_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

//X++
static void cpu_type00_inx(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->mregister.X++;
    setZN(cpu, cpu->mregister.X);
}

//Z=1分支
static void cpu_type00_beq(nes_cpu_t *cpu, u_int8_t opcode) {
    int8_t data = (int8_t)read_opcode_byte(cpu);
    if(!(cpu->flags.Z)) return; 
    cpu->mregister.PC -= 2;
    cpu->mregister.PC += data;
}

//D = 1
static void cpu_type00_sed(nes_cpu_t *cpu, u_int8_t opcode) {
    cpu->flags.D = 1;
}

static u_int8_t get_oper_cpx(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t (*get_oper_fun[])(nes_cpu_t *cpu) = {
        addr_imm, addr_zp, NULL, addr_abs,
        NULL, NULL, NULL, NULL
    };
    return (*(get_oper_fun[(opcode >> 2) & 0x07]))(cpu);
}

//Y与M进行比较
static void cpu_type00_cpx(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int16_t result = cpu->mregister.X;
    result -= (u_int16_t)get_oper_cpx(cpu, opcode);
    if(result > 0xff) {
        cpu->flags.C = 1;
    } else {
        cpu->flags.C = 0;
    }
    setZN(cpu, result);
}

//111
static void cpu_type00_111(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*cpu_type00_111_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_cpx, cpu_type00_cpx, cpu_type00_inx, cpu_type00_cpx, 
        cpu_type00_beq, NULL, cpu_type00_sed, NULL
    };
    (*cpu_type00_111_funs[(opcode >> 2) & 0x07])(cpu, opcode);
}

static void cpu_type00(nes_cpu_t *cpu, u_int8_t opcode) {
    u_int8_t type = opcode >> 5;
    void (*type_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00_000, cpu_type00_001, cpu_type00_010,
        cpu_type00_011, cpu_type00_100, cpu_type00_101,
        cpu_type00_110, cpu_type00_111
    };
    (*(type_funs[type]))(cpu, opcode);
}

static void cpu_run_code(nes_cpu_t *cpu, u_int8_t opcode) {
    void (*run_code_funs[])(nes_cpu_t *cpu, u_int8_t opcode) = {
        cpu_type00, cpu_type01, cpu_type10, NULL
    };
    (*(run_code_funs[opcode & 0x03]))(cpu, opcode);
}