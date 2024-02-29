#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"
#include <ctype.h> 

#define LINE_SIZE 1024      // max can be 1024 bytes
#define BUFFER_SIZE 128
// #define MAX_ARGS 20 

// reads a single line from the terminal returns a pointer
// to the character array or NULL if an error is encountered

/*Further explanation about why returning a char pointer ; a C string is represented as a 
sequence of characters, and it is typically accessed through a pointer to the first character 
in that sequence. Therefore, when you have a char * pointer in C, it can be used to point to 
the beginning of a character sequence, which effectively makes it a string.*/
char *readLine(){
    int lineLength;
    char *line = (char *) malloc(sizeof(char) * LINE_SIZE);
    char c;
    int count = 0;
    int cap = LINE_SIZE;

    while(1){
        c = getchar();
        if(c == '\n' || c == EOF){
            line[count] = '\0';
            return line;
        }

        if(count == cap){
            cap += LINE_SIZE;
            line = (char *) realloc(line, cap);
            if(line == NULL){
                fprintf(stderr, "readLine: cannot allocate memory\n");
                return NULL;
            }
        }

        line[count++] = c;
    }

    return line;
}

// obtains user input line and splits up into multiple commands separated by pipes
// returns a double pointer to a string array, each entry containing the string 
// associated with a single command in the pipe
/*
I/O example:
input:
    line = "cmd1 < inputFile.txt | cmd2 | cmd3 arg1 arg2 &"
    numOfCommands is not initialized, it will be calculated and returned by reference
output:
    numOfCommands = 3
    return value = 
    {
        "cmd1 < inputFile.txt",
        "cmd2",
        "cmd3 arg1 arg2 &"
    }
*/
// Splits a command line into separate commands based on the pipe '|' character
// and returns an array of commands.
char **splitLine(char *line, int *numOfCommands) {
    int bufsize = BUFFER_SIZE; 
    int position = 0; 
    char **tokens = malloc(bufsize * sizeof(char*)); // Allocate memory for the tokens array
    char *token; // Pointer to store each token found by strtok

    // Check if memory allocation for tokens array was successful
    if (!tokens) {
        fprintf(stderr, "splitLine: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Use strtok to find the first token, split by the pipe character
    token = strtok(line, "|");
    while (token != NULL) { // Continue until no more tokens are found
        tokens[position++] = token; // Assign the token to the current position in the array

        // If we've reached the current buffer size, increase the buffer size
        if (position >= bufsize) {
            bufsize += BUFFER_SIZE; 
            tokens = realloc(tokens, bufsize * sizeof(char*)); 
            // Check if memory reallocation was successful
            if (!tokens) {
                fprintf(stderr, "splitLine: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // Find the next token
        token = strtok(NULL, "|");
    }
    tokens[position] = NULL; 
    *numOfCommands = position; 
    return tokens; 
}


// obtains a single string representing a command
// parses the words of the command into tokens and strips
// any single or double quotes around any one word
// NOTE: arguments surrounded by double or single quotes
// are considered one argument or one token; meaning:
// cmd1 "input string" has two tokens, not 3.
/*
I/O example1:
input:
    command = "cmd3 arg1 arg2 &"
    numOfWords is not initialized, it will be calculated and returned by reference
output:
    numOfWords = 4
    return value =
    {
        "cmd3",
        "arg1",
        "arg2",
        "&"
    }

I/O example 2:
input:
    command = echo "Hello world"
    numOfWords is not initialized, it will be calculated and returned by reference
output:
    numOfWords = 2
    return value = 
    {
        "echo",
        "Hello world"
    }
*/
// Parses a single command into tokens, taking into account quoted strings
// and returns an array of tokens/words found in the command.
char **parseCommand(char *command, int *numOfWords) {
    int bufsize = BUFFER_SIZE; 
    int position = 0; 
    char **tokens = malloc(bufsize * sizeof(char*)); 
    char *token; // Pointer to store the start of each token
    char *cursor; // Pointer used to iterate through the command string
    int inSingleQuote = 0, inDoubleQuote = 0; // Flags to track if inside quotes

    // Check if memory allocation for tokens array was successful
    if (!tokens) {
        fprintf(stderr, "parseCommand: allocation error\n");
        exit(EXIT_FAILURE);
    }

    cursor = command; // Initialize cursor to the start of the command
    while (*cursor) { // Iterate through the command until the end of the string
        // Skip any leading whitespace before a token
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;

        if (!*cursor) break; // If reached the end of the string, stop parsing

        // Mark the start of a token
        token = cursor;

        // Iterate over the characters until the end of the token is found
        while (*cursor) {
            if (*cursor == '\'' && !inDoubleQuote) { // Handling single quotes
                inSingleQuote = !inSingleQuote; // Toggle the inSingleQuote flag
                memmove(cursor, cursor + 1, strlen(cursor)); // Remove the quote from the command
            } else if (*cursor == '\"' && !inSingleQuote) { // Handling double quotes
                inDoubleQuote = !inDoubleQuote; // Toggle the inDoubleQuote flag
                memmove(cursor, cursor + 1, strlen(cursor)); // Remove the quote from the command
            } else if (isspace((unsigned char)*cursor) && !inSingleQuote && !inDoubleQuote) {
                break; // End of the token if whitespace is found outside quotes
            } else {
                cursor++; // Move to the next character
            }
        }

        // Check if the tokens array needs to be resized
        if (position >= bufsize) {
            bufsize += BUFFER_SIZE; 
            tokens = realloc(tokens, bufsize * sizeof(char*)); 
            if (!tokens) {
                fprintf(stderr, "parseCommand: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // Assign the token to the current position in the array and increment the position
        tokens[position++] = token;
        if (*cursor) {
            *cursor = '\0'; // Null-terminate the token
            cursor++; // Move past the end of the current token
        }
    }

    tokens[position] = NULL; 
    *numOfWords = position; 
    return tokens; 
}




// obtains the tokens and number of tokens associated with a single command
// also obtain the in and out file descriptors if successful, this function
// will execute the function given, reading from inFD and writing to outFD
// and won't return. If not, it will return 1. This function is also 
// responsible for handling redirects and the handling of background processes
int shellExecute(char *tokens[], int numOfTokens, int inFD, int outFD) {
    pid_t pid;
    int status;
    int background = 0;
    int inputRedirected = 0, outputRedirected = 0;

    // Parse tokens for redirections and background flag
    for (int i = 0; i < numOfTokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            inputRedirected = 1;
            inFD = open(tokens[i + 1], O_RDONLY);
            if (inFD < 0) {
                perror("Failed to open input file");
                return -1;
            }
            tokens[i] = NULL; // Remove redirection from command
            i++; // Skip filename
        } else if (strcmp(tokens[i], ">") == 0) {
            outputRedirected = 1;
            char filepath[BUFFER_SIZE]; // Define a buffer to hold the filepath
            sprintf(filepath, "./%s", tokens[i + 1]); // Prepend "./" to the filename
            outFD = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);  
            if (outFD < 0) {
                perror("Failed to open output file");
                return -1;
            }
            tokens[i] = NULL; // Remove redirection from command
            i++; // Skip filename
        } else if (strcmp(tokens[i], "&") == 0) {
            background = 1;
            tokens[i] = NULL; // Remove background symbol from command
        }
    }

    pid = fork();
    if (pid == 0) { // Child process
        // Redirect input if necessary
        if (inFD != STDIN_FILENO) {
            dup2(inFD, STDIN_FILENO);
            close(inFD);
        }
        // Redirect output if necessary
        if (outFD != STDOUT_FILENO) {
            dup2(outFD, STDOUT_FILENO);
            close(outFD);
        }

        // Close the read end of the pipe if it's not STDIN
        if (inFD != 0) close(inFD);
        // Close the write end of the pipe if it's not STDOUT
        if (outFD != 1) close(outFD);

        // Prepend /bin/ to the command if it doesn't already specify a path
        if (tokens[0][0] != '/') {
            char cmdpath[BUFFER_SIZE]; // Define a buffer to hold the filepath
            sprintf(cmdpath, "/bin/%s", tokens[0]); // Prepend "./" to the filename
            tokens[0] = cmdpath; // Point the first token to the full command path
        }

        if (execvp(tokens[0], tokens) == -1) {
            perror("execvp error");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        // Error forking
        perror("fork error");
        return -1;
    } else {
        // Parent process does not wait for background processes
        if (!background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("[Running in background] PID: %d\n", pid);
            // Optionally, to ensure the shell doesn't wait for this process:
            setpgid(pid, pid); // Detach the process group of the background process
        }

        // Close file descriptors in the parent to avoid leaks
        if (inFD != 0) close(inFD);
        if (outFD != 1) close(outFD);
    }

    return 1;
}


int main(){
    char *line = NULL;              // line read from terminal
    /*NOTE: this line is not representitive of a concatenated argv since
    it may include symbols such as such as '|', '&', '<' or '>'*/

    char **commandList = NULL;      // list of commands stored in a string array
    int numOfCommands;              // number of commands in a single line
    /*NOTE: a single entry within the commandList is not argv since that entry
    may contain symbols such as '&', '<' or '>'*/

    char **tokens = NULL;           // tokens associated with a single command
    int numOfTokens;                // number of tokens associated with a single command
    /*NOTE: these are not yet argv and argc since they may contain non argument tokens
    such as '&', '<' or '>'.*/

    int status;                     // return status of an executed command

    while(1){
        printf("\n>> ");
        line = readLine();
        commandList = splitLine(line, &numOfCommands);
        
        // if a single command is parsed, it means there are
        // no pipes and we can just execute it normally
        if(numOfCommands == 1){
            tokens = parseCommand(line, &numOfTokens);

            // we check if the command given is a shell builtin
            // those have other APIs we can utilize without needing
            // to fork or do anything fancy
            if(strcmp(tokens[0], "exit") == 0){
                printf("Exiting...\n");
                break;
            }
            else if(strcmp(tokens[0], "cd") == 0){
                if (tokens[1] == NULL) {
                    fprintf(stderr, "cd: no path directory not specified\n");
                }
                else {
                    if (chdir(tokens[1]) != 0) {
                        fprintf(stderr, "cd: directory not found\n");
                    }
                }
            }
            else if(strcmp(tokens[0], "help") == 0){
                printf("SHELL HELP\n");
                printf("These are the built-in commands:\n");
                printf("    - help\n");
                printf("    - cd <path>\n");
                printf("    - exit\n");
                printf("Type man <command> to know about a command\n");
                printf("Type man to know about other commands\n");
                status = 0;
            }
            
            // if it's not a shell builtin, we execute it with standard 
            // in and out file descriptors
            else
                status = shellExecute(tokens, numOfTokens, 0, 1);
        }
        
        // if we have multiple commands and one or more pipes
        // then we have to construct and allocate the pipes
        // such that we can establish interprocess communication (IPC) 
        else if(numOfCommands > 1){
            int inFD = 0; // Initially, input is from stdin
            for (int i = 0; i < numOfCommands; i++) {
                int fd[2];
                // Create a pipe for all but the last command
                if (i < numOfCommands - 1) {
                    if (pipe(fd) != 0) {
                        perror("Pipe failed");
                        exit(EXIT_FAILURE);
                    }
                }

                char **tokens;
                int numOfTokens;
                tokens = parseCommand(commandList[i], &numOfTokens);

                if (i == numOfCommands - 1) { // For the last command, output to stdout
                    shellExecute(tokens, numOfTokens, inFD, STDOUT_FILENO);
                } else {
                    // For other commands, output to the pipe
                    shellExecute(tokens, numOfTokens, inFD, fd[1]);
                    close(fd[1]); // Close the write-end of the pipe after use
                }

                // The next command should read from the current pipe
                if (inFD != STDIN_FILENO) {
                    close(inFD); // Close the previous read-end of the pipe
                }
                inFD = fd[0]; // For the next command, read from the current pipe

                // Clean up
                free(tokens);
            }
        }

        // if you used dynamic mmeory allocation (which I highly recommend in this
        // instance) this code will deallocate it
        if(tokens){
            free(tokens);
            tokens = NULL;
        }
        if(line){
            free(line);
            line = NULL;
        }
        if(commandList){
            free(commandList);
            commandList = NULL;
        }
    }
    // IF for whatever reason we left the while loop earlier - make sure everything is fine 
    if(tokens) free(tokens);
    if(line) free(line);
    if(commandList) free(commandList);
    return 0;
}



// **********  TESTING ********** 
// worked: 
    // cat starterCode.c | wc > newfile.txt 
    // wc < newfile.txt | cat & 
    // wc < newfile.txt | cat
    // wc < newfile.txt | cat | cat > finalfile.txt

// "cmd1 < inputFile.txt | cmd2 | cmd3 arg1 arg2 &"

// ls 
// ls -l 

// cat 
    // on its own 
    // cat testfile.txt | wc 
    // cat tempfile.txt | wc > FILEOUT &

// wc < tempfile.txt | cat | cat > finalFILE_new

// TESTING FOR ALL TYPES OF EDGE CASES // 

// Echo Command (echo):
    // echo "Hello, world!"
    // echo "The price is \$10."
    // echo ""
    // echo "Hello," "world!"
    // echo "Hello, world!" > output.txt
    // echo -e "Hello\tworld\n" ???? 


// List Files (ls):
    // ls
    // ls -a
    // ls -l
    // ls /path/to/directory -> ls /Users/mac/Desktop/OS/lab3_OS
    // ls -lh
    // ls -lt
    // ls -R
    // ls -l -a



// Word Count (wc):
    // wc filename.txt
    // wc -l filename.txt
    // wc -w filename.txt
    // wc -c filename.txt
    // echo "Hello, world!" | wc -w



// Concatenate (cat):
    // cat filename.txt
    // cat file1.txt file2.txt
    // cat file1.txt file2.txt > newfile.txt
    // cat -n filename.txt
    // ls | cat -n
    // cat file1.txt >> file2.txt -  ????


// Background Process (&):
    // Run a command in the background: sleep 10 &
    // Multiple background processes: sleep 5 & echo "Hello" & ls -l & - ??? 


// Pipes (|):
    // ls | grep "file"
    // ls | wc -l
    // ls | grep "file" | wc -l
    // ls | grep "file" > output.txt


// Redirection (<, >):
    // wc < input.txt
    // ls > filelist.txt
    // sort < input.txt > output.txt
    // cat < input.txt | grep "keyword"