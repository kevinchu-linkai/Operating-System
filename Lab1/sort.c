#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 10

// Function to compare integers for qsort
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Function to compare integers in reverse order for qsort
int reverse_compare(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

int main(int argc, char *argv[]) {
    int capacity = INITIAL_SIZE;
    int size = 0;
    int *numbers = malloc(capacity * sizeof(int));
    if (numbers == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    FILE *input = stdin;
    FILE *output = stdout;
    int reverse = 0;

    // Parsing command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            reverse = 1;
        } 
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output = fopen(argv[++i], "w");
                if (output == NULL) {
                    perror("Error opening output file");
                    free(numbers);
                    return EXIT_FAILURE;
                }
            } 
            else {
                fprintf(stderr, "No output file specified\n");
                free(numbers);
                return EXIT_FAILURE;
            }
        } 
        else {
            input = fopen(argv[i], "r");
            if (input == NULL) {
                perror("Error opening input file");
                free(numbers);
                return EXIT_FAILURE;
            }
        }
    }

    // Reading
    while (scanf("%d", &numbers[size]) == 1) {
        size++;
        if (size == capacity) {
            capacity *= 2;
            numbers = realloc(numbers, capacity * sizeof(int));
            if (numbers == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return EXIT_FAILURE;
            }
        }
    }

    // Sorting
    qsort(numbers, size, sizeof(int), reverse ? reverse_compare : compare);

    for (int i = 0; i < size; i++) {
        fprintf(output, "%d\n", numbers[i]);
    }

    // Clear the allocation
    free(numbers);
    if (input != stdin) {
        fclose(input);
    }
    if (output != stdout) {
        fclose(output);
    }

    return EXIT_SUCCESS;
}