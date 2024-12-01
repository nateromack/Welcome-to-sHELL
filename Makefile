shell: shell.o utilities.o
	gcc -o shell shell.o utilities.o

CPPFLAGS := -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L

shell.o: shell.c header.h
	gcc -c -g -Wall shell.c

utilities.o: utilities.c header.h
	gcc -c -g -Wall utilities.c