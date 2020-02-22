
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


int order = 0;
// ... //
//Each array can store 256 strings and each string has the maximum length of 256 bytes.
//Names of directories created
char dir_list[ 256 ][ 256 ];
int curr_dir_idx = -1;

//Names of files created
char files_list[ 256 ][ 256 ];
int curr_file_idx = -1;

//Maintains the contents of the files
char files_content[ 256 ][ 256 ];
int curr_file_content_idx = -1;


//Initially no entries in each of the arrays so index -1


//Incremeent directory index meaning one new directory created, put it in the dire list
void add_dir( const char *dir_name )
{
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );
}


//Checks our directory array to see if given path, is it in the list of directories.
//If it's in list it means that it's in our file system
//Returns 1 if in list 0 if not
int is_dir( const char *path )
{
	//Pointers are numbers to starting position of some array
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

//removes / and gets file name 
//Notice curr file index and file contents related
//files are related by index and contents
//We created a file and initialize it to blank string, write is another function.
void add_file( const char *filename )
{
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}



//same as isDir() function we made above.
//checks to see if its in our file system by checking our arrays.
//IF WE WANT TO GET SUB DIRECTORIES WOULD NEED TO REMOVE "/" until objects name not just files
int is_file( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}


//When we are given a path we need to get the associated index
//of the file. 
//We eliminate the /home/filename unimportant part to just get the name
//We then look for the name in our array of files and return the index else return -1 file not in our array.
int get_file_index( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}


//get path of the file you wish to write to and the content to be written
//Get the index of where to write given the path. If file not found return -1.
//else copy your new content into the filescostent with array index we just found.
void write_to_file( const char *path, const char *new_content )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( files_content[ file_idx ], new_content ); 
}

// ... //

static int do_getattr( const char *path, struct stat *st )
{
	order = 0;
	printf("-----------getattr: %i\n", order);
	printf("File is %s\n", path);
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	if ( strcmp( path, "/" ) == 0 || is_dir( path ) == 1 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else if ( is_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else
	{
		return -ENOENT;
	}
	
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	order++;
	printf("-----------do_readdir: %i\n", order);
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
			filler( buffer, dir_list[ curr_idx ], NULL, 0 );
	
		for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
			filler( buffer, files_list[ curr_idx ], NULL, 0 );
	}
	
	return 0;
}


static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	order++;
	printf("-----------doread: %i\n", order);
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 )
		return -1;
	
	char *content = files_content[ file_idx ];
	
	memcpy( buffer, content + offset, size );
		
	return strlen( content ) - offset;
}

static int do_mkdir( const char *path, mode_t mode )
{	
	order++;
	printf("-----------domkdir: %i\n", order);
	path++;
	add_dir( path );
	
	return 0;
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
	order++;
	printf("-----------domaknod: %i\n", order);
	path++;
	add_file( path );
	
	return 0;
}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	order++;
	printf("-----------dowrite: %i\n", order);
	write_to_file( path, buffer );
	
	return size;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
};


int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &operations, NULL );
}