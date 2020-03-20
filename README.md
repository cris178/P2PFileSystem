# P2PFileSystem
A Peer to Peer file system built on top of FUSE (File System in User Space ) and openDHT(Distributed Hash Table). Our P2PFileSystem allows any user to make text files and directories available to any person with out file system mounted. Just write a .txt in your preffered text editor and watch as the file appears on any other computer.  


# Installing FUSE
In order to get the filesystem working several conditions need to be met.

Ubuntu: FUSE is not Windows supported by default. The FUSE Kernel Module is not installed and even if you use 
the Windows Subsystem for Linux, you'll run to this problem.

While FUSE comes pre installed with most versions of Linux we still need extra supportting software to support building our file system.
With a fresh install of Ubuntu, download and install the following.


## FUSE Install Requirements

Below are the terminal instructions required to install a Fuse File System. Feel free to copy and paste the terminal commands. 

**On the Ubuntu Terminal**

sudo apt-get update

sudo apt-get install fuse

sudo apt-get install libfuse-dev

**Download the fuse library of functions
https://github.com/libfuse/libfuse**

**In order to test if libfuse is working well
Will need the following**

**meson and ninja needed to build libfuse** 

sudo apt-get install meson

**For Meson to work we need to build essentials**
sudo apt-get upgrade

sudo apt-get install build essential

sudo apt-get install pkg-config

**Need Python3 to run the tests to see if FUSE is working**

sudo apt-get install python3.7

sudo apt-get install python3-pip

pip3 install --user pytest

**optional**

This small section can be ignored and you can move onto the openDHT section. In order to test if you installed fuse correctly go to the readme in the libfuse folder and just follow those instructions. You should see some passing tests if correctly installed.

Now your OS is ready to support a FUSE filesystem. **Next setup OpenDHT**. 

## OpenDHT Install Requirements


sudo apt-get --upgrade

sudo apt-get install build-essential

sudo apt install cmake

sudo apt install autoconf

**Install OpenDHT dependencies**

sudo apt install libncurses5-dev libreadline-dev nettle-dev libgnutls28-dev libargon2-0-dev libmsgpack-dev librestbed-dev libjsoncpp-dev

**clone the repo**

git clone https://github.com/savoirfairelinux/opendht.git

**build and install** 

cd opendht

mkdir build && cd build

cmake -DOPENDHT_PYTHON=OFF -DCMAKE_INSTALL_PREFIX=/usr ..

**Workaround for asio from issues thread in OpenDHT repository**

wget https://github.com/aberaud/asio/archive/b2b7a1c166390459e1c169c8ae9ef3234b361e3f.tar.gz \
&& tar -xvf b2b7a1c166390459e1c169c8ae9ef3234b361e3f.tar.gz && cd asio-b2b7a1c166390459e1c169c8ae9ef3234b361e3f/asio \
&& ./autogen.sh && ./configure --prefix=/usr --without-boost --disable-examples --disable-tests  \
&& sudo make install

cd ..

cd ..

**Go in the build folder**

make -j4

sudo make install


## Building our P2P File System

Now that you installed Fuse and OpenDHT you are ready to make our filesystem.

* Open P2PFileSystem Folder

* Open src Folder

mkdir rootdir 

mkdir Mountingpoint

make clean

make

./refreshFS -d -s Mountingpoint

This will make the file system run in single threaded mode and in devmode. 
Once it starts you will see the fuse file system operations on screen. Open a new 
tab on the terminal and navigate to the mounting point. You can now create and
edit files in a P2P filesystem. 

## Testing and Running

**Test Scripts**

Once you have two computers running our P2PFileSystem we made a script that automatically creates files and directories and another that checks if they were successfully created. 

    CreateFilesAndDirs.py

Makes several files and directories. 

    TestFilesAndDirsCreated.py	

Checks if the files were successfully created. 


**GDB**

You can run the program as stated above using the -d flag which puts the fuse file system in dev mode and allows you to see the functions running in real time. Or you can use GDB if that's your personal preference. 


In order
to run the program using GDB in the src folder hit the command

* gdb refreshFS

