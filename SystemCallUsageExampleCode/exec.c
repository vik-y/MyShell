#include <unistd.h>

int main()
{
    char cmd[20]={"/bin/ls"};
    char *args[]={"/home/csguest","-al", (char*)0 };
    execvp(cmd, args);
    return 0;
}
