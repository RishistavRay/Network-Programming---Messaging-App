CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

all: server client

server: server.c helper.c helper.h
	$(CC) $(CFLAGS) -o server server.c helper.c

client: client.c helper.c helper.h
	$(CC) $(CFLAGS) -o client client.c helper.c

clean:
	rm -f server client
