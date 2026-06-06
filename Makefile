CC = gcc
CFLAGS = -Wall -Wextra -lpthread

all:
	$(CC) $(CFLAGS) -o portaria src/portaria.c

clean:
	rm -f portaria
