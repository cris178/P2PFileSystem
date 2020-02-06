COMPILER = gcc
FILESYSTEM = P2PFS.c

build: $(FILESYSTEM)
		$(COMPILER) $(FILESYSTEM) -o P2PFS `pkg-config fuse --cflags --libs`
		echo "Mount template: ./P2PFS -f -s or -d [Mounting Point]"
		echo "Unmount template: fusermount -u [Mounting Point] or CTRL C on running program."

clean:
	rm P2PFS