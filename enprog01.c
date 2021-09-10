// https://stackoverflow.com/questions/61174235/implementation-of-wc-command-from-shell-in-c-language

#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    int sumL=0,sumW=0,sumB=0,index=0;
    char buffer[1];
    enum states { WHITESPACE, WORD };

    if ( argc==1 )
    {
        printf( "Nu ati introdu snumele  fisierului\n%s", argv[0]);
    }
    else
    {
        while(--argc>0)
        {
            int bytes = 0;
            int words = 0;
            int newLine = 0;
            int state = WHITESPACE;
            FILE *file = fopen( *++argv, "r");

            if(file == 0)
            {
                printf("can not find :%s\n",argv[1]);
            }
            else
            {
                char *thefile = *argv;

                while (read(fileno(file),buffer,1) ==1 )
                {
                    bytes++;
                    if ( buffer[0]== ' ' || buffer[0] == '\t'  )
                    {
                        state = WHITESPACE;
                    }
                    else if (buffer[0]=='\n')
                    {
                        newLine++;
                        state = WHITESPACE;
                    }
                    else
                    {
                        if ( state == WHITESPACE )
                        {
                            words++;
                        }
                        state = WORD;
                    }

                }
                printf(" %d %d %d %s\n",newLine,words,bytes,thefile);
                sumL+=newLine;
                sumW+=words;
                sumB+=bytes;
                index++;
            }
        }
        if(index>1)
            printf(" %d %d %d total \n",sumL,sumW,sumB);
    }
    return 0;
}