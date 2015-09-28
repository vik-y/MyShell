#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <readline/readline.h>

/* If you understood the program dup.c, this should be easy for you */

int main()
{
    char *s;
    FILE *infile, *outfile;
    pid_t pid;
    
    char cmd1[20]={"cat"};
    char *args1[]={"cat", "1.txt",NULL};
    char cmd2[20]={"echo"};
    char *args2[]={"echo", "1.txt",NULL};

    pid=fork();
	if(0==pid)
	{
    	//infile = fopen("1.txt", "r");
    	outfile = fopen("2.txt", "w");
    	//dup2(fileno(infile), 0);
    	dup2(fileno(outfile), 1);
		execvp(cmd1, args1);		
    }
    else
    {
    	outfile = fopen("3.txt", "w");
    	//dup2(fileno(infile), 0);
    	dup2(fileno(outfile), 1);
		execvp(cmd2, args2);		
    	printf("DONE\n");
    }
    return 0;
}
