#include <stdio.h>
#include <sys/types.h> //This and above are for getting uid(userid) and gid(groupid)
#include <time.h>      //Used to get acess time of files and modification times
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    printf("ArgC count: %d\n", argc);

    int i = 0;

    for (i = 0; i < argc; i++)
    {
        printf("ArgV Value: %i "
               "%s\n",
               i, argv[i]);
        char *rootdir = realpath(argv[1], NULL); // We get /
        //printf("\n\n%c", *rootdir);
        //printf("\n\n%c", *rootdir);
        printf("First Spot %c\n", *rootdir);
    }

    printf("%s\n", realpath(argv[argc - 3], NULL)); //Finds a file or directory and returns its path if not file or directory will seg fault
    printf("We can see above that we can't ever get n so go one under");

    printf("Hello World ");
    return 0;
}