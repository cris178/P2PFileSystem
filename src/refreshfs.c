//USE UUID instead of inode in dht
#ifndef FUSE__USE_VERSION 30
#define FUSE_USE_VERSION 30
#endif

#include "params.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdarg.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static int do_getattr( const char *path, struct stat *st )
{
	int length = strlen(path);
	/*if(strncmp(path, "/.Trash", length) == 0 | strncmp(path, "/.Trash-1000", length) == 0 | strncmp(path, "/.xdg-volume-info", length) == 0 | strncmp(path, "/autorun.inf", length) == 0){
		return -1;
	}*/

	
	struct stat buff;
	int retstat;
    /*char fpath[PATH_MAX];
	printf( "getattr: Entered\n" );
	printf( "\tRelative Path Provided is %s\n", path );
	printf("getattr: Rootdir pulling %s\n", REFRESH_DATA->rootdir);
    strcpy(fpath, REFRESH_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will break | full path not needed
	printf("getattr: full path acquired %s\n",fpath);
	*/
	retstat = lstat(path,&buff);
	printf("getattr: completed lstat() returning %d\n",retstat);
	
	if(retstat == 0){
		printf("getAttr: Successfully set up stat struct\n"); 
		st->st_uid = buff.st_uid;
		st->st_gid = buff.st_gid;
		st->st_atime = buff.st_atime;
		st->st_mtime = buff.st_mtime;
		st->st_mode = buff.st_mode;
		st->st_nlink = buff.st_nlink;
		st->st_size = buff.st_size;
		return retstat;
	}else{
		perror("Error in getattr: ");
		return -errno;
	}

	
	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon 
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
	
}	
/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int do_open(const char *path, struct fuse_file_info *fi)
{
	printf("open: Entered Open.\n");
    int retstat = 0;
    int fd;
    char fpath[PATH_MAX];

	/*
	printf( "\tRelative Path Provided is %s\n", path );
	printf("getattr: Rootdir pulling %s\n", REFRESH_DATA->rootdir);
    strcpy(fpath, REFRESH_DATA->rootdir);
    strncat(path, path, PATH_MAX); // ridiculously long paths will break | full path not needed
	printf("getattr: full path acquired %s\n",fpath);
	*/
    
    //log_msg("\nbb_open(path\"%s\", fi=0x%08x)\n",path, fi);
    //bb_fullpath(fpath, path);
    
    // if the open call succeeds, my retstat is the file descriptor,
    // else it's -errno.  I'm making sure that in that case the saved
    // file descriptor is exactly -1.
    fd = open(path, fi->flags); //Call to open creates new open file descriptor
								//In table of open files
								//file descriptor is a reference to open file description
								//file descriptor is smallest non neg number assigned in table
								//used in system calls (read, write,lseek,fcntl)
    if (fd < 0){
	  perror("open Error: ");
	  retstat = -errno;
	}
	printf("open: No errors found in file descriptor.\n");

    fi->fh = fd; //Set file handler to file descripter
				//retstat = log_error("open");

    //log_fi(fi);
    
    return retstat;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf("do_readdir: Entered read directory\n");
	int returnStatus = 0;//Should return this value unless an error comes up
	DIR *directoryPointer;
	struct dirent *directoryEntry;
	
	//char fpath[PATH_MAX];
	//strcpy(fpath, REFRESH_DATA->rootdir);
    //strncat(fpath, path, PATH_MAX); // ridiculously long paths will break | full path not needed
	//printf("getattr: full path acquired %s\n",fpath);


	//Not working with a path but will need to cast fi-fh which is the file handler aka file descripter. 
	directoryPointer = (DIR *) (uintptr_t) fi->fh;
	printf("This is the file handler");
	printf("readdir: directoryPointer initialized to filehandler %d\n", fi->fh);

	//Directories have two entries, one to itself and the other to it's parent . and ..
	//First call to readdir could return null which is an error
	directoryEntry = readdir(directoryPointer);
	printf("readir: Successfully returned from readir().\n");

	//Deal with the error
	if(directoryEntry == 0){
		perror("do_readdir: Error found: ");
		returnStatus = -errno;
		return returnStatus;
	}
	printf("readdir: No errors detected in directoryEntries");

	do {
		printf("do_readdir: calling filler with name %s\n", directoryEntry->d_name);
		if (filler(buffer, directoryEntry->d_name, NULL, 0) != 0) {
			printf("do_readdir: Error Buffer is full\n");
			return -ENOMEM; //Not enough space can't allocate memory man errno
		}
    } while ((directoryEntry = readdir(directoryPointer)) != NULL);  //Reading the directory until nothing more to read


	return returnStatus;

	/*
	printf( "--> Getting The List of Files of %s\n", path );
	
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		filler( buffer, "file54", NULL, 0 );
		filler( buffer, "file349", NULL, 0 );
	}
	
	return 0;
	*/
} 

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int do_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int returnstatus = 0;
    char fpath[PATH_MAX];
    
    printf("opendir: Entered open directory\n");

	/*printf( "\tRelative Path Provided is %s\n", path );
	printf("getattr: Rootdir pulling %s\n", REFRESH_DATA->rootdir);
    strcpy(fpath, REFRESH_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will break | full path not needed
	printf("getattr: full path acquired %s\n",fpath);*/

	/*printf("opendir: Opendir pulling rootdir %s\n", REFRESH_DATA->rootdir);
    strcpy(fpath, REFRESH_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will break | full path not needed
	printf("opendir: full path acquired %s\n",fpath);*/
    //bb_fullpath(fpath, path);

    // since opendir returns a pointer, takes some custom handling of
    // return status.
    dp = opendir(path);
	printf("opendir: opendir returned successfully.\n");
    //log_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL){
		returnstatus = -errno;
	}
    
    fi->fh = (intptr_t) dp;
    
    
    
    return returnstatus;
}




