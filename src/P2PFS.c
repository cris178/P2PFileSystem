
/*
Fuse Functions
struct fuse_operations {
	int (*getattr) (const char *, struct stat *);
	int (*readlink) (const char *, char *, size_t);
	int (*getdir) (const char *, fuse_dirh_t, fuse_dirfil_t);
	int (*mknod) (const char *, mode_t, dev_t);
	int (*mkdir) (const char *, mode_t);
	int (*unlink) (const char *);
	int (*rmdir) (const char *);
	int (*symlink) (const char *, const char *);
	int (*rename) (const char *, const char *);
	int (*link) (const char *, const char *);
	int (*chmod) (const char *, mode_t);
	int (*chown) (const char *, uid_t, gid_t);
	int (*truncate) (const char *, off_t);
	int (*utime) (const char *, struct utimbuf *);
	int (*open) (const char *, struct fuse_file_info *);
	int (*read) (const char *, char *, size_t, off_t,
		     struct fuse_file_info *);
	int (*write) (const char *, const char *, size_t, off_t,
		      struct fuse_file_info *);
	int (*statfs) (const char *, struct statvfs *);
	int (*flush) (const char *, struct fuse_file_info *);
	int (*release) (const char *, struct fuse_file_info *);
	int (*fsync) (const char *, int, struct fuse_file_info *);
	int (*setxattr) (const char *, const char *, const char *, size_t, int);
	int (*getxattr) (const char *, const char *, char *, size_t);
	int (*listxattr) (const char *, char *, size_t);
	int (*removexattr) (const char *, const char *);
	int (*opendir) (const char *, struct fuse_file_info *);
	int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t,
			struct fuse_file_info *);
	int (*releasedir) (const char *, struct fuse_file_info *);
	int (*fsyncdir) (const char *, int, struct fuse_file_info *);
	void *(*init) (struct fuse_conn_info *conn);
	void (*destroy) (void *);
	int (*access) (const char *, int);
	int (*create) (const char *, mode_t, struct fuse_file_info *);
	int (*ftruncate) (const char *, off_t, struct fuse_file_info *);
	int (*fgetattr) (const char *, struct stat *, struct fuse_file_info *);
	int (*lock) (const char *, struct fuse_file_info *, int cmd,
		     struct flock *);
	int (*utimens) (const char *, const struct timespec tv[2]);
	int (*bmap) (const char *, size_t blocksize, uint64_t *idx);
	int (*ioctl) (const char *, int cmd, void *arg,
		      struct fuse_file_info *, unsigned int flags, void *data);
	int (*poll) (const char *, struct fuse_file_info *,
		     struct fuse_pollhandle *ph, unsigned *reventsp);
	int (*write_buf) (const char *, struct fuse_bufvec *buf, off_t off,
			  struct fuse_file_info *);
	int (*read_buf) (const char *, struct fuse_bufvec **bufp,
			 size_t size, off_t off, struct fuse_file_info *);
	int (*flock) (const char *, struct fuse_file_info *, int op);
	int (*fallocate) (const char *, int, off_t, off_t,
			  struct fuse_file_info *);
};

*/

//The function of getattr event will be called when the system tries to get the attributes of the file.
//It's similar to stat function on Linux terminals

#define FUSE_USE_VERSION 30

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

/**********************
 * Get the Root Directory we
 * saved in params.h and 
 * concatenate it with the 
 * local path provided in each function
 * ************************/
static void p2pFullPath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, P2PDATA->rootDir);
	strncat(fpath, path, PATH_MAX); // If path is huge may surpass PATH_MAX and break
	printf("Assembled Full Path: Root Dir = \"%s\",path = \"%s\", fullpath = \"%s\"\n", P2PDATA->rootDir, path, fpath);
	//log_msg("    p2pFullPath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",BB_DATA->rootdir, path, fpath);
}

/*********************************\
 * 			p2pReturnStatus
 * Checks the Returns Status
 * 
 * *******************************/
