#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int childstatus;
    pid_t pid;
    pid = fork();	
    if(0 == pid )
    {
        getchar();
        printf("I am the child process\n");
        
    }
    else
    {
	    wait(&childstatus);
        printf("I am the parent processor\n");
    }

    return 0;
}