//fusexmp code returns the amount of data returned by a read.
static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	printf("read: Entered read\n");
	int returnstatus = 0;
    
    printf("\nread: path is = \"%s\", buffer is = 0x%08x, size is = %d, offset is = %lld, fi is = 0x%08x)\n",path, buffer, size, offset, fi);
    // since we already have a file descriptor in fi->fh we don't really need a path here. 
	returnstatus = pread(fi->fh, buffer, size, offset); //On succes returns number of bytes read, failure returns -1 and errno set.
	if(returnstatus < 0){
		perror("read Error: "); //Returns errno description
		return -errno;
	}
	printf("read: Read %d of bytes\n",returnstatus);
    return returnstatus;
	
}

void *do_init(struct fuse_conn_info *conn)
{
    printf("\ndo_init()\n");
    
    
    //log_fuse_context(fuse_get_context());
    
    return REFRESH_DATA;
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
	.open		= do_open,
	.opendir 	= do_opendir,
	.init = do_init,
};


int main(int argc, char *argv[])

{
	struct refresh_state *refresh_data; //We need to keep track of the state of our file system. Will save path here.
	int fuse_stat;

	//Check the provided arguements in command line argc = number of arguements
	//argv is the array of provided arguements.
	if (argc < 1)
	{
		printf("please provide a mounting point\n");
		abort(); //End program
	}



	//Don't allow user to run as Root. Major security issues.
	if ((getuid() == 0) || (geteuid() == 0))
	{
		printf("Can't Run Fuse File System as Root User! Security issues.\n");
		return 1; //End Program
	}


	printf("Starting up FileSystem...\n");
	refresh_data = malloc(sizeof(struct refresh_state));

	if (refresh_data == NULL)
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
	refresh_data->rootdir = realpath(argv[argc - 1], NULL); //0 is instructions, 1 is -f, 2 is mountdir total 3
	//argv[argc-2] = argv[argc-1];
    //argv[argc-1] = NULL;
    //argc--;

	printf("Ceated Pointer to Path of Root Directory: %s\n", refresh_data->rootdir);



	//-s single thread -f debug



	//Check Fuse Version we are runnign
	printf("Checking Fuse Version... %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
	fuse_stat = fuse_main(argc, argv, &operations, refresh_data);

	return fuse_stat;
}