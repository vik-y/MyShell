# Please refer to make-utility-notes.pdf - also in the zip file

LIB=-lreadline -lcurses    #read gcc manpages
CFLAGS=-DUNIX -ansi -pedantic-errors #read gcc manpages
DEBUG=-g
#DEBUG=

all: shell


shell:	shell.c parse.c parse.h
	gcc $(CFLAGS) $(DEBUG) shell.c parse.c -o shell $(LIB)
clean:
	rm -f shell *~