void p2pReturnStatus(char *func, int returnStatus)
{
	int errSave = errno;
	//log_msg("    %s returned %d\n", func, returnStatus);
	printf("p2pReturnStatus: Return Status Returned: %s , %d\n", func, returnStatus);
	errno = errSave;
}

// Report errors to logfile and give -errno to caller
int p2pError(char *func)
{
	int ret = -errno;
	//log_msg("    ERROR %s: %s\n", func, strerror(errno));
	printf("p2pError: Error %s: %s\n", func, strerror(errno));
	return ret;
}
/********************************
 * 			p2pSysCall
 * This function makes a system call.
 * Check the returns status and sometimes logging error.
 * 
 * *******************************/
// make a system call, checking (and reporting) return status and
// possibly logging error
int p2pSysCall(char *func, int returnStatus, int min_ret)
{
	p2pReturnStatus(func, returnStatus);

	if (returnStatus < min_ret)
	{
		p2pError(func);
		returnStatus = -errno;
	}

	printf("p2pSysCall: a syscall was made and returning status %i\n", returnStatus);
	return returnStatus;
}

//This gets file attributes
//getattr - similar to stat function in linux
//We will pass in two paramters and retunr an integer
//Param 1: path of file we will get attributes of
//Param 2: stat structure that needs to be that needs to be filled with attributes
//Return 0: if success
//return -1: with errno with correct errorcode
static int p2pGetAttr(const char *path, struct stat *stats) //stats is a buffer
{

	int returnStatus;
	char filePath[PATH_MAX];

	p2pFullPath(filePath, path);
	//printf("\np2pGetAttr: Compiled FilePath \"%s\", stat buffer=0x%08x)\n", path, stats);

	returnStatus = p2pSysCall("lstat", lstat(filePath, stats), 0);

	return returnStatus; //Returns 0 if good

	/******
	stats->st_uid = getuid();	 //stuid is the owner of the file. ->Make this person who mounted the directory.
	stats->st_gid = getgid();	 //owner group of the files or directories/subdirectories. ->Make this person who mounted the directory
	stats->st_atime = time(NULL); //last acess time
	stats->st_mtime = time(NULL); //last modification time
	**/
	/**
	if (strcmp(path, "/") == 0) //Wil run first option if in root
	{
		stats->st_mode = S_IfileDescriptorIR | 0755; //(check if file or dir) |(permission bits)        st_mode shows if is regular file, directior, other and permission bits of that file.
		stats->st_nlink = 2;						 //Shows number of Hardlinks, Hardlinks, like copying a file but not a copy, a link to original file but modifying hardlink modifies original as well.

		//Seeing what S_IfileDescriptorIR retunrs
		//printf("CHECK Testing: \t%i", S_IfileDescriptorIR);
	
	} **/
	//Reason why twp hardlinks  https://unix.stackexchange.com/questions/101515/why-does-a-new-directory-have-a-hard-link-count-of-2-before-anything-is-added-to/101536#101536
	/****else
	{
		stats->st_mode = S_IFREG | 0644;
		stats->st_nlink = 1;
		stats->st_size = 1024; //Size of files in bytes
	}

	return 0;
	******/
}

