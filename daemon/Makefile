CC = gcc

install: daemon

daemon: daemon.o
	$(CC) -o bin/daemon bin/daemon.o

daemon.o:	daemon.c 
	$(CC) -o bin/daemon.o -c daemon.c -D_XOPEN_SOURCE

clean: 
	rm bin/daemon*

init:
	mkdir bin
