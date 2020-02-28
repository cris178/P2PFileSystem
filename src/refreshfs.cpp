
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

/*
class NodeODHT{
	public:
		char* pathnameODHT;
		char* content;
};


std::vector<NodeODHT> synchedNodes;


*/


dht::DhtRunner node;




int order = 0;





std::string translateDHTEntry_dataRetrieved;
int translateDHTEntry(const char* path) 
{
	node.get(path, 
	[&](const std::vector<std::shared_ptr<dht::Value>>& values)  
	{

		
		// Callback called when values are found
		for (const auto& value : values)
		{
			std::stringstream mystream;
			std::string dataAsString;
			// std::cout << value.ValueType << endl;
			mystream << *value;
			dataAsString = mystream.str();

			dataAsString = dataAsString.substr(dataAsString.find("data:") + 7);
			dataAsString.pop_back();


			int len = dataAsString.length();
			std::string newString;
			for(int i=0; i< len; i+=2)
			{
				std::string byte = dataAsString.substr(i,2);
				char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
				newString.push_back(chr);
			}


			cout << "FINAL: " << newString << endl << endl; 
			
			translateDHTEntry_dataRetrieved = newString;


			cout << translateDHTEntry_dataRetrieved <<endl;
			// cout << "The string stread contains: " << mystream.str() << "kkkkkkkkkkkkkkkkkkkkkkk" << endl;

			// std::cout << dht::crypto::PublicKey({'K', '5'}).encrypt({5,10}) << std::endl;
			// std::cout << "Decrypted: " << decrypt(*value) << std::endl;
		}

		// usleep(100);

		return true; // return false to stop the search
    });

		return 0;
}




std::vector<std::string> listOfFiles;
int translateListOfFiles() 
{
	// listOfFiles.clear();
	
	node.get((char*)"LIST_OF_FILES", 
	[&](const std::vector<std::shared_ptr<dht::Value>>& values)  
	{

		
		// Callback called when values are found
		for (const auto& value : values)
		{
			std::stringstream mystream;
			std::string dataAsString;
			// std::cout << value.ValueType << endl;
			mystream << *value;
			dataAsString = mystream.str();

			dataAsString = dataAsString.substr(dataAsString.find("data:") + 7);
			dataAsString.pop_back();


			int len = dataAsString.length();
			std::string newString;
			for(int i=0; i< len; i+=2)
			{
				std::string byte = dataAsString.substr(i,2);
				char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
				newString.push_back(chr);
			}
			
			listOfFiles.push_back(newString);
		}

		return true; // return false to stop the search
    });

		return 0;
}




//Initially no entries in each of the arrays so index -1
int do_release(const char *path, struct fuse_file_info *fi)
{
	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);


	order++;
	printf("in do_release--------------------------------------------------------------------------------%i\n", order);



    return close(fi->fh);
}



