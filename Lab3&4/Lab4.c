#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 256
#define MAX_ARGS 64
#define MAX_CMDS 16

void strip_newline(char *str) {
    if (str == NULL) return;
    char *newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

int parse_command(char *cmd, char **cmds) {
    int i = 0;
    cmds[i] = strtok(cmd, "|");
    while (cmds[i] != NULL) {
        cmds[++i] = strtok(NULL, "|");
    }
    return i; // Number of commands
}

void parse_args(char *cmd, char **argv, char **input_file, char **output_file, int *is_background) {
    int i = 0;
    *is_background = 0;
    *input_file = NULL;
    *output_file = NULL;

    argv[i] = strtok(cmd, " ");
    while (argv[i] != NULL) {
        if (strcmp(argv[i], "<") == 0) {
            *input_file = strtok(NULL, " ");
        } else if (strcmp(argv[i], ">") == 0) {
            *output_file = strtok(NULL, " ");
        } else if (strcmp(argv[i], "&") == 0) {
            *is_background = 1;
        } else {
            argv[++i] = strtok(NULL, " ");
            continue;
        }
        argv[i] = strtok(NULL, " ");
    }
}

void run_command(char **argv, int in_fd, int out_fd, int is_background, char *input_file, char *output_file) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) { // Child process
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        if (input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (output_file) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Parent process
        if (!is_background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITed(status) && !WIFSIGNAled(status));
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

void execute_command(char **cmds, int cmd_count) {
    int i;
    int in_fd = 0, fd[2];
    char *argv[MAX_ARGS];
    char *input_file, *output_file;
    int is_background;

    for (i = 0; i < cmd_count; i++) {
        parse_args(cmds[i], argv, &input_file, &output_file, &is_background);

        if (i < cmd_count - 1) { // Not the last command
            pipe(fd);
            run_command(argv, in_fd, fd[1], is_background, input_file, output_file);
            close(fd[1]);
            if (in_fd != STDIN_FILENO) close(in_fd);
            in_fd = fd[0]; // Next command reads from here
        } else { // Last command
            run_command(argv, in_fd, STDOUT_FILENO, is_background, input_file, output_file);
            if (in_fd != STDIN_FILENO) close(in_fd);
        }
    }

    if (!is_background) {
        while (wait(NULL) > 0); // Wait for all child processes if not in background
    }
}

int main() {
    char line[MAX_LINE];
    char *commands[MAX_CMDS];
    int command_count;

    while (1) {
        printf("Shell> ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) break;
        strip_newline(line);

        if (strcmp(line, "exit") == 0) break;

        command_count = parse_command(line, commands);
        execute_command(commands, command_count);
    }

    return 0;
}
