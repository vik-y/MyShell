#include <stdio.h>
#include <sys/types.h>

int main()
{
    pid_t pid;
    pid = fork();	
    if(0 == pid )
    {
        printf("I am the child process\n");
    	while(1);
	        
    }
    else
    {
        printf("I am the parent processor\n");
    	while(1);
    }

    return 0;
}
