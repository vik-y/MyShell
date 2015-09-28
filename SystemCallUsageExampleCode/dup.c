#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>

int main()
{
    char *s;
    FILE *infile, *outfile;
    infile = fopen("1.txt", "r");
    outfile = fopen("2.txt", "w");
    dup2(fileno(infile), 0);    /* makes file 1.txt as your standard input */
    dup2(fileno(outfile), 1);   /* makes file 2.txt as your standard output - all the following printfs will not be shown on your display, but go to 2.txt */
    s = readline("csguest:-> ");
    printf("The command is %s\n", s);
    return 0;
}
