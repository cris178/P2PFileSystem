
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>



#include <iostream>
#include <vector>
#include <opendht.h>


using std::cout;
using std::endl;

#define PATH_MAX 4096

struct refreshfs_dirp
{
	DIR *dp;
	struct dirent *entry;
	off_t offset;
};

static struct mountpoint
{
	int fd;
	struct refreshfs_dirp *dir;
	char *path;
} mountpoint;




 dht::DhtRunner node;




int order = 0;

//Initially no entries in each of the arrays so index -1
int do_release(const char *path, struct fuse_file_info *fi)
{
	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);


	printf("in do_release---------------------------------------------------------------\n");

    return close(fi->fh);
}



int do_open(const char *path, struct fuse_file_info *fi)
{

	order++;
	printf("----------------------------------------------------------------------------------------------------------------------------do_open: %i\n", order);

    int retstat = 0;
    int fd;
    
	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
    
    // if the open call succeeds, my retstat is the file descriptor,
    // else it's -errno.  I'm making sure that in that case the saved
    // file descriptor is exactly -1.

	
	printf("in do_open path: %s", path);
    fd = open(fpath, fi->flags);
    if (fd < 0)
	{
		perror("error in do_open");
		retstat = -errno;
	}

    fi->fh = fd;

    
    return retstat;
}


int do_opendir(const char *path, struct fuse_file_info *fi)
{
	order++;
	printf("-----------do_opendir: %i\n", order);

    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];
    
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);

    // since opendir returns a pointer, takes some custom handling of
    // return status.

	// ++path;
	// strncpy(fpath, mountpoint.path, PATH_MAX);
	// strncat(fpath, path, PATH_MAX);

	// printf("in do_opendir path: %s\n", path);
	
	printf("in do_opendir path: %s\n", path);
    dp = opendir(fpath);
    // log_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL)
	{
		 perror("in do_opendir dp is null\n");
		retstat = -errno;
	}
    fi->fh = (intptr_t) dp;
    
	if(fi->fh == NULL)
	{
		printf("in do_opendir fi->fh is null\n");
	}
    // log_fi(fi);
    
    return retstat;
}

// ... //

static int do_getattr(const char *path, struct stat *st)
{

	order = 0;
	printf("-----------getattr: %i\n", order);
	printf("File is %s\n", path);

	int returnStatus;


	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);


	returnStatus = stat(fpath, st);


	if (returnStatus < 0)
	{
		perror("Something went wrong in do_getattr: \n");
		returnStatus = -errno;
	}

	if (S_ISDIR(st->st_mode))
	{
		// st->st_mode = S_IFDIR | 0755;
		// st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
		printf("Inode number %d\n", st->st_ino);
		printf("is directory: %d\n", S_ISDIR(st->st_mode));
	}
	else if (S_ISREG(st->st_mode))
	{

		printf("File NOT directory\n");
	}
	

	return returnStatus;

}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	int retstat = 0;
	order++;
	printf("-----------do_readdir: %i\n", order);
	
		
	DIR *dp;
	struct dirent *de;
		
		
	
	dp = (DIR *) (uintptr_t) fi->fh;
		

		

	// Every directory contains at least two entries: . and ..  If my
	// first call to the system readdir() returns NULL I've got an
	// error; near as I can tell, that's the only condition under
	// which I can get an error from readdir()

	printf("Before--------------------------\n");
	de = readdir(dp);
	printf("After--------------------------\n");

	if(de == 0)
	{
		return retstat; 
	}


	do 
	{
		// log_msg("calling filler with name %s\n", de->d_name);
		if (filler(buffer, de->d_name, NULL, 0) != 0) 
		{
			// log_msg("    ERROR bb_readdir filler:  buffer full");
			perror("error in readdir");
			return -ENOMEM;
		}
    } 
	while ((de = readdir(dp)) != NULL);
    
   
	return retstat;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	order++;
	printf("--------------------------------------------------------------------------------------------------------doread: %i\n", order);
	
	int retstat = 0;
	
	retstat = pread(fi->fh, buffer, size, offset);

	if(retstat < 0)
	{
		perror("error in do_read ");
		return -errno;
	}

	return retstat;
	
}


static int do_mkdir(const char *path, mode_t mode)
{
	order++;
	printf("-----------do mkdir: %i\n", order);
	int returnStatus = -1;
	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);


	returnStatus = mkdir(fpath, mode);

	if (returnStatus < 0)
	{
		perror("problem in mkdir: %s");
		return -errno;
	}
	return returnStatus;
}