//Only need first 3 params - Path of dir  - buffer - filler (filler comes from fuse.h)
static int p2pReadDir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) //fuse fill dir t  Function to add an entry in a readdir() operation https://libfuse.github.io/doxygen/fuse_8h.html
{
	int returnStatus;
	DIR *directoryPointer;		   //man opendir
	struct dirent *directoryEntry; //man readdir

	//log_msg("\nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",path, buf, filler, offset, fi);
	printf("\np2pReadDir path=\"%s\",buffer=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n", path, buffer, filler, offset, fi);

	// once again, no need for fullpath -- but note that I need to cast fi->fh
	directoryPointer = (DIR *)(uintptr_t)fi->fh; //Type casting fi= file information we do this to use readdir() later on

	// Every directory contains at least two entries: . and ..  If my
	// first call to the system readdir() returns NULL I've got an
	// error; near as I can tell, that's the only condition under
	// which I can get an error from readdir()
	directoryEntry = readdir(directoryPointer); //man readdir useful thanks to dirent.h
	//log_msg("    readdir returned 0x%p\n", directoryEntry);
	printf("Using readdir() function...\n readdir returned 0x%p\n", directoryEntry);

	if (directoryEntry == 0)
	{
		returnStatus = p2pError("p2pReadDir readdir");
		return returnStatus;
	}

	// This will copy the entire directory into the buffer.  The loop exits
	// when either the system readdir() returns NULL, or filler()
	// returns something non-zero.  The first case just means I've
	// read the whole directory; the second means the buffer is full.
	do
	{
		printf("calling filler with name %s\n", directoryEntry->d_name);
		if (filler(buffer, directoryEntry->d_name, NULL, 0) != 0) //directoryEntry has name field   https://stackoverflow.com/questions/12991334/members-of-dirent-structure
		{
			//log_msg("    ERROR bb_readdir filler:  buffer full");
			printf("ERROR p2pReadDir filler:  buffer full");
			return -ENOMEM; //Not enough core. Not enough memory is available for the attempted operator. https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=vs-2019
		}
	} while ((directoryEntry = readdir(directoryPointer)) != NULL); //Read until readdir returns null

	//log_fi(fi);

	return returnStatus;

	/**********************

	//Path is is the path to current directory
	//Buffer is what we will fill with names of files and directories in current location
	//Filler is a Fuse a function sent by fuse which we will use to fill the buffer using the path

	filler(buffer, ".", NULL, 0);  //Current Directory
	filler(buffer, "..", NULL, 0); //Parent Directory

	//See if valid path
	if (strcmp(path, "/") == 0)
	{
		//Use our Fuse provided filler Functions
		filler(buffer, "file54", NULL, 0);
		filler(buffer, "file349", NULL, 0);
	}

	return 0; //returns 0 on success.

	**********************/
}

//Do next
static int p2pRead(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int returnStatus;
	printf("p2pRead: (path=\"%s\", buffer=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n", path, buffer, size, offset, fi);
	return p2pSysCall("pread", pread(fi->fh, buffer, size, offset), 0);
	/*
	char file54Text[] = "Hello World From File54!";
	char file349Text[] = "Hello World From File349!";
	char *selectedText = NULL;

	// ... //

	if (strcmp(path, "/file54") == 0)
		selectedText = file54Text;
	else if (strcmp(path, "/file349") == 0)
		selectedText = file349Text;
	else
		return -1;

	// ... //

	memcpy(buffer, selectedText + offset, size);

	return strlen(selectedText) - offset;
	*/
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
	printf("\np2pOpen: Opening File in Path \"%s\", with fi=0x%08x\n", path, fi);
	p2pFullPath(filePath, path); //returns the fullpath concatenated

	// if the open call succeeds, my returnStatus is the file descriptor,
	// else it's -errno.  I'm making sure that in that case the saved
	// file descriptor is exactly -1.
	fileDescriptor = p2pSysCall("open", open(filePath, fi->flags), 0);
	if (fileDescriptor < 0)
	{
		returnStatus = p2pError("open");
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
	printf("Aguement Values are... %s %s %s\n", argv[0], argv[1], argv[2]);

	//Need root directory which is index 2 or (3-1)
	p2pData->rootDir = realpath(argv[argc - 1], NULL); //0 is instructions, 1 is -f, 2 is mountdir total 3
	printf("Ceated Pointer to Path of Root Directory: %s\n", p2pData->rootDir);
	//argv[argc - 2] = argv[argc - 1];
	//argv[argc - 1] = NULL;
	//argc--;

	//bb_data->logfile = log_open();

	//Check Fuse Version we are runnign
	fprintf(stderr, "Checking Fuse Version... %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
	return fuse_main(argc, argv, &operations, p2pData);
}
