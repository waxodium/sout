#include "sout_printout.h"

int main(void) {
    char text[] = "Hello, World!";
    int limit = 5;

    soutf("Normal: %s\n", text);

    soutf("Fixed limit: %.5s\n", text);

    soutf("Dynamic limit: %.*s\n", limit, text);

    soutf("Dynamic 8: %.*s\n", 8, text);

    soutf("Dynamic 20: %.*s\n", 20, text);

    soutf("Negative precision: %.*s\n", -1, text);

    soutf("NULL: %.5s\n", (char *)0);

    soutf("Hex: %x\n", 305441741);
    soutf("Hex prefix: %#x\n", 305441741);
    soutf("Padded hex: %08x\n", 305441741);
    soutf("Prefixed padded hex: %#08x\n", 305441741);

    soutf("Decimal: %d\n", -12345);
    soutf("Unsigned: %u\n", 12345u);
    soutf("Character: %c\n", 'A');
    soutf("Percent: %%\n");

    return 0;
}

