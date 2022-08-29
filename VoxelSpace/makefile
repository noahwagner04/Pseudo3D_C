.PHONY: all
all: build/libvoxelspace.a

build/libvoxelspace.a: build/voxelspace.o
	ar rcs $@ $<

build/voxelspace.o: src/voxelspace.c include/voxelspace.h
	gcc -Wall -c -I include $< -o $@

.PHONY: clean
clean:
	rm build/voxelspace.o build/libvoxelspace.a