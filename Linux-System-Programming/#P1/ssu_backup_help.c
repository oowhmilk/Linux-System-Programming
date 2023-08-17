#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** tokens)
{
    printf("Usage:\n\t> add [FILENAME] [OPTION]\n\t\t-d : add directory recursive\n\t> remove [FILENAME] [OPTION]\n\t\t-a : remove all file(recursive)\n\t\t-c : clear backup directory\n\t> recover [FILENAME] [OPTION]\n\t\t-d : recover directory recursive\n\t\t-n [NEWNAME] : recover file with new name\n\t> ls\n\t> vi\n\t> vim\n\t> help\n\t> exit\n");

    return 0;
}