
CC = gcc
CFLAGS = -g -Wall -Wextra -pthread
OBJS = main.o event.o manager.o resource.o system.o

all: program

program: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o program

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) program
