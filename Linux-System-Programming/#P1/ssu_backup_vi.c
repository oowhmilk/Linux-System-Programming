#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** tokens)
{
    
    if(tokens[1] != NULL)
    {
        execl("/usr/bin/vi", "vi", tokens[1], NULL);
        perror("exec error");
        exit(0);
    }
    else
    {
        execl("/usr/bin/vi", "vi", NULL);
        perror("exec error");
        exit(0);
    }

    return 0;
}