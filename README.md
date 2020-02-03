# P2PFileSystem
A Peer to Peer file system built on top of FUSE (File System in User Space )


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

Then just follow instructions on libfuse github
until you see passing tests. Make sure a majority of the tests pass.

Now your OS is ready to support a FUSE filesystem. 



## Building the File System
The src Folder contains the program for our FUSE file system. 

In order to mount the file system build the program using gcc 

* gcc P2PFS.c -o P2PFS `pkg-config fuse --cflags --libs`

Next you will run the program
* ./P2PFS -f [mounting point]       -f is to see print statements while program is running
                                    -mounting point is the name to an empty folder you select to turn into our FS

The program will run displaying a bunch of print statements.
Open another tab on your Terminal, hit ls -lr to see if the mounting point is now a file system.
Also, you can check if a file system is created using the regular Ubuntu UI for the file explorer.


To unmount use the instruction

fusermount -u [mounting point]

or

ctrl c on the first tab with the print statements.


## Resources

IBM Developer: https://developer.ibm.com/articles/l-fuse/

* If you were looking for FUSE resources before hand this was perhaps the first link that popped up.
Don't listen to anything about the FUSE set up, that's an older version that caused us issues. Instead
just pay attention the function descriptions. That's probably the most useful information you'll get.

