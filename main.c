#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nes_system.h"

nes_emulator_t emu;

int main() {
    load_game(&emu, "smb.nes");
    return 0;
}