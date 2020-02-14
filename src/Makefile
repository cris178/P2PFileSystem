COMPILER = gcc
FILESYSTEM = refreshfs.c

build: $(FILESYSTEM)
		$(COMPILER) $(FILESYSTEM) -g -o refreshFS `pkg-config fuse --cflags --libs`
		echo "Mount template: ./refreshFS -f -s or -d [Mounting Point]"
		echo "Unmount template: fusermount -u [Mounting Point] or CTRL C on running program."

clean:
	rm refreshFS