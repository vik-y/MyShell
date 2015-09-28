#include <stdio.h>

int main()
{
    char c;
    FILE *infile, *outfile;
    infile = fopen("1.txt", "r");
    outfile = fopen("2.txt", "w");
    while(!feof(infile))
    {
        c=fgetc(infile);
        printf("%c", c);
        fputc(c, outfile);
    }

    fclose(infile);
    fclose(outfile);
    return 0;
}
