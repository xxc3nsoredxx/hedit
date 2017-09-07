OBJS = main.o
CC = gcc
CFLAGS = -Wall -Wextra -pedantic-errors -std=c89 -c
LFLAGS = -lncurses

all: hedit

hedit: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o hedit

main.o: main.c
	$(CC) $(CFLAGS) main.c
