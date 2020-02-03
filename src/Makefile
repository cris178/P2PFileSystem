COMPILER = gcc
FILESYSTEM = P2PFS.c

build: $(FILESYSTEM)
		$(COMPILER) $(FILESYSTEM) -o P2PFS `pkg-config fuse --cflags --libs`
		echo 'Mount template: ./P2PFS -f [Mounting Point]'

clean:
	rm P2PFS