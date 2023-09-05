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
#define BUFF 1000

#define DEBUG 1  /* In case you want debug messages */

void error(char *s);

// Split character array into two character arrays
void toSplit(char **argToSplit, char **splitArgs, size_t args_size, size_t *nargsToSplit, size_t *splitNArgs) {

}

// This function will take in an array of char pointers and split it into two when it finds a |
void splitter(char **argToSplit, char **splitOne, char **splitTwo, size_t args_size, const size_t *nargsToSplit,
              size_t *nargsOne, size_t *nargsTwo) {
    // splitOne is an array of char pointers the function caller will execute
    // splitTwo is an array to be passed to the child
    size_t i = 0;

    for (; i < *nargsToSplit; i++) {
        if (!strcmp(argToSplit[i], "|")) {
            break;
        } else {
            splitOne[i] = argToSplit[i];
        }
    }
    *nargsOne = i;
    // Increment by one to skip |
    splitOne[++i] = NULL;
    size_t j = 0;
    for (; j + i < *nargsToSplit; j++) {
        splitTwo[j] = argToSplit[i + j];
    }
    *nargsTwo = j;
    splitTwo[++j] = NULL;
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
            for (int i = 0; i < num_args; i++) {
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
                // Character buffer input
                char buf[BUFF];
                buf[0] = '\0';

                    // Enter a for loop to execute each pipe sequentially
                    for(int i = 0; i < count; i++) {
                        // Pipe declarations
                        int in[2], out[2], childPid;

                        // Create pipes
                        if (pipe(in) < 0) error("pipe in");
                        if (pipe(out) < 0) error("pipe out");

                        // Argument holder declarations
                        char *toExecute[ARR_SIZE];
                        size_t nArgsExecute;

                        // Split the args into two args by the first encountered |
                        splitter(args, toExecute, args, ARR_SIZE, &num_args, &nArgsExecute, &num_args);

                        childPid = fork();
                        if(childPid == -1) {
                            printf("grandchild for error\n");
                        }else if(childPid == 0) {
                            // Child

#ifdef DEBUG
// Add debug output here
                            printf("Command Grandchild is executing = %s\n", toExecute[0]);
#endif

                            // Close stdin, stdout, sterr
                            close(0);
                            close(1);
                            close(2);

                            // Make pipes new stdin, stdout, and stderr
                            dup2(in[0], 0);
                            dup2(out[1], 1);
                            dup2(out[1], 2);

                            // Close ends of pipe that parent will use
                            close(in[1]);
                            close(out[0]);

                            // Execute the command with execvp
                            if(execvp(toExecute[0], toExecute)) {
                                puts(strerror(errno));
                                exit(127);
                            }
                            //exit(1);
                        } else {
                            // Parent


                            // Close the pipe ends the child used
                            close(in[0]);
                            close(out[1]);

                            usleep(1000);
                            // Write data to the child as input for command
                            if(buf[0] != '\0') {
                                write(in[1], buf, strlen(buf));
                            }
                            // Close the pipe so the child does not block execution
                            // This sends EOF to the child on it's stdin
                            close(in[1]);

                            int n = read(out[0], buf, BUFF - 1);
                            buf[n] = 0;
                            close(out[0]);
                            childPid = wait(NULL);
                        }
                    }
                    printf("%s\n",buf);
                    exit(0);

                /* Deprecated
                // Determine if another fork is needed
                if (count > 0) { // The args must be split and another fork created
#ifdef DEBUG
                    printf("grandchild required, count = %i\n", count);
#endif
                    --count;
                    char *argsToExecuteOne[ARR_SIZE];
                    size_t numArgsExecute;
                    char *argsToPassOnOne[ARR_SIZE];
                    size_t numArgsPass;

                    splitter(args, argsToExecuteOne, argsToPassOnOne, ARR_SIZE, &num_args, &numArgsExecute,
                             &numArgsPass);


#ifdef DEBUG
                    printf("Grandchild argsToPassOn = %s\n", argsToPassOnOne[0]);
                    printf("Grandchild number of arguments = %zu\n", numArgsPass);
                    for (int i = 0; i < numArgsPass; i++) {
                        printf(" grandchild Argument %2i = %s\n", i + 1, argsToPassOnOne[i]);
                    }
                    printf("Parent argsToExecute = %s\n", argsToExecuteOne[0]);

                    printf("Parent number of arguments = %zu\n", numArgsExecute);
                    for (int i = 0; i < numArgsExecute; i++) {
                        printf(" Parent Argument %2i = %s\n", i + 1, argsToExecuteOne[i]);
                    }
#endif
                    // Set up pipes
                    int pipeInOne[2], pipeOutOne[2];
                    if (pipe(pipeInOne) < 0) error("Pipe in");
                    if (pipe(pipeOutOne) < 0) error("Pipe out");

                    int childpid;
                    childpid = fork();
                    if (childpid == -1) {
                        printf("Child fork failed\n");
                        exit(1);
                    } else if (childpid == 0) {  // Child

                        // Close stdin, stdout, stderr
                        close(0);
                        close(1);
                        close(2);
                        // Make our pipes our new stdin, stdout, and stderr
                        dup2(pipeInOne[0], 0);
                        dup2(pipeOutOne[1], 1);
                        dup2(pipeOutOne[1], 2);
                        // close other end of pipe that parent will use
                        close(pipeInOne[1]);
                        close(pipeOutOne[0]);
                        // execute argument passed to child

                        if (execvp(argsToPassOnOne[0], argsToPassOnOne)) {
                            puts(strerror(errno));
                            exit(127);
                        }
                    } else {    // Parent
                        // Execute argsToExecuteOne
                        // Close pipe ends child would use
                        close(pipeInOne[0]);
                        close(pipeOutOne[1]);

                        // Close stdout then replace with our pipes
                        close(1);
                        // Replace stdout with pipe
                        dup2(pipeInOne[1], 1);

                        if (execvp(argsToExecuteOne[0], argsToExecuteOne)) {
                            puts(strerror(errno));
                            exit(127);
                        }
                    }
                } else
                 */
            }
        }
    }
    return 0;
}


