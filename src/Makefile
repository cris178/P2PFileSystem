COMPILER = g++
FILESYSTEM = refreshfs.cpp

build: $(FILESYSTEM)
		$(COMPILER) $(FILESYSTEM) -g -o  refreshFS -lopendht `pkg-config fuse --cflags --libs `
		echo "Mount template: ./refreshFS -f -s or -d [Mounting Point]"
		echo "Unmount template: fusermount -u [Mounting Point] or CTRL C on running program."

clean:
	rm refreshFS
