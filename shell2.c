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
#define BUFF 10000

#define DEBUG 1  /* In case you want debug messages */

void error(char *s);

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
                if(count == 0) {
                    if (execvp(args[0], args)) {
                        puts(strerror(errno));
                        exit(127);
                    }
                }
                // Character buffer input
                char buf[BUFF];
                buf[0] = '\0';

                // Pid for grandchild
                int childPid;

                // Pipe declarations
                //int in[count + 1][2];
                //int out[count + 1][2];
                int link[count + 1][2];

                // Create pipes
                for(int i = 0; i <= count; i++) {
                    //if(pipe(in[i]) < 0) error("pipe in");
                    //if(pipe(out[i]) < 0) error("pipe out");
                    if(pipe(link[i]) < 0) error("link pipe");
                }

                // Enter a for loop to execute each pipe command
                for (int i = 0; i <= count; i++) {

                    // Argument holder declarations
                    char *toExecute[ARR_SIZE];
                    size_t nArgsExecute;

                    // Split the args into two args by the first encountered |
                    splitter(args, toExecute, args, ARR_SIZE, &num_args, &nArgsExecute, &num_args);

                    childPid = fork();
                    if (childPid == -1) {
                        printf("grandchild fork error\n");
                        exit(1);
                    } else if (childPid == 0) {
                        // Grand child
                        if(i == 0) {
                            // First grandchild
#ifdef DEBUG
                            printf("Grandchild %i makes link[1][1] new stdout, stderr\n", i);
                            printf("Grandchild %i executes %s\n", i, toExecute[0]);
#endif
                            // Close stdin, stdout, stderr
                            close(0);   // Close stdin
                            close(1);   // Close stdout
                            close(2);   // Close stderr

                            // Make pipes new stdin, stdout, stderr
                            dup2(link[0][0], 0);    // Link 0 is reading from parent
                            dup2(link[1][1], 1);    // Link 1 is directing stdout to next grandchild
                            dup2(link[1][1], 2);    // Link 1 is directing stderr to next grandchild

                            // Close links parent will use
                            close(link[0][1]);
                        } else  if(count == i) {
                            // Last grandchild outputs final response to stdout.
                            // Don't close stdout or stderr on the last one.
#ifdef DEBUG
                            printf("Grandchild %i makes link[%i][0] new stdin \n", i, i);
                            printf("Grandchild %i executes %s\n", i, toExecute[0]);
#endif
                            // Close stdin
                            close(0);   // Close stdin

                            // Make pipes new stdin
                            dup2(link[i][0], 0);

                            // Close links parent will use
                            close(link[i][1]);
                        } else {
                            // Grandchildren between first and last
#ifdef DEBUG
                            printf("Grandchild %i makes link[%i][0] new stdin \n", i, i);
                            printf("Grandchild %i executes %s\n", i, toExecute[0]);
#endif
                            // Close stdin, stdout, stderr
                            close(0);   // Close stdin
                            close(1);   // Close stdout
                            close(2);   // Close stderr

                            // Make pipes new stdin, stdout, stderr
                            dup2(link[i - 1][0], 0);    // Link i - 1 is reading from parent
                            dup2(link[i][1], 1);        // Link i is directing stdout to next grandchild
                            dup2(link[i][1], 2);        // Link i is directing stderr to next grandchild

                            // Close links parent will use
                            close(link[i][1]);
                        }

                        // Execute the command with execvp
                        if (execvp(toExecute[0], toExecute)) {
                            puts(strerror(errno));
                            exit(127);
                        }

                    } else {
                        // Parent
                        if(count == i) {
                            close(link[i][1]);
                        } else {
                            close(link[i][0]);
                            close(link[i][1]);
                        }
                    }
                }
                // After for loop print what was returned by the last grandchild
                usleep(10000);
                printf("for loop finished, count = %i\n",count);
                exit(0);
            }
        }
    }
    return 0;
}


