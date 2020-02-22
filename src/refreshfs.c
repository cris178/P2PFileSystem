
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

int order = 0;
// ... //
//Each array can store 256 strings and each string has the maximum length of 256 bytes.
//Names of directories created
char dir_list[256][256];
int curr_dir_idx = -1;

//Names of files created
char files_list[256][256];
int curr_file_idx = -1;

//Maintains the contents of the files
char files_content[256][256];
int curr_file_content_idx = -1;

//Initially no entries in each of the arrays so index -1

//Incremeent directory index meaning one new directory created, put it in the dire list
void add_dir(const char *dir_name)
{
	curr_dir_idx++;
	strcpy(dir_list[curr_dir_idx], dir_name);
}

//Checks our directory array to see if given path, is it in the list of directories.
//If it's in list it means that it's in our file system
//Returns 1 if in list 0 if not
int is_dir(const char *path)
{
	//Pointers are numbers to starting position of some array
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
		if (strcmp(path, dir_list[curr_idx]) == 0)
			return 1;

	return 0;
}

//removes / and gets file name
//Notice curr file index and file contents related
//files are related by index and contents
//We created a file and initialize it to blank string, write is another function.
void add_file(const char *filename)
{
	curr_file_idx++;
	strcpy(files_list[curr_file_idx], filename);

	curr_file_content_idx++;
	strcpy(files_content[curr_file_content_idx], "");
}

//same as isDir() function we made above.
//checks to see if its in our file system by checking our arrays.
//IF WE WANT TO GET SUB DIRECTORIES WOULD NEED TO REMOVE "/" until objects name not just files
int is_file(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
		if (strcmp(path, files_list[curr_idx]) == 0)
			return 1;

	return 0;
}

//When we are given a path we need to get the associated index
//of the file.
//We eliminate the /home/filename unimportant part to just get the name
//We then look for the name in our array of files and return the index else return -1 file not in our array.
int get_file_index(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
		if (strcmp(path, files_list[curr_idx]) == 0)
			return curr_idx;

	return -1;
}

//get path of the file you wish to write to and the content to be written
//Get the index of where to write given the path. If file not found return -1.
//else copy your new content into the filescostent with array index we just found.
void write_to_file(const char *path, const char *new_content)
{
	int file_idx = get_file_index(path);

	if (file_idx == -1) // No such file
		return;

	strcpy(files_content[file_idx], new_content);
}

// ... //

static int do_getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{

	printf("Path is: %s: \n", path);
	// printf("Path dereferenced is: %s \n", *path);
	order = 0;
	printf("-----------getattr: %i\n", order);
	printf("File is %s\n", path);
	// printf("Mountdir Path name is: ");
	
	char* tempPath = path;

	tempPath++;


	printf("tempPath: %s\n", tempPath);

	int returnStatus;
	char fpath[PATH_MAX];

	//form the absolute path to the file in question
	printf("mountpoint path: %s\n", mountpoint.path);
	printf("fpath: %s\n", fpath);
	printf("path: %s\n", path);

	strncpy(fpath, mountpoint.path, PATH_MAX);
	strncat(fpath,path,PATH_MAX);
	
	printf("AFTER\n\n");
	printf("mountpoint path: %s\n", mountpoint.path);
	printf("fpath: %s\n", fpath);
	printf("path: %s\n", path);


	printf("Full absolute path created: %s\n", fpath);

	// lstat(fpath, st);


	if(strcmp(path, "/") != 0)
	{
		returnStatus = stat(tempPath, st);

	}
	else
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink=2;
		return 0;
	}
	
	if(returnStatus < 0)
	{
		perror("Something went wrong in do_getattr: \n");
		returnStatus = -errno;
	}
	

	


	if (S_ISDIR(st->st_mode))
	{
		// st->st_mode = S_IFDIR | 0755;
		// st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
		printf("Inode number %d\n", st->st_ino);
		printf("is directory: %d\n", S_ISDIR(st->st_mode) ); 
	}
	else if (S_ISREG(st->st_mode))
	{
		// st->st_mode = S_IFREG | 0644;
		// st->st_nlink = 1;
		// st->st_size = 1024;


		printf("File NOT directory\n");
	}
	// else
	// {
	// 	return -ENOENT;
	// }

	return returnStatus;


	// st->st_uid = getuid();	 // The owner of the file/directory is the user who mounted the filesystem
	// st->st_gid = getgid();	 // The group of the file/directory is the same as the group of the user who mounted the filesystem
	// st->st_atime = time(NULL); // The last "a"ccess of the file/directory is right now
	// st->st_mtime = time(NULL); // The last "m"odification of the file/directory is right now

	// if (strcmp(path, "/") == 0 || is_dir(path) == 1)
	// {
	// 	st->st_mode = S_IFDIR | 0755;
	// 	st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	// }
	// else if (is_file(path) == 1)
	// {
	// 	st->st_mode = S_IFREG | 0644;
	// 	st->st_nlink = 1;
	// 	st->st_size = 1024;
	// }
	// else
	// {
	// 	return -ENOENT;
	// }

	// return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	order++;
	printf("-----------do_readdir: %i\n", order);
	filler(buffer, ".", NULL, 0);  // Current Directory
	filler(buffer, "..", NULL, 0); // Parent Directory

	if (strcmp(path, "/") == 0) // If the user is trying to show the files/directories of the root directory show the following
	{
		for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
			filler(buffer, dir_list[curr_idx], NULL, 0);

		for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
			filler(buffer, files_list[curr_idx], NULL, 0);
	}

	return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	order++;
	printf("-----------doread: %i\n", order);
	int file_idx = get_file_index(path);

	if (file_idx == -1)
		return -1;

	char *content = files_content[file_idx];

	memcpy(buffer, content + offset, size);

	return strlen(content) - offset;
}

