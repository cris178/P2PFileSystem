
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

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>	//http://man7.org/linux/man-pages/man2/getuid.2.html
#include <sys/types.h> //This and above are for getting uid(userid) and gid(groupid)
#include <time.h>	  //Used to get acess time of files and modification times
#include <string.h>
#include <stdlib.h>

//getattr - similar to stat function in linux
//We will pass in two paramters and retunr an integer
//Param 1: path of file we will get attributes of
//Param 2: stat structure that needs to be that needs to be filled with attributes
//Return 0: if success
//return -1: with errno with correct errorcode
static int p2pGetAttr(const char *path, struct stat *stats)
{

	stats->st_uid = getuid();	 //stuid is the owner of the file. ->Make this person who mounted the directory.
	stats->st_gid = getgid();	 //owner group of the files or directories/subdirectories. ->Make this person who mounted the directory
	stats->st_atime = time(NULL); //last acess time
	stats->st_mtime = time(NULL); //last modification time

	if (strcmp(path, "/") == 0) //Wil run first option if in root
	{
		stats->st_mode = S_IFDIR | 0755; //(check if file or dir) |(permission bits)        st_mode shows if is regular file, directior, other and permission bits of that file.
		stats->st_nlink = 2;			 //Shows number of Hardlinks, Hardlinks, like copying a file but not a copy, a link to original file but modifying hardlink modifies original as well.

		/*Seeing what S_IFDIR retunrs*/
		printf("CHECK Testing: \t%i", S_IFDIR);

	} //Reason why twp hardlinks  https://unix.stackexchange.com/questions/101515/why-does-a-new-directory-have-a-hard-link-count-of-2-before-anything-is-added-to/101536#101536
	else
	{
		stats->st_mode = S_IFREG | 0644;
		stats->st_nlink = 1;
		stats->st_size = 1024; //Size of files in bytes
	}

	return 0;
}

//Only need first 3 params - Path of dir  - buffer - filler
static int p2pDoReadDir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("--> Getting The List of Files of %s\n", path);

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
}

static int p2pDoRead(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("--> Trying to read %s, %u, %u\n", path, offset, size);

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
}

static struct fuse_operations operations = {
	.getattr = p2pGetAttr,
	.readdir = p2pDoReadDir,
	.read = p2pDoRead,
};

int main(int argc, char *argv[])
{
	//Check Fuse Version we are runnign
	fprintf(stderr, "Checking Fuse Version... %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

	//Don't allow user to run as Root. Major security issues.
	if ((getuid() == 0) || (geteuid() == 0)) {
    		fprintf(stderr, "Can't Run Fuse File System as Root User! Security issues.\n");
    		return 1;
    	}
	return fuse_main(argc, argv, &operations, NULL);
}