int do_open(const char *path, struct fuse_file_info *fi)
{

	order++;
	printf("------------------------------------------------------------------------------do_open: %i\n", order);

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
	printf("--------------------------------------------------------------------do_opendir: %i\n", order);

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


	translateListOfFiles();



	cout << "List of Files =============================================================================================================================\n";
	if(listOfFiles.size()!= 0){
		for(int i = 0; i < listOfFiles.size(); ++i)
		{
			cout << listOfFiles.at(i) << ", ";
		}
		cout << endl;
	}
	

	order = 0;
	printf("------------------------------------------------------getattr: %i\n", order);
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
	printf("---------------------------------------------------------------do_readdir: %i\n", order);
	
		
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
	printf("-----------------------------------------------------------------------------doread: %i\n", order);
	
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
	printf("--------------------------------------do mkdir: %i\n", order);
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
	printf("-----------------------------------do unlink: %i\n", order);
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
	printf("--------------------------------dosymlink: %i\n", order);
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
	printf("----------------------------------------rename: %i\n", order);

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
	printf("----------------------------------------rmdir: %i\n", order);
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
	printf("---------------utime: %i\n", order);
    
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
	printf("----------------------------------------------domaknod: %i\n", order);

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
	printf("--------------------------------------------------------------------------------------------dowrite: %i\n", order);

	

	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	

	int retstat = 0;

	retstat = pwrite(fi->fh, buffer, size, offset);

	cout << "fpath also caoncatenated and put in node \n" ;


	if(retstat < 0)
	{
		perror("error in do_write ");
		return -errno;
	}

	
	node.put("LIST_OF_FILES", path);
	node.put(path, buffer);

	// translateDHTEntry(path);
	//cout << "translateDHTEntry: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" << translateDHTEntry_dataRetrieved << endl;
 	// for (int i = 0; i < translateDHTEntry_dataRetrieved.size(); ++i)
	// {

	return retstat;
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
static int do_access(const char *path, int mask)
{
	order++;
	cout << "-----------------in do_access: " << order << endl;
    int retstat = 0;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	
    
    retstat = access(fpath, mask);
    
    if (retstat < 0){
		perror("Error in access");
		return -errno;
	}
	
    
    return retstat;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
static int do_statfs(const char *path, struct statvfs *statv)
{
	order++;
	cout << "-------------in do_statfs: "   << order << endl;
    int retstat = 0;
	char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
    
    
    // get stats for underlying filesystem
	retstat = statvfs(fpath, statv);
	if(retstat < 0){
		perror("Error in do statfs");
		return -errno;
	}
    
    return retstat;
}



/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
// this is a no-op in BBFS.  It just logs the call and returns success
int do_flush(const char *path, struct fuse_file_info *fi)
{
	order++;
	cout << "-------------in flush: "   << order << endl;
    return 0;
}




/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
static int do_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
   order++;
    cout << "-------------in fsync: "  << order << endl;
    // some unix-like systems (notably freebsd) don't have a datasync call
#ifdef HAVE_FDATASYNC
    if (datasync){
		int returnstatus = 0;
		returnstatus= fdatasync(fi->fh)
		if(returnstatus < 0){
			perror("Error in first part of fsync");
			return returnstatus;
		}
		return returnstatus;
	}
    else
#endif	
	int returnstatus =0;
	returnstatus = fsync(fi->fh);
	if(returnstatus < 0){
		perror("Error in second part of fsync");
		return returnstatus;
	}

	return returnstatus;
}


/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ??? >>> I need to implement this...
static int do_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;
    order++;
    cout <<" -------------------------------------------FSYNCDIR"   << order << endl;
    
    return retstat;
}

/** Change the size of a file */
static int do_truncate(const char *path, off_t newsize)
{
	order++;
	cout << "------------------------------In Truncate " << order << endl;
    char fpath[PATH_MAX];
	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath, path, PATH_MAX);
	printf("Full absolute path created: %s\n", fpath);
	int returnstatus = 0;
	returnstatus = truncate(fpath, newsize);
	if(returnstatus < 0){
		perror("Error in do_truncate");
		return -errno;
	}

    return returnstatus;
}



static struct hello_fuse_operations:fuse_operations 
{
	hello_fuse_operations()
	{
	getattr = do_getattr; //1
	//.readlink = do_readlink; read a symbolic link
	readdir = do_readdir;
	getdir = NULL; //Deprecated
	mknod = do_mknod; //Creates non directory, non sym link nodes
	opendir = do_opendir;
	open = do_open;
	read = do_read;
	mkdir = do_mkdir; //makes a directory node
	unlink = do_unlink; //removes a file
	
	//symlink = do_symlink; //Who cares about symbolic links
    rename = do_rename; //rename a file or directory

	//link = do_link; Hard link meh who cares
  	//chmod = do_chmod; Change the permission bits, doesn't matter for our project
  	//chown = do_chown; change the owner or group of a file
  	truncate = do_truncate; //change size of file

	utime = do_utime;  //change access/modification time
	rmdir = do_rmdir; //removes a directory
	
	write = do_write;
	release = do_release;
	statfs = do_statfs;
	flush = do_flush;
	fsync = do_fsync;

	access= do_access;

	fsyncdir = do_fsyncdir;
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
    node.bootstrap("10.0.2.4", "33164");

    // std::cout << "P Data" << std::endl;
    // node.put("fpath", "buffer");

    // std::cout << "-----------------" << std::endl;


    // wait for dht threads to end




	returnStatus = fuse_main(argc, argv, &operations, NULL);

	closedir(mountpoint.dir->dp);
	free(mountpoint.path);

	node.join();


	return returnStatus;
}













