CC = gcc -ggdb -g -Wall
xterm = xterm -geom 151x51 -bg black -fg white -name shelter

all: build build/sockets_send.o build/keyboard.o build/xterm_control.o build/utils.o build/graphics.o build/shelter.o build/mystore
	$(CC) build/sockets_send.o build/keyboard.o build/xterm_control.o build/utils.o build/graphics.o build/shelter.o -o shelter

build/mystore:
	$(CC) lib/mystore.c build/sockets_send.o -o build/mystore

build/shelter.o:
	$(CC) -c src/shelter.c -o build/shelter.o

build/graphics.o:
	$(CC) -c src/graphics.c -o build/graphics.o

build/utils.o:
	$(CC) -c src/utils.c -o build/utils.o

build/xterm_control.o:
	$(CC) -c lib/xterm_control.c -o build/xterm_control.o

build/keyboard.o:
	$(CC) -c lib/keyboard.c -o build/keyboard.o

build/sockets_send.o:
	$(CC) -c lib/sockets_send.c -o build/sockets_send.o

startserver:
	./build/mystore start &

stopserver:
	./build/mystore stop

run: all
	$(xterm) ./shelter

clean:
	rm -rf build
	rm shelter

build:
	mkdir build
