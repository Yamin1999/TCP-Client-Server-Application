CC = gcc
CFLAGS = -Wall -Wextra
MYSQL_FLAGS = $(shell mysql_config --cflags --libs)

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(MYSQL_FLAGS)

clean:
	rm -f client server

.PHONY: all clean