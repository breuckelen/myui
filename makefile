CC = gcc -ggdb -g

all: build build/keyboard.o build/xterm_control.o build/myui.o build/mystore
	$(CC) build/keyboard.o build/xterm_control.o build/myui.o -o myui

build/mystore:
	$(CC) lib/mystore.c -o build/mystore

build/myui.o:
	$(CC) -c src/myui.c -o build/myui.o

build/xterm_control.o:
	$(CC) -c lib/xterm_control.c -o build/xterm_control.o

build/keyboard.o:
	$(CC) -c lib/keyboard.c -o build/keyboard.o

clean:
	rm -rf build
	rm myui

build:
	mkdir build
