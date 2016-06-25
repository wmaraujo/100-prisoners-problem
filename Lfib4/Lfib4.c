#include <stdio.h>

#define ARRAY_SIZE (1 << 8)

typedef unsigned char Uc;
static Uc c;
static unsigned int t[ARRAY_SIZE];

void Lfib4_seed(unsigned char seedVal, unsigned int* a) {
    c = seedVal;

    for (int i = 0; i < ARRAY_SIZE; i++) {
        t[i] = a[i];
    }
}

unsigned int Lfib4() {
    t[c]=t[c]+t[(Uc)(c+58)]+t[(Uc)(c+119)]+t[(Uc)(c+179)];
    return t[++c];
}

/* test
int main() {
    FILE* urandom = fopen("/dev/urandom", "r");
    unsigned int seedVal;
    fread(&seedVal, sizeof(seedVal), 1, urandom);
    unsigned int a[ARRAY_SIZE];
    fread(a, sizeof(unsigned int), ARRAY_SIZE, urandom);
    Lfib4_seed(seedVal, a);
    printf("c=%u\n", c);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("t[%d]=%u\n", i, t[i]);
    }

    for (int i = 0; i < 10; i++) {
        printf("%u\n", Lfib4());
    }
    return 0;
}
*/
