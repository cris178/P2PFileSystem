

//The function of getattr event will be called when the system tries to get the attributes of the file.
//It's similar to stat function on Linux terminals


#include "params.h" //

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>	//http://man7.org/linux/man-pages/man2/getuid.2.html also for lstat() https://linux.die.net/man/2/lstat
#include <sys/types.h> //This and above are for getting uid(userid) and gid(groupid),and used with dirent.h
#include <time.h>	  //Used to get acess time of files and modification times
#include <string.h>
#include <stdlib.h>
#include <limits.h> //PATH_MAX comes form this library
#include <dirent.h> //man opendir/readdir used for DIR *directorypointer easy way to open directories.
#include <errno.h>  //This is for error messages https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=vs-2019

//Conn contains issues about the connections like the sockets
//Really not necesssary to build this function.
/*
Initialize the filesystem. This function can often be left unimplemented, 
but it can be a handy way to perform one-time setup such as allocating 
variable-sized data structures or initializing a new filesystem. 
The fuse_conn_info structure gives information about what features 
are supported by FUSE, and can be used to request certain capabilities 
(see below for more information). The return value of this function is 
available to all file operations in the private_data field of fuse_context. 
It is also passed as a parameter to the destroy() method.
https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201001/homework/fuse/fuse_doc.html#function-purposes
*/

void *p2pInit(struct fuse_conn_info *conn)
{
	//log_msg("\nbb_init()\n");

	//log_conn(conn);
	//log_fuse_context(fuse_get_context());
	printf("p2pInit: Initializing Data\n");
	printf("p2pInit: Running fuse_get_context()\n");
	struct fuse_context *context = fuse_get_context();
	printf("p2pInit: From Context Retrieving uid, gid, pid. %s %s %s\n", context->uid, context->gid, context->pid);
	return P2PDATA;
}

/**********************
 * Get the Root Directory we
 * saved in params.h and 
 * concatenate it with the 
 * local path provided in each function
 * ************************/
static void p2pFullPath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, P2PDATA->rootDir);
	printf("p2pFullPath: Placing saved RootDir in fPath\n%s\n",fpath );
	printf("p2pFullPath: Local Path is %s\n", path);
	
	strncat(fpath, path,PATH_MAX); // If path is huge may surpass PATH_MAX and break
	printf("p2pFullPath: fpath now concatenated with new local path \n%s\n", fpath);
	//P2PDATA->rootDir = fpath;

	return;
}



//Get Attribute gets info on directory and files
//Returns 0 if fine, -1 and error code unique if something is wrong.
int p2pGetAttr(const char *path, struct stat *stats) //stats is a buffer
{
	printf("p2pGetAttr: Entering getAttr\n");
	printf("p2oGetAttr: Relative Path is %s\n", path);
	
	int returnStatus;
	char filePath[PATH_MAX];
	p2pFullPath(filePath, path); //Will print the completed path name.
	printf("p2pGetAttr: Returned with full path\n%s\n", filePath);
	returnStatus = lstat(filePath, stats);
	printf("This is what lstat(Returns) should be 0 or > if path set up the structure if < 0 return status I should setup -errno: %i\n", returnStatus);
	if (returnStatus < 0)
	{
		returnStatus = -errno;
	}
	printf("p2pGetAttr: Exiting with Return Status %i\n", returnStatus);
	printf("p2pGetAttr: Stats Structure Mode = %o\n", stats->st_mode);
	printf("p2pGetAttr: Stats Structure UID = %i\n", stats->st_uid);

	//returnStatus = p2pSysCall("lstat", lstat(filePath, stats), 0);
	//printf("We should be returning a 0 or 1 in getAttri %i", returnStatus);
	return returnStatus;
}

//Only need first 3 params - Path of dir  - buffer - filler (filler comes from fuse.h)
//Returns one or more directory entries. Use struct dirent.
//Starts at a given offset not a byte offset
static int p2pReadDir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) //fuse fill dir t  Function to add an entry in a readdir() operation https://libfuse.github.io/doxygen/fuse_8h.html
{
	//1) In order to read the directory we will need to gather the information
	//2) We will find saved file descriptors in fi->fh
	//3) We use a pointer to the descriptor to read the directory and return a dirent struct pointer
	//4) With our new Directory Entry pointer
	printf("p2pReadDir: Enter p2pReadDir");
	
	int returnStatus;
	DIR *directoryPointer;		   //man opendir
	struct dirent *directoryEntry; //man readdir

	//log_msg("\nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",path, buf, filler, offset, fi);
	printf("\np2pReadDir path=\"%s\",buffer=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n", path, buffer, filler, offset, fi);

	// Don't need full path for this function but we do need to cast fi->fh
	directoryPointer = (DIR *)(uintptr_t)fi->fh; //Type casting fi= file information we do this to use readdir() later on
												//Our file descriptors are in fh

	// Every directory contains at least two entries: . and ..  If my
	// first call to the system readdir() returns NULL I've got an
	// error; near as I can tell, that's the only condition under
	// which I can get an error from readdir()
	directoryEntry = readdir(directoryPointer); //man readdir useful thanks to dirent.h
												//Will return a pointer to dirent structure
												//man readdir 3
	

	printf("Using readdir() function...\n readdir returned 0x%p\n", directoryEntry);

	if (directoryEntry == 0) //If first call to readdir returns null we have an error
	{
		returnStatus = -errno;
		return returnStatus;
	}

	// This will copy the entire directory into the buffer.  The loop exits
	// when either the system readdir() returns NULL, or filler()
	// returns something non-zero.  The first case just means I've
	// read the whole directory; the second means the buffer is full.
	do
	{
		printf("p2pReadDir: In loop displaying directory entry's %s\n", directoryEntry->d_name);
		if (filler(buffer, directoryEntry->d_name, NULL, 0) != 0) //directoryEntry has name field   https://stackoverflow.com/questions/12991334/members-of-dirent-structure
		{														  //fuse filler (void *buf, const char *name,const struct stat *stbuf, off_t off);
			//log_msg("    ERROR bb_readdir filler:  buffer full");
			printf("p2pReadDir: filler buffer full not enough memory ERROR");
			return -ENOMEM; //Not enough core. Not enough memory is available for the attempted operator. https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=vs-2019
		}
	} while ((directoryEntry = readdir(directoryPointer)) != NULL); //Read until readdir returns null

	//log_fi(fi);

	return returnStatus;

	

}

