.PHONY: all
all: build/libraycast.a

build/libraycast.a: build/raycast.o
	ar rcs $@ $<

build/raycast.o: src/raycast.c include/raycast.h
	gcc -Wall -c -I include $< -o $@

.PHONY: clean
clean:
	rm build/raycast.o build/libraycast.a