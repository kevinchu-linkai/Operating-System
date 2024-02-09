#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 256
#define ARGV_MAX 20
//MAX_COMMAND_LENGTH: the maximum length of the command
//ARGV_MAX: the maximum number of the input of the argument vector

// Function to split the command into program and arguments
void parse_command(char *command, char **argv) {
    for (int i = 0; i < ARGV_MAX; i++) {
        argv[i] = strsep(&command, " ");
        if (argv[i] == NULL) break;
        if (strlen(argv[i]) == 0) i--; 
        // Skip any empty strings
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char *argv[ARGV_MAX];
    char path[MAX_COMMAND_LENGTH];

    while (1) {
        printf("Shell>> ");
        //Reading the user's input command
        //reads the user's input into the buffer command with fgets() 
        //If fgets() returns NULL, there is an error reading the user's input
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            printf("Command reading error\n");
            break; // Exit on EOF
        }
        command[strcspn(command, "\n")] = 0; // Remove newline character

        if (strcmp(command, "exit") == 0) {
            break; // Exit command
        }

        // Check if the user wants to change the working directory
        if (strncmp(command, "cd ", 3) == 0) {
            char *dir = &command[3];
            if (chdir(dir) == -1) {
                printf("Error: Failed to change directory to %s\n", dir);
            }
            continue;
        }

        parse_command(command, argv);
        if (argv[0][0] != '/') {
            // Assume command is in /bin/ if no absolute path is provided
            snprintf(path, sizeof(path), "/bin/%s", argv[0]);
            argv[0] = path;
        }

        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (execv(argv[0], argv) == -1) {
                printf("Execv error\n");
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {
            // Parent process
            wait(NULL); // Wait for child process to finish
        } else {
            // Fork failed
            printf("Fork error\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

