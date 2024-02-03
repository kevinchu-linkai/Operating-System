#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int c;
    char old_char, new_char;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <old_char> <new_char>\n", argv[0]);
        return EXIT_FAILURE;
    }

    old_char = argv[1][0];
    new_char = argv[2][0];

    while ((c = getchar()) != EOF) {
        if (c == old_char) {
            c = new_char;
        }
        putchar(c);
    }

    return EXIT_SUCCESS;
}