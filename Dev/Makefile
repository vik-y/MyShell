# Please refer to make-utility-notes.pdf - also in the zip file

LIB=-lreadline -lcurses    #read gcc manpages
CFLAGS=-DUNIX  #read gcc manpages
DEBUG=-g
#DEBUG=

all: shell


shell:	shell.c parse.c jobs.c linkedList.c
	gcc $(CFLAGS) $(DEBUG) shell.c parse.c jobs.c linkedList.c -o shell $(LIB)
clean:
	rm -f shell *~

