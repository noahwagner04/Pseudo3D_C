.PHONY: all
all: build/libraycaster.a

build/libraycaster.a: build/raycaster.o
	ar rcs $@ $<

build/raycaster.o: src/raycaster.c include/raycaster.h
	gcc -Wall -c -I include $< -o $@

.PHONY: clean
clean:
	rm build/raycaster.o build/libraycaster.a