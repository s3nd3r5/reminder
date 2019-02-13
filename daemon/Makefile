CC = gcc
CFLAGS = `pkg-config --cflags libnotify`
FLAGS = -D_XOPEN_SOURCE
INC = `pkg-config --libs libnotify`

install: daemon

daemon: daemon.o
	$(CC) $(CFLAGS) -o bin/daemon bin/daemon.o $(INC) 

daemon.o:	daemon.c 
	$(CC) $(CFLAGS) -o bin/daemon.o -c daemon.c $(FLAGS) $(INC)

clean: 
	rm bin/daemon*

init:
	mkdir bin