//Do next
//Read sizebytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. 
//Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. 
//Required for any sensible filesystem.
//https://linux.die.net/man/2/pread
int p2pRead(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)//From fi we can get the descriptor fh and use pread 
{
	int returnStatus = 0;
	printf("p2pRead: Entering p2pRead, this function takes the file descriptor fi->fh and reads files until EoF\n");
	
	returnStatus = pread(fi->fh, buffer,size,offset); //On success, the number of bytes read or written is 
													  //returned (zero indicates that nothing was written, 
													  //in the case of pwrite(), or end of file, in the 
													  //case of pread()), or -1 on error, in which case errno is set to indicate the error.
													  //https://linux.die.net/man/2/pread
	printf("Read file descriptor and if success will return 0 at EoF.");
	if(returnStatus < 0){
		returnStatus = -errno;
	}
	return returnStatus;
}

/********************
		p2pOpen
This is how filesystem opens files.
Checks if has permission to create files.
Can pass file handle to fuse_file_info structure
which will then be passed to all of the file handling
functions/
Takes in path and file info from fuse in file info pointer
********************/

int p2pOpen(const char *path, struct fuse_file_info *fi) //from the fuse_file info structure we get the permission flags
{
	int returnStatus = 0;
	int fileDescriptor;
	char filePath[PATH_MAX]; //Path MAX(maximum string length of path provided by linux) https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux

	//log_msg("\nbb_open(path\"%s\", fi=0x%08x)\n", path, fi); logging this to a file.
	//printf("\np2pOpen: Opening File in Path \"%s\", with fi=0x%08x\n", path, fi);
	p2pFullPath(filePath, path); //returns the fullpath concatenated

	// if the open call succeeds, my returnStatus is the file descriptor,
	// else it's -errno.  I'm making sure that in that case the saved
	// file descriptor is exactly -1.
	//fileDescriptor = p2pSysCall("open", open(filePath, fi->flags), 0);
	
	fileDescriptor = open(filePath,fi->flags);	//Open will return a file descriptor, the smallest non negative number not yet assigned
												//This creates an entry on the system wide table of open files
												//The table holds the file offset and the file status flags modifiable  bby fcntl see man page
												//https://linux.die.net/man/2/open
	if (fileDescriptor < 0)
	{
		//returnStatus = p2pError("open");
		returnStatus = -errno;
	}
	fi->fh = fileDescriptor; //Store fileDescriptor for later use
							 //"File handle id. May be filled in by filesystem in create, open, and opendir().
							 // Available in most other file operations on the same file handle.
							 //"https://libfuse.github.io/doxygen/structfuse__file__info.html#a45314d0b92a8d4c9de33d996aa59ada8

	//log_fi(fi);

	return returnStatus;
}

static struct fuse_operations operations = {
	.getattr = p2pGetAttr,
	.readdir = p2pReadDir,
	.read = p2pRead,
	.open = p2pOpen,
	.init = p2pInit,
};

int main(int argc, char *argv[])
{

	struct p2pState *p2pData; //We need to keep track of the state of our file system. Will save path here.

	//Check the provided arguements in command line argc = number of arguements
	//argv is the array of provided arguements.
	if (argc < 1)
	{
		fprintf(stderr, "please provide a mounting point\n");
		abort(); //End program
	}

	//Don't allow user to run as Root. Major security issues.
	if ((getuid() == 0) || (geteuid() == 0))
	{
		fprintf(stderr, "Can't Run Fuse File System as Root User! Security issues.\n");
		return 1; //End Program
	}

	printf("Starting up FileSystem...\n");
	p2pData = malloc(sizeof(struct p2pState));
	if (p2pData == NULL)
	{
		perror("main calloc");
		abort();
	}
	//Need to save some data
	//For performance reasons instead of getting the path every single time we can construct it using a saved Root
	//-f is counted
	printf("Arguements Count is %d\n", argc);
	printf("Aguement Values are... %s %s %s\n", argv[0], argv[1], argv[2], argv[4]);

	//Need root directory which is index 2 or (3-1)
	p2pData->rootDir = realpath(argv[argc - 1], NULL); //0 is instructions, 1 is -f, 2 is mountdir total 3
	printf("Ceated Pointer to Path of Root Directory: %s\n", p2pData->rootDir);

	//-s single thread -f debug

	//Check Fuse Version we are runnign
	fprintf(stderr, "Checking Fuse Version... %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
	return fuse_main(argc, argv, &operations, p2pData);
}