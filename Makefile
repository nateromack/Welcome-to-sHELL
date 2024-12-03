shell: shell.o utilities.o builtin.o
    gcc -o shell shell.o utilities.o builtin.o

CPPFLAGS := -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L

shell.o: shell.c header.h
    gcc -c -g -Wall shell.c

utilities.o: utilities.c header.h
    gcc -c -g -Wall utilities.c

builtin.o: builtin.c header.h
    gcc -c -g -Wall builtin.c
clean:
    rm -f *.o shell
	