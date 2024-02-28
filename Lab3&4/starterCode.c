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
char **splitLine(char *line, int *numOfCommands){
    int bufsize = BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "splitLine: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, "|");
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += BUFFER_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "splitLine: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

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
char **parseCommand(char *command, int *numOfWords) {
    int bufsize = BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    char *cursor;
    int inSingleQuote = 0, inDoubleQuote = 0;

    if (!tokens) {
        fprintf(stderr, "parseCommand: allocation error\n");
        exit(EXIT_FAILURE);
    }

    cursor = command;
    while (*cursor) {
        // Skip leading whitespace
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;

        if (!*cursor) break; // Break if end of string

        // Start of a token
        token = cursor;

        while (*cursor) {
            if (*cursor == '\'' && !inDoubleQuote) {
                inSingleQuote = !inSingleQuote;
                memmove(cursor, cursor + 1, strlen(cursor)); // Remove quote from command
            } else if (*cursor == '\"' && !inSingleQuote) {
                inDoubleQuote = !inDoubleQuote;
                memmove(cursor, cursor + 1, strlen(cursor)); // Remove quote from command
            } else if (isspace((unsigned char)*cursor) && !inSingleQuote && !inDoubleQuote) {
                break; // End of token if not within quotes
            } else {
                cursor++;
            }
        }

        if (position >= bufsize) {
            bufsize += BUFFER_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "parseCommand: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // Cut the token and move to the next
        tokens[position++] = token;
        if (*cursor) {
            *cursor = '\0'; // Null-terminate the token
            cursor++;
        }
    }

    tokens[position] = NULL; // Null-terminate the list of tokens
    *numOfWords = position;
    return tokens;
}



// obtains the tokens and number of tokens associated with a single command
// also obtain the in and out file descriptors if successful, this function
// will execute the function given, reading from inFD and writing to outFD
// and won't return. If not, it will return 1. This function is also 
// responsible for handling redirects and the handling of background processes
int shellExecute(char *tokens[], int numOfTokens, int inFD, int outFD){
    pid_t pid, wpid;
    int status, background = 0;

    // Check for background process
    if (strcmp(tokens[numOfTokens - 1], "&") == 0) {
        background = 1;
        tokens[numOfTokens - 1] = NULL;
    }

    pid = fork();
    if (pid == 0) {
        // Child process
        if (inFD != 0) {
            dup2(inFD, STDIN_FILENO);
            close(inFD);
        }

        if (outFD != 1) {
            dup2(outFD, STDOUT_FILENO);
            close(outFD);
        }

        if (execvp(tokens[0], tokens) == -1) {
            perror("shellExecute");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("shellExecute");
    } else {
        // Parent process
        if (!background) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
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
        line = readLine();
        commandList = splitLine(line, &numOfCommands);
        printf("\n>> ");

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
    // cat testfile.txt | wc > newfile.txt 
    // wc < tempfile.txt | cat & 
    // wc < tempfile.txt | cat
    // wc < tempfile.txt | cat | cat > finalfile.txt

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