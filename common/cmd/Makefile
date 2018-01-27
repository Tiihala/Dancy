cmd: main.o program.o
	$(CC) -o cmd main.o program.o

main.o: main.c program.h
	$(CC) -Wall -Wextra -Wwrite-strings -pedantic -c main.c

program.o: program.c program.h
	$(CC) -Wall -Wextra -Wwrite-strings -pedantic -c program.c

.PHONY: clean
clean:
	$(RM) cmd main.o program.o
