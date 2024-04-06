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
    char **tokens = malloc(bufsize * sizeof(char*)); // Malloc buffer size * size of char*
    char *token; // Pointer to store each token found by strtok

    // Fail check
    if (!tokens) {
        fprintf(stderr, "splitLine: allocation error\n");
        exit(EXIT_FAILURE);
    }
    // Strtok() to split by pipe character
    token = strtok(line, "|");
    
    
    while (token != NULL) { // Continue until no more tokens are found
        tokens[position++] = token; // Assign the token to the current position in the array

        // Resize function
        if (position >= bufsize) {
            bufsize += BUFFER_SIZE; 
            tokens = realloc(tokens, bufsize * sizeof(char*)); 
            // Fail check
            if (!tokens) {
                fprintf(stderr, "splitLine error:\n");
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
// Parses a single command into tokens, taking into account quoted strings
// and now also ignores backslashes.
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
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;

        if (!*cursor) break;
        token = cursor;

        while (*cursor) {
            if (*cursor == '\\' && !(inSingleQuote || inDoubleQuote)) {
                // If we encounter a backslash outside quotes, skip it and move to the next character.
                cursor++;
                continue;
            } else if (*cursor == '\\' && (inSingleQuote || inDoubleQuote))
            {
                // If we encounter a backslash inside quotes,interpret the special characters.
                if (*(cursor + 1) == 'n') {
                    *cursor = '\n';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 't') {
                    *cursor = '\t';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 'v') {
                    *cursor = '\v';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 'b') {
                    *cursor = '\b';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 'r') {
                    *cursor = '\r';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 'f') {
                    *cursor = '\f';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == 'a') {
                    *cursor = '\a';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == '0') {
                    *cursor = '\0';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else if (*(cursor + 1) == '\\') {
                    *cursor = '\\';
                    memmove(cursor + 1, cursor + 2, strlen(cursor + 2) + 1);
                } else {
                    memmove(cursor, cursor + 1, strlen(cursor));
                }
            } 
            
            if (*cursor == '\'' && !inDoubleQuote) {
                inSingleQuote = !inSingleQuote;
                memmove(cursor, cursor + 1, strlen(cursor));
            } else if (*cursor == '\"' && !inSingleQuote) {
                inDoubleQuote = !inDoubleQuote;
                memmove(cursor, cursor + 1, strlen(cursor));
            } else if (isspace((unsigned char)*cursor) && !inSingleQuote && !inDoubleQuote) {
                break;
            } else {
                cursor++;
            }
        }

        if (position >= bufsize) {
            bufsize += BUFFER_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "parseCommand error: \n");
                exit(EXIT_FAILURE);
            }
        }

        tokens[position++] = token;
        if (*cursor) {
            *cursor = '\0';
            cursor++;
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
    int status; // status of the shell execute (if 1, success)
    int background = 0; // Background flag
    int inputRedirected = 0, outputRedirected = 0; // Redirection Fladgs

    // Parse tokens
    for (int i = 0; i < numOfTokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            inputRedirected = 1;
            inFD = open(tokens[i + 1], O_RDONLY);
            if (inFD < 0) {
                perror("Failed to open input file");
                return -1;
            }
            tokens[i] = NULL; // Remove char
            i++; // Skip filename (it will be parsed later).
        } else if (strcmp(tokens[i], ">") == 0) {
            outputRedirected = 1;
            outFD = open(tokens[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);  
            if (outFD < 0) {
                perror("Failed to open output file");
                return -1;
            }
            tokens[i] = NULL; //Remove char
            i++; // Skip filename (it will be parsed later).
        } else if (strcmp(tokens[i], "&") == 0) {
            background = 1;
            tokens[i] = NULL; // Remove background symbol from command
        }
    }

    pid = fork(); // Fork initiate
    if (pid == 0) { // Child process
        // Redirect input
        if (inFD != STDIN_FILENO) {
            dup2(inFD, STDIN_FILENO);
            close(inFD);
        }
        // Redirect output
        if (outFD != STDOUT_FILENO) {
            dup2(outFD, STDOUT_FILENO);
            close(outFD);
        }

        // Close the read end of the pipe if it's not STDIN
        if (inFD != 0) close(inFD);
        // Close the write end of the pipe if it's not STDOUT
        if (outFD != 1) close(outFD);
        
        if (execvp(tokens[0], tokens) == -1) {
            perror("Execvp Error: ");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        //Fail check
        perror("Fork error");
        return -1;
    } else {
        // NOTE*: Parent process does not wait for background processes
        if (!background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            // Ensure the shell doesn't wait for this process:
            setpgid(pid, pid); // Detach the process group of the background process.
        }

        // Close in and out FD
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
            int inFD = 0; 
            for (int i = 0; i < numOfCommands; i++) {
                int fd[2];
                if (i < numOfCommands - 1) { // Pipes = commands-1 (1 pipe for 2 commands, 2 for 3 etc..)
                    if (pipe(fd) != 0) {
                        // Fail check
                        perror("Pipe Failure");
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
                    close(fd[1]); // We do not need the input anymore, close.
                }

                if (inFD != STDIN_FILENO) {
                    close(inFD); // Close the previous read of the pipe.
                }
                inFD = fd[0]; // Read from pipe.
                // Free tokens just in case.
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
    // cat ExtendedShell.c | wc > newfile.txt 
    // wc < newfile.txt | cat & 
    // wc < newfile.txt | cat
    // wc < newfile.txt | cat | cat > finalfile.txt

// "cmd1 < inputFile.txt | cmd2 | cmd3 arg1 arg2 &"

// ls 
// ls -l 

// cat 
    // on its own 
    // cat finalfile.txt | wc 
    // cat tempfile.txt | wc > FILEOUT &

// wc < finalfile.txt | cat | cat > finalFILE_new

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
    // wc < filelist.txt
    // ls > filelist.txt
    // sort < filelist.txt > output.txt
    // cat < filelist.txt | grep "keyword"