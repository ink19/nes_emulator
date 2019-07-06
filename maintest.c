#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main() {
    u_int8_t datau8 = 0xfe;
    int16_t data2 = (int8_t)datau8;
    u_int16_t data3 = 45;
    data3 += data2;
    printf("%d\n", data3);
}