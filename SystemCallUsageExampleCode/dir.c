#include <stdio.h>

int main()
{
    char cmd[1024];
    getcwd(cmd, 1024);  /* fetches the current working directory - equivalent to a pwd command on your Unix/Linux commandline */
    printf("Current working directory is %s\n", cmd);
    return 0;
}