static int do_mkdir(const char *path, mode_t mode)
{
	order++;
	printf("-----------domkdir: %i\n", order);
	path++;
	add_dir(path);

	return 0;
}

static int do_mknod(const char *path, mode_t mode, dev_t rdev)
{
	order++;
	printf("-----------domaknod: %i\n", order);
	path++;
	add_file(path);

	return 0;
}

static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
	order++;
	printf("-----------dowrite: %i\n", order);
	write_to_file(path, buffer);

	return size;
}

static struct fuse_operations operations = {
	.getattr = do_getattr,
	.readdir = do_readdir,
	.read = do_read,
	.mkdir = do_mkdir,
	.mknod = do_mknod,
	.write = do_write,
};

char *fuse_mnt_resolve_path(const char *progname, const char *orig)
{
	char buf[PATH_MAX];
	char *copy;
	char *dst;
	char *end;
	char *lastcomp;
	const char *toresolv;

	if (!orig[0])
	{
		fprintf(stderr, "%s: invalid mountpoint '%s'\n", progname,
				orig);
		return NULL;
	}

	copy = strdup(orig);
	if (copy == NULL)
	{
		fprintf(stderr, "%s: failed to allocate memory\n", progname);
		return NULL;
	}

	toresolv = copy;
	lastcomp = NULL;
	for (end = copy + strlen(copy) - 1; end > copy && *end == '/'; end--)
		;
	if (end[0] != '/')
	{
		char *tmp;
		end[1] = '\0';
		tmp = strrchr(copy, '/');
		if (tmp == NULL)
		{
			lastcomp = copy;
			toresolv = ".";
		}
		else
		{
			lastcomp = tmp + 1;
			if (tmp == copy)
				toresolv = "/";
		}
		if (strcmp(lastcomp, ".") == 0 || strcmp(lastcomp, "..") == 0)
		{
			lastcomp = NULL;
			toresolv = copy;
		}
		else if (tmp)
			tmp[0] = '\0';
	}
	if (realpath(toresolv, buf) == NULL)
	{
		fprintf(stderr, "%s: bad mount point %s: %s\n", progname, orig,
				strerror(errno));
		free(copy);
		return NULL;
	}
	if (lastcomp == NULL)
		dst = strdup(buf);
	else
	{
		dst = (char *)malloc(strlen(buf) + 1 + strlen(lastcomp) + 1);
		if (dst)
		{
			unsigned buflen = strlen(buf);
			if (buflen && buf[buflen - 1] == '/')
				sprintf(dst, "%s%s", buf, lastcomp);
			else
				sprintf(dst, "%s/%s", buf, lastcomp);
		}
	}
	free(copy);
	if (dst == NULL)
		fprintf(stderr, "%s: failed to allocate memory\n", progname);
	return dst;
}

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
	mountpoint.path = realpath(argv[argc - 1], NULL);
	printf("MOUNTPATH GOT: %s\n", mountpoint.path);
	//testpath = fuse_mnt_resolve_path(strdup(argv[0]), argv[argc - 1]); returns same thing


	printf("---------This is the realpath should be absolute: %s\n", mountpoint.path);
	//printf("---------This is the testpath should be: %s\n", testpath);

	mountpoint.dir = malloc(sizeof(struct refreshfs_dirp));
	if (mountpoint.dir == NULL)
	{ //Not enough memory
		return -ENOMEM;
	}
	mountpoint.dir->dp = opendir(mountpoint.path);

	if ((mountpoint.fd = dirfd(mountpoint.dir->dp)) == -1)
	{
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}
	mountpoint.dir->offset = 0;
	mountpoint.dir->entry = NULL;

	// closedir(mountpoint.dir->dp);
	// free(mountpoint.path);

	returnStatus = fuse_main(argc, argv, &operations, NULL);

	closedir(mountpoint.dir->dp);
	free(mountpoint.path);

	return returnStatus;
}