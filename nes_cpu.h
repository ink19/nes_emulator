#ifndef NES_CPU_H__
#define NES_CPU_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

//6502 小端

typedef struct {
    //7 6 5 4 3 2 1 0
    //N V E B D I Z C
    //0011 0100
    struct {
        //当操作结果高位被设置（为负），该位被设置，否则被清除
        u_int8_t N;
        //当操作发生借位或进位时，该位被设置
        u_int8_t V;
        //遇到‘BRK’指令时，该位被设置
        u_int8_t B;
        //当该位被设置时，所以的算术操作都是BCD (例如：09+01=10)。当该位被清除时，所有的算术操作都是以二为补码的二进制 (例如： 09+01=0A)
        u_int8_t D;
        //当该位被设置时，不会发生中断
        u_int8_t I;
        //当操作结果为零时，该位被设置。否则被清除
        u_int8_t Z;
        //当发生进位时，该位被设置
        u_int8_t C;
    } flags;
    struct {
        u_int8_t A;
        u_int8_t X;
        u_int8_t Y;
        u_int16_t PC;
        u_int8_t SP;
    } mregister;
    u_int8_t *memory;
} nes_cpu_t;

#endif