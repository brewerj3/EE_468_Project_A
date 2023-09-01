/*
 * This is an example program that was copied from the web site
 * www.stackflow.com.  It shows how to call a program from within
 * a C program using "execl", which is a variation of exec.
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *s);

char *data = "Some input data\n";

main() {
    int in[2], out[2], n, pid;
    char buf[255];

    /* Creating two pipes: 'in' and 'out' */
    /* In a pipe, xx[0] is for reading, xx[1] is for writing */
    if (pipe(in) < 0) error("pipe in");
    if (pipe(out) < 0) error("pipe out");

    if ((pid = fork()) == 0) {
        /* This is the child process */

        /* Close stdin, stdout, stderr */
        close(0);
        close(1);
        close(2);
        /* make our pipes, our new stdin,stdout and stderr */
        dup2(in[0], 0);
        dup2(out[1], 1);
        dup2(out[1], 2);

        /* Close the other ends of the pipes that the parent will use, because if
         * we leave these open in the child, the child/parent will not get an EOF
         * when the parent/child closes their end of the pipe.
         */
        close(in[1]);
        close(out[0]);

        /* Over-write the child process with the hexdump binary.
         * The zeroth argument is the path to the program 'hexdump'
         * The second argument is the name of the program to be run, which
         * is 'hexdump'.  This is a bit redundant to the zeroth argument,
         * but it's how it works.  The third argument '-C' is one of the
         * options of hexdump.  The fourth argument is a terminating
         * symbol -- a NULL pointer -- to indicate the end of the arguments.
         * In general, execl accepts an arbitrary number of arguments.
         */
        execl("/usr/bin/hexdump", "hexdump", "-C", (char *) NULL);

        /* If hexdump wasn't executed then we would still have the following
         * function, which would indicate an error
         */
        error("Could not exec hexdump");
    }

    /*  The following is in the parent process */
    printf("Spawned 'hexdump -C' as a child process at pid %d\n", pid);

    /* This is the parent process */
    /* Close the pipe ends that the child uses to read from / write to so
     * the when we close the others, an EOF will be transmitted properly.
     */
    close(in[0]);
    close(out[1]);

    /* The following displays on the console what's in the array 'data'
     * The array was initialized at the top of this program with
     * the string 'Some input data'
     */
//  printf("<- %s", data);  Galen replaced this line with the following
    printf("String sent to child: %s\n\n", data);

    /* From the parent, write some data to the childs input.
     * The child should be 'hexdump', which will process this
     * data.
     */
    write(in[1], data, strlen(data));

    /* Because of the small amount of data, the child may block unless we
     * close its input stream. This sends an EOF to the child on it's
     * stdin.
     */
    close(in[1]);

    /* Read back any output */
    n = read(out[0], buf, 250);
    buf[n] = 0;
//  printf("-> %s",buf);  Galen replaced this line with the following
    printf("This was received from the parent: %s", buf);

    exit(0);
}

void error(char *s) {
    perror(s);
    exit(1);
}

