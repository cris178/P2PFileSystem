# P2PFileSystem
A Peer to Peer file system built on top of FUSE (File System in User Space ). 
P2PFS is an inmemory file system meaning that all files and directories created 
will not exist once that filesystem is unmounted.


# Installing FUSE
In order to get the filesystem working several conditions need to be met.

Ubuntu: FUSE is not Windows supported by default. The FUSE Kernel Module is not installed and even if you use 
the Windows Subsystem for Linux, you'll run to this problem.

While FUSE comes pre installed with most versions of Linux we still need extra supportting software to support building our file system.
With a fresh install of Ubuntu, download and install the following.


## FUSE Requirements

On the Ubuntu Terminal.

* sudo apt-get update
* sudo apt-get install fuse
* sudo apt-get install libfuse-dev

Download the fuse library of functions
https://github.com/libfuse/libfuse

In order to test if libfuse is working well
Will need the following

meson and ninja needed to build libfuse 
* sudo apt-get install meson

For Meson to work we need to build essentials
* sudo apt-get upgrade
* sudo apt-get install build essential
* sudo apt-get install pkg-config

Need Python3 to run the tests to see if FUSE is working
* sudo apt-get install python3.7
* sudo apt-get install python3-pip
* pip3 install --user pytest

Then just follow instructions on libfuse github readMe
until you see passing tests. Make sure a majority of the tests pass.

Now your OS is ready to support a FUSE filesystem. 



## Building the File System
The src Folder contains the program for our FUSE file system. 

Once in src just run the Make file using the command Make.
Once refreFS wil be the program that will be ran in order
to build the filesystem.

## Testing and Running

Currently we are debugging the program using GDB. In order
to run the program using GDB in the src folder hit the command

* gdb refreshFS

Once in the gdb environment you can run the command
* file refreshFS

This will allow you to set break points using the command 'b [line number]'.

To skip the break points once entering gdb just enter

 * run -s -d Mountdir
 * -s: Runs the file system as a single thread program
 * -d: Allows for debugging options
 * Mountdir: is the name of the folder that will be tourned to our mounting point.

ctrl c and q exits the program.




To unmount use the instruction

* fusermount -u [mounting point]




## Resources

IBM Developer: https://developer.ibm.com/articles/l-fuse/

* If you were looking for FUSE resources before hand this was perhaps the first link that popped up.
Don't listen to anything about the FUSE set up, that's an older version that caused us issues. Instead
just pay attention the function descriptions. That's probably the most useful information you'll get.

