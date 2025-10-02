CC=gcc
CFLAG=-Wall -std=c99 -D_POSIX_C_SOURCE=200809L -pedantic -g

mymake: mymake.o graphstruct.o processFile.o processCommand.o
	$(CC) $(CFLAG) mymake.o graphstruct.o processFile.o processCommand.o -o mymake
%.o: %.c mymake.h
	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o mymake