#include <stdio.h>
#include <sys/types.h>

int main()
{
    pid_t pid;
    pid = getpid();
    printf("current running process id is %d\n", pid);
    while(1);
    return 0;
}
