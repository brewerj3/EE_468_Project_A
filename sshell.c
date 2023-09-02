/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  But it doesn't allow misdirection, e.g., <, >, >>, or |
 *  The project is to fix this.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>


#define BUFFER_SIZE 80
#define ARR_SIZE 80

//#define DEBUG 1  /* In case you want debug messages */

// This function will take in an array of char pointers and split it into two when it finds a |
void splitter(char **argToSplit, char **splitOne, char **splitTwo, size_t args_size, const size_t *nargsSplit, size_t *nargsOne, size_t *nargsTwo) {
    // splitOne is an array of char pointers the function caller will execute
    // splitTwo is an array to be passed to the child
    size_t i = 0;

    for (; i < *nargsSplit; i++) {
        if (!strcmp(argToSplit[i], "|")) {
            break;
        } else {
            splitOne[i] = argToSplit[i];
        }
    }
    splitOne[i + 1] = NULL;
    *nargsOne = i;
    i++;    // Increment by one to skip |
    size_t count = 0;
    for (; i < *nargsSplit; i++) {
        splitTwo[i] = argToSplit[i];
        count++;
    }
    splitTwo[i + 1] = NULL;
    *nargsTwo = count;
}

void parse_args(char *buffer, char **args, size_t args_size, size_t *nargs) {
    char *buf_args[args_size];

    char *wbuf = buffer;
    buf_args[0] = buffer;
    args[0] = buffer;  /* First argument */
/*
 *  The following replaces delimiting characters with '\0'.
 *  Example:  " Aloha World\n" becomes "\0Aloha\0World\0\0"
 *  Note that the for-loop stops when it finds a '\0' or it
 *  reaches the end of the buffer.
 */
    for (char **cp = buf_args; (*cp = strsep(&wbuf, " \n\t")) != NULL;) {
        if ((**cp != '\0') && (++cp >= &buf_args[args_size]))
            break;
    }

/* Copy 'buf_args' into 'args' */
    size_t j = 0;
    for (size_t i = 0; buf_args[i] != NULL; i++) {
        if (strlen(buf_args[i]) > 0)  /* Store only non-empty tokens */
            args[j++] = buf_args[i];
    }

    *nargs = j;
    args[j] = NULL;
}

int main(int argc, char *argv[], char *envp[]) {
    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];

    size_t num_args;
    pid_t pid;

    while (1) {
        printf("ee468>> ");
        fgets(buffer, BUFFER_SIZE, stdin); /* Read in command line */
        parse_args(buffer, args, ARR_SIZE, &num_args);

        if (num_args > 0) {
#ifdef DEBUG
            printf("number of arguments = %zu\n", num_args);
            for(int i = 0; i < num_args; i++) {
                printf("Argument %2i = %s\n", i + 1, args[i]);
            }
#endif
            if (!strcmp(args[0], "exit")) exit(0);  // Ends the program
            int count = 0;
            for (int i = 0; i < num_args; i++) {
                if (!strcmp(args[i], "|")) {
                    count++;
                }
            }
            if (count > 4) {
                printf("To many | arguments. Maximum is 4, you entered %i\n", count);
                continue;
            }
#ifdef DEBUG
            printf("number of pipe | arguments = %i\n", count);
#endif

            pid = fork();                           // Forks the program once
            if (pid) {  /* Parent */

#ifdef DEBUG
                printf("Waiting for child (%d)\n", pid);
#endif
                pid = wait(NULL);                   // Wait for the child to complete
#ifdef DEBUG
                printf("Child (%d) finished\n", pid);
#endif
            } else {  /* Child executing the command */
                // Determine if another fork is needed
                if (count > 0) { // The args must be split and another fork created
                    --count;
                    char *argsToExecuteOne[ARR_SIZE];
                    size_t numArgsExecute;
                    char *argsToPassOnOne[ARR_SIZE];
                    size_t numArgsPass;

                    splitter(args, argsToExecuteOne, argsToPassOnOne, ARR_SIZE, &num_args, &numArgsExecute,
                             &numArgsPass);

                    // Set up pipes
                    int pipeInOne[2], pipeOutOne[2];
                    if(pipe(pipeInOne) < 0) error()
                    int childpid;
                    childpid = fork();
                    if (childpid == -1) {
                        printf("Child fork failed\n");
                        exit(1);
                    } else if (childpid == 0) {  // Child

                    } else {    // Parent
                        // Execute argsToExecuteOne
                    }
                } else if (execvp(args[0], args)) {
                    puts(strerror(errno));
                    exit(127);
                }
            }

        }
    }
    return 0;
}


