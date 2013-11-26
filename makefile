CC = gcc

all: build build/keyboard.o build/xterm_control.o build/myui.o build/mystore
	$(CC) build/keyboard.o build/xterm_control.o build/myui.o -o myui -lm

build/mystore:
	$(CC) lib/mystore/mystore.c -o build/mystore

build/myui.o:
	$(CC) -c src/myui.c -o build/myui.o

build/xterm_control.o:
	$(CC) -c lib/xterm/xterm_control.c -o build/xterm_control.o 

build/keyboard.o:
	$(CC) -c lib/xterm/keyboard.c -o build/keyboard.o

clean:
	rm -rf build
	rm myui

build:
	mkdir build
