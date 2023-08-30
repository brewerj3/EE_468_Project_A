#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pid;
    pid = fork();
    if(pid) {
        printf("Parent pid = %i\n", pid);
        pid = wait(NULL);
        printf("Parent pid after wait = %i\n",pid);
    } else {
        printf("child pid before fork = %i\n", pid);
        int childpid = fork();
        if(childpid) {
            printf("child pid after fork = %i\n", childpid);
        } else {
            printf("grandchild pid after fork = %i\n", childpid);
        }
    }
    return 0;
}