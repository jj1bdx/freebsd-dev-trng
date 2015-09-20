/*
 * Testing /dev/random
 * by TestU01 BigCrush test
 *
 * To compile (on FreeBSD Port math/testu01):
 * cc -O3 -o randomtest randomtest.c -I/usr/local/include/TestU01 -L/usr/local/lib -ltestu01 -lprobdist -lmylib -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include "gdef.h"
#include "swrite.h"
#include "bbattery.h"
#include "unif01.h"

FILE *fp;

void open_random(void) {
    if ((fp = fopen("/dev/random", "r")) == NULL) {
        perror("open_random(): failed to open /dev/random");
        exit(-1);
    }
    return;
}

unsigned int get_devrandom(void) {
    unsigned int y;
    int num;

    if ((num = fread(&y, sizeof(unsigned int), 1, fp)) == 0) {
        perror("get_devrandom(): failed to fread");
        exit(-1);
    }
    return y;
}

int main (void)
{
    unif01_Gen *gen;

    open_random();

    gen = unif01_CreateExternGenBits("devrandom", get_devrandom);
    bbattery_BigCrush(gen);
    unif01_DeleteExternGenBits(gen);

    fcloseall();
    return 0;
}