Once in the gdb environment you can run the command
* file refreshFS

This will allow you to set break points using the command 'b [line number]'.

To skip the break points once entering gdb just enter

 * run -s -d Mountingpoint
 * -s: Runs the file system as a single thread program
 * -d: Allows for debugging options
 * Mountdir: is the name of the folder that will be tourned to our mounting point.

ctrl c and q exits the program.



**To unmount use the instruction**

While in the directory containing the mountingPoint folder.

* fusermount -u mountingPoint



## About the File System

The base of our P2P file system started out from. LSYSFS, a less simple yet stupid file system.
LSYSFS is a in memory(all files disapear on dismount) file system that is as simple as it gets, it reads only one directory level down. With 
LSYSFS as a base we looked at how each function worked. What does each function need? What does it return?
Having a basic understanding of how the file system worked we could then build on top of it by looking at 
more feature complete file systems and how they handle routines that every file system needs. 


### Pre Req Knowledge

Before you dive in there are some things you must know.

# OpenDHT 
* OpenDHT stands for Distributed Hashtable. A distributed hashtable is essentially a hashtable that is shared among different IP addresses that all have access to the hashtable. It is distributed because the hashtable values are stored across the IP addresses. 

* OpenDHT works by connecting nodes that with different IP addresses such that the number of connections is minimized but all nodes can reach all values in the hashtable. The specific implementation of the connection algorithm is called Kademlia and uses an exponential function to determine the connections.

* To put a value on the hashtable, a node will send a put request using the OpenDHT API that specifies a key-value pair. That value will remain stored on that node's storage but the key and value will be accessible from all nodes connected to the hashtable. When another node performs a get request to request the data associated with a key, the OpenDHT follows a path of connections to a node that has the value stored on the computer and then returns the value following a set of connections.

* Everything is a file, even am a file
In Unix everything is a file. Yes that means a directory is a file too. 


* Paths
You need to know what's the difference between a relative path and an absolute path. 

https://stackoverflow.com/questions/46121000/what-is-an-absolute-pathname-vs-a-relative-pathname

Whenver you have to deal with an object(file or directory) in our filesystem, fuse will scan everything in a directory and return
the path of files and directories. Say you want to write some file, Fuse will send a write request
to our file system with the full path "/mountdir/filename". If your file is in the root of your file system it will look like "/filename".


* The difference between a Root Directory and Mounting Point

The difference between the mounting point and root directory is something that caused me to waste several weeks on a bug. Because I didn't have background information on this topic I was trying to solve a problem that was a deeper issue at hand. You may have not noticed it browsing other file systems but there should always be two parameters to give fuse. A mounting point and a root directory. Why is this? This is something you should explore on your own. But know, if understand this a lot of possible questions could be answered.


* Inodes
Files and their contents are not stored together but are found together with their index position. The relationship will help us find the file and its contents. Inodes are a data structure in unix type file systemsthat describe a file or directory. Inodes store the attributes and disk block locations
of the objects data. File-system object attributes may include metadata (times of last change, access, modification), as well as owner and permission data. 

Directories are lists of names assigned to inodes. A directory contains an entry for itself, its parent, and each of its children. As you have seen you can't create a directory or file of the same name because a list is maintaining each file and its unique name.

https://en.wikipedia.org/wiki/Inode


Below are some of the more important functions for our P2PFile System and how they work.

### getattr: 

getattr(const char *path, struct stat *statbuf)

This function gets the file attributes. Basically it tells the OS if this is a file or directory. It's perhaps the first step that hapens in fuse, if you don't make an init function.
"Also, it tells about the owner, group and permission bits of the object. This function is a vital one that without it we can’t have a functional filesystem" -SYSFS

