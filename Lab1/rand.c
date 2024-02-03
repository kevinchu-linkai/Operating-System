#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]);

    if (N <= 0) {
        fprintf(stderr, "Please enter a positive integer\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        printf("%d ", rand());
    }
    printf("\n");

    return EXIT_SUCCESS;
}
