#
# Copyright (c) 2000, 2007 by Cesare Placanica
# All rights reserved.
#
#

CLIENT_OBJECTS=chatcli.o
IPV4_CLIENT_OBJECTS=chatcli_ipv4.o
SERVER_OBJECTS=chatser.o
IPV4_SERVER_OBJECTS=chatser_ipv4.o
CC = gcc
LOCALFLAGS = -g -W -Wall
LOCALINCS = -I.

.SUFFIXES: $(SUFFIXES)

client: $(CLIENT_OBJECTS)
	$(CC) -o $@ $(CLIENT_OBJECTS) $(LIBSPATH) $(LIBS)

client_ipv4: $(IPV4_CLIENT_OBJECTS)
	$(CC) -o $@ $(IPV4_CLIENT_OBJECTS) $(LIBSPATH) $(LIBS)

server: $(SERVER_OBJECTS)
	$(CC) -o $@ $(SERVER_OBJECTS) $(LIBSPATH) $(LIBS)

server_ipv4: $(IPV4_SERVER_OBJECTS)
	$(CC) -o $@ $(IPV4_SERVER_OBJECTS) $(LIBSPATH) $(LIBS)

all: client server client_ipv4 server_ipv4

clean:
	rm -f *.o client* server*

.c.o:
	$(CC) $(LOCALFLAGS) $(LOCALINCS) -c $<