The parameters for getattr Fuse functions provides the path to the file/directory being analyzed and a Stat pointer to the struct.

    struct stat {
        dev_t     st_dev;     /* ID of device containing file */
        ino_t     st_ino;     /* inode number */
        mode_t    st_mode;    /* protection */
        nlink_t   st_nlink;   /* number of hard links */
        uid_t     st_uid;     /* user ID of owner */
        gid_t     st_gid;     /* group ID of owner */
        dev_t     st_rdev;    /* device ID (if special file) */
        off_t     st_size;    /* total size, in bytes */
        blksize_t st_blksize; /* blocksize for file system I/O */
        blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
        time_t    st_atime;   /* time of last access */
        time_t    st_mtime;   /* time of last modification */
        time_t    st_ctime;   /* time of last status change */
    };
The getattr function requires you to fill out this information if you want to use the command ls -l in a terminal. If you use the stat or lstat system call and provide the full path and that stat pointer provided by fuse, the stat system call will fill out all the information of the above stat structure. You can ignore using the stat system call and manually fill out the details if you so chose to.

For our file system this was a neccessary function in order to let to file system differentiate between files and directories. For more information on the stat structure and system call check out the official man page. (A recurring theme you'll see here is looking at man pages to understand the parameters fuse passes to you.)
https://linux.die.net/man/2/stat



**openDHT**

With the default functionality out of the way, we also added some other super important openDHT functionality here. Because the getAttr function runs so many times we decided this function would be best at looking at any changes in our distributed hash table. Every time getAttr runs, we use the get functions from the openDHT api and see if new files and directories have been pushed. Once a new file or directory is detected we create the file or directory. A good way to force updates is to hit ls several times in the terminal in order to force a getAttr call.

### readir:

readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,struct fuse_file_info *fi)

This function takes usually takes place after the getattr function. Before moving into a directory fuse first needs to read the contents of the directory. The elements passed into this function are important as these are the structures required to read a directory. The function of the event readdir provides the operating system with the list of files/directories that reside in a given directory. A buffer, a fuse filler of dir t, an offset and structure of fuse file info.

The **buffer** is where we will fill the list of available files and directories that reside in the path given to us by fuse. Something else provided by fuse is the filler. With the filler function given to us by fuse we can use it to fill the buffer. In the readir function the first thing we will do is fill it with two directories "." and ".." explanation here (http://unix.stackexchange.com/a/101536).

Something to keep in mind is we need to get information about files in the directory which is where **fuse_file_info** comes into play. **fi** is a pointer to a structure of file information. The most important field is **fh otherwise known as the file handle id**.
https://libfuse.github.io/doxygen/structfuse__file__info.html 

A file handle or fh in our case is a File Structure that may contain a file descrptor usually stating whether a file is open or not. https://unix.stackexchange.com/questions/146113/file-handles-and-filenames. We get a pointer to this file structure because fh is a file handle id. We call this pointer dp. The id fh now in dp is used as an index with the syscall **readdir(dp)** we can then return what the file handle id represents. A structure of type dirent contaning 

    struct dirent {

               ino_t          d_ino;       /* Inode number */
         
               off_t          d_off;       /* Not an offset; see below */
               
               unsigned short d_reclen;    /* Length of this record */
               
               unsigned char  d_type;      /* Type of file; not supported
               
                                              by all filesystem types */
                                              
                                              
               char           d_name[256]; /* Null-terminated filename */
               
    };

http://man7.org/linux/man-pages/man3/readdir.3.html

So the readir(dp) call retrieved the above data and placed it in a pointer of type dirent. We will use this method and keep retriveing dirent structures containing all the information of the files in the directory. We fill in the buffer using the filler and passing the d_name in our de(directory entry) pointer. 

The first check is to see if the directory entry is null. If it's null something is wrong because every directory, as stated before should have "." and ".." as its first directory entries. Once we verify that the first de is not null we can loop and continue to use readdir(dp) for all of the entries until we reach the end which is null. In the loop we call filler, filling it with the de->dname and at the same time check if there are no errors.

With that in mind lets refresh what we did in this function. We made a directory pointer dp and set it to fh(another fuse function sets up fh), then we use that fh(file handle id) that's in dp to read the directory which returns the directory entry structure above. We place the name of the directory entry in a filler and loop until we reach the end of dp. In short this function puts names of null terminated files in a filler. 

You can see how this could be useful for a a file function like LS. 


### write

do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)