/** Remove a file */
static int do_unlink(const char *path)
{
	order++;
	printf("-----------do unlink: %i\n", order);
	int returnstatus = 0;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	returnstatus = unlink(fpath);
	if(returnstatus < 0){
		perror("Problem in do_unlink");
		return -errno;
	}

    return returnstatus;
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
static int do_symlink(const char *path, const char *link)
{
	order++;
	printf("-----------dosymlink: %i\n", order);
	int returnstatus = 0;
    char flink[PATH_MAX];
	strncpy(flink, mountpoint.path, PATH_MAX);
	strncat(flink, link, PATH_MAX);

	returnstatus = symlink(path, flink);
	if(returnstatus < 0){
		perror("Error in Symlink");
		return -errno;
	}

    return returnstatus;
}


static int do_rename(const char *path, const char *newpath)
{
	order++;
	printf("-----------rename: %i\n", order);

    int returnstatus = 0;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	

    char fnewpath[PATH_MAX];
	strncpy(fnewpath, mountpoint.path, PATH_MAX);
	strncat(fpath, newpath, PATH_MAX);

	returnstatus = rename(fpath, fnewpath);
    if(returnstatus < 0){
		perror("Error in rename");
		return -errno;
	}
   

    return returnstatus;
}


/** Remove a directory */
static int do_rmdir(const char *path)
{
	order++;
	printf("-----------rmdir: %i\n", order);
    int returnstatus = 0;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	returnstatus =  rmdir(fpath);
	if(returnstatus < 0){
		perror("Problem in do_unlink");
		return -errno;
	}

    return returnstatus;
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
static int do_utime(const char *path, struct utimbuf *ubuf)
{
	order++;
	printf("-----------utime: %i\n", order);
    
    int returnstatus = 0;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
    
    returnstatus = utime(fpath, ubuf);
	if(returnstatus < 0){
		perror("Error in utime");
		return -errno;
	}


    return returnstatus;
}


static int do_mknod(const char *path, mode_t mode, dev_t rdev)
{
	order++;
	printf("-----------domaknod: %i\n", order);

	int retstat;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
    
    
    // On Linux this could just be 'mknod(path, mode, dev)' but this
    // tries to be be more portable by honoring the quote in the Linux
    // mknod man page stating the only portable use of mknod() is to
    // make a fifo, but saying it should never actually be used for
    // that.
	


    if (S_ISREG(mode)) 
	{
		retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (retstat >= 0)
		{
			retstat = close(retstat);
		}

		if (retstat < 0)
		{
			perror("Something went wrong in do_mknod S_ISREG: \n");
			retstat = -errno;
		}
	} 
	else
	{
		if (S_ISFIFO(mode))
		{
			retstat =mkfifo(fpath, mode);
			if (retstat < 0)
			{
				perror("Something went wrong in do_mknod S_ISFIFO: \n");
				retstat = -errno;
			}
		}
		else
		{
			retstat = mknod(fpath, mode, rdev);

				perror("Something went wrong in do_mknod ELSE: \n");
				retstat = -errno;
		}
	}

	return retstat;
}

static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	order++;
	printf("------------------------------------------------------------------------------------dowrite: %i\n", order);

	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	

	int retstat = 0;

	retstat = pwrite(fi->fh, buffer, size, offset);

	cout << "fpath also caoncatenated and put in node \n" ;
	cout << "--------------------------------------------in do_write Path: " << path << " buffer: " << buffer << endl; 
	


	if(retstat < 0)
	{
		perror("error in do_write ");
		return -errno;
	}

	return retstat;
}



static struct hello_fuse_operations:fuse_operations 
{
	hello_fuse_operations()
	{
	getattr = do_getattr;
	readdir = do_readdir;
	opendir = do_opendir;
	open = do_open;
	read = do_read;
	mkdir = do_mkdir;
	unlink = do_unlink;
	symlink = do_symlink;
    rename = do_rename;
	utime = do_utime;
	rmdir = do_rmdir;
	mknod = do_mknod; //Creates non directory, non sym link nodes
	write = do_write;
	release = do_release;
	}
} operations;




int main(int argc, char *argv[])
{

	int returnStatus;
	char *testpath;

	//No Root Users, Security issues arise!
	if ((getuid() == 0) || (geteuid() == 0))
	{
		fprintf(stderr, "Running FUSE file systems as root opens unnacceptable security holes\n");
		return 1;
	}

	//Returns absolute path
	mountpoint.path = realpath("rootdir", NULL);
	printf("MOUNTPATH GOT: %s\n", mountpoint.path);
	

	printf("---------This is the realpath should be absolute: %s\n", mountpoint.path);
	

	mountpoint.dir = (refreshfs_dirp*)malloc(sizeof(struct refreshfs_dirp));
	if (mountpoint.dir == NULL)
	{ //Not enough memory
		return -ENOMEM;
	}
	
	mountpoint.dir->offset = 0;
	mountpoint.dir->entry = NULL;




	std::cout << "starting DHT stuff" << std::endl; 

	//
    // Launch a dht node on a new thread, using a
    // generated RSA key pair, and listen on port 4222.
	//This is my node
    node.run(4222, dht::crypto::generateIdentity(), true);

    // Join the network through any running node,
    // here using a known bootstrap node.
    node.bootstrap("bootstrap.ring.cx", "4222");

    std::cout << "P Data" << std::endl;
   node.put("fpath", "buffer");

    std::cout << "-----------------" << std::endl;


    // wait for dht threads to end




	returnStatus = fuse_main(argc, argv, &operations, NULL);

	closedir(mountpoint.dir->dp);
	free(mountpoint.path);

	node.join();


	return returnStatus;
}









