#include "nes_system.h"

int load_game(nes_emulator_t *emu, char *filename) {
    FILE *filep = fopen(filename, "rb");
    unsigned char nes_head[16];
    fread(nes_head, sizeof(unsigned char), 16, filep);
    emu->prg_n = nes_head[4];
    emu->chr_n = nes_head[5];
    emu->mapper = (nes_head[6] >> 4) | (nes_head[7] & 0xff00);
    emu->mirror_type = nes_head[6] & 0x0001;
    emu->ram_n = nes_head[8];
    emu->video_type = nes_head[9] & 0x0001;
    emu->cpu.mregister.A = 0;
    emu->cpu.mregister.X = 0;
    emu->cpu.mregister.Y = 0;
    emu->cpu.mregister.SP = 0xFD - 3;
    emu->cpu.flags.N = 0;
    emu->cpu.flags.V = 0;
    emu->cpu.flags.B = 1;
    emu->cpu.flags.D = 0;
    emu->cpu.flags.I = 0;
    emu->cpu.flags.Z = 0;
    emu->cpu.flags.C = 0;
    emu->cpu.memory = (u_int8_t *)malloc(sizeof(u_int8_t) * (0x8000 + emu->prg_n * 0x4000));

    fread(emu->cpu.memory + 0x8000, sizeof(u_int8_t), 0x4000 * emu->prg_n, filep);

    printf("%d %d %d\n", emu->prg_n, emu->chr_n, emu->mapper);

    fclose(filep);
    return 0;
}


int destroy_game(nes_emulator_t *emu) {
    free(emu->cpu.memory);
}
