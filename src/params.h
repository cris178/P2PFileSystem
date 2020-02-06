

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
// This will give you some extra functionality that exists on most recent UNIX/BSD/Linux systems, but probably doesn't exist on other systems such as Windows.
#define _XOPEN_SOURCE 500 //https://stackoverflow.com/questions/5378778/what-does-d-xopen-source-do-mean

// maintain bbfs state in here
#include <limits.h>
#include <stdio.h>

//We will maintain the state of the filesystem here even after closure.
struct p2pState
{
    //FILE *logfile; //only if we want to log what happens.
    char *rootDir;
};
#define P2PDATA ((struct p2pState *)fuse_get_context()->private_data)

#endif
