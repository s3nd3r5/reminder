CC = gcc

install: remindme

remindme: remindme.o
	$(CC) -o bin/remindme bin/remindme.o
remindme.o:	remindme.c
	$(CC) -o bin/remindme.o -c remindme.c -D_XOPEN_SOURCE

clean: 
	rm bin/remindme*

init:
	mkdir bin
