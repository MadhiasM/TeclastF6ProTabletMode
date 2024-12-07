#include <stdio.h>
#include <stdint.h>

// Pointer casting
int main() {
    float x = -5.2f;

    // Den Wert als ganzzahligen Wert (int) interpretieren
    uint32_t bits = *((uint32_t*)&x);

    // Das Vorzeichenbit ist das höchstwertige Bit (MSB)
    if (bits & 0x80000000) {
        printf("Die Zahl ist negativ.\n");
    } else {
        printf("Die Zahl ist positiv.\n");
    }

    return 0;
}


// UNION

/*
#include <stdio.h>
#include <stdint.h>

typedef union {
    float f;
    uint32_t i;
} FloatUnion;

int main() {
    FloatUnion u;
    u.f = -5.2f;

    if (u.i & 0x80000000) {
        printf("Die Zahl ist negativ.\n");
    } else {
        printf("Die Zahl ist positiv.\n");
    }

    return 0;
}
 */