The write function is the most important part of our file system. This is where we part with providing Fuse with the neccessary information it requires. Normally this is where you would use the pwrite() system call and we do using **pwrite(fi->fh, buffer, size, offset)**. But because we are making a P2P Filesystem we could remove the function entirely which we did at one point. The reason behind this is because instead of writing to a file we need to take the content in the buffer and put it in our distributed hash table. 

https://linux.die.net/man/2/pwrite

First we ignore swp files that are created when writing a file with VIM or other text editors. Then we append to the buffer a time stamp. The reason for this is because if we are to modify any files the openDHT does not get the latest content, it instead randomly selects content from a provided key. That aside after appending the data we send the content to openDHT using the put function below.

    node.put(path, tempString.c_str(), [](bool success) 
	{
         std::cout << "\n\nnode.put(path, buffer) ------------ with " << (success ? "success" : "failure") << std::endl;
	     wait = 1;
	});
    
For more information on the OpenDHT API:
https://github.com/savoirfairelinux/opendht/wiki/API-Overview



### read

do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)

The read function, similar to the write function, also doesn't follow a normal format for a fuse file system. Because we want the content from our distributed hash table to read we don't have insterest in the provided buffer. For our read function we need to use the get function from the OpenDHT api. With it we use the path given as a key to retrieve the data from the hash table. The content retrieved will be in a different format, we turn it into a string and put it into the buffer in return the size. 

We had an issue with the size so when reading a file you will see some extra content being displayed along with the correct text. 


### mkdir

mkdir(const char *path, mode_t mode)

The Mkdir function is responsible for creating directories. The provided mode is responsible for the permission bits such as executable, read, or write for the owner, group, or other. Our mkdir is modified to send the path to the openDHT in order for other file systems to get the same directory.

    node.put("LIST_OF_DIRS", path, [](bool success) 
    {
        std::cout << "\n\nnode.put(LIST_OF_DIRS, path) ------------ with " << (success ? "success" : "failure") << std::endl;
        wait = 1;
    });
We have a hard coded List of Dirs key and put the name of all directories in it. We retrieve that list in our get attr function and create the directories there using the **mkdir()** system call. We pass the path which is in our DHT and a mode which we set as 0755 which is permission for read, write and execute. 

https://linux.die.net/man/1/mkdir

https://stackoverflow.com/questions/19547085/differences-between-chmod-755-vs-750-permissions-set



## Conclusion

As detailed above, you now get an overview of how we send and retrieve data to the distributed hash table. We send data on directory creation and when writing to a file. The important parts to send are the relative paths, and the buffer content when writing. We then retrieve that information in the getAttr function and create empty files and directories. On read we retreive the content associated to that relative path and place that in the buffer that is being read. Overall we have a nearly fully featured filesystem. Besides the size issue the only real missing features are symbolic links, chown, and renaming files which is technically impossible because openDHT does not allow keys to be renamed. That being said, you could rename, then modify the file, and that file will then be shared to others running our P2PFileSystem.

## Resources

IBM Developer: https://developer.ibm.com/articles/l-fuse/

* If you were looking for FUSE resources before hand this was perhaps the first link that popped up.
Don't listen to anything about the FUSE set up, that's an older version that caused us issues. Instead
just pay attention the function descriptions. That's probably the most useful information you'll get.



Big Brother File System: https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/

* The Big Brother File System is a Fuse Filesystem that does as the title implies. It captures your actions inside of a file system and outputs it to a seperate file. So if you read a file it will keep track of that, if you opened a a folder it keeps track of that. While that sounds simple, BBFS is actually quite complex but it's quite possibly the best resource on Fuse on the internet.  Joseph J. Pfeiffer, the proffessor who made the BBFS offered insight on some bugs I had building the Fuse File System. He is available for consulting at joseph@pfeifferfamily.net



Simple Yet Stupid File System: http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/

* The Simple Yet Stupid File System is exactly as it sounds, a simple file system that barely does anything. Every file is created and stored locally. It only works in the rootdirectory. With all its faults it's still a useful look at how simple instructions work. SYSFS should be the first place people look into when looking into FUSE.
