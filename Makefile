#
# Copyright (c) 2000 by Cesare Placanica
# All rights reserved.
#
#

CC = gcc
LOCALFLAGS = -g -W -Wall
LOCALINCS = -I.

# Define different object files for IPv4 and IPv6 versions
CLIENT_OBJECTS_IPV6 = client_ipv6.o
CLIENT_OBJECTS_IPV4 = client_ipv4.o
SERVER_OBJECTS_IPV6 = server_ipv6.o
SERVER_OBJECTS_IPV4 = server_ipv4.o

all: client_ipv6 client_ipv4 server_ipv6 server_ipv4

# IPv6 Client Target
client_ipv6: $(CLIENT_OBJECTS_IPV6)
	$(CC) -o $@ $(LOCALFLAGS) $(LOCALINCS) $(CLIENT_OBJECTS_IPV6)

# IPv4 Client Target
client_ipv4: $(CLIENT_OBJECTS_IPV4)
	$(CC) -o $@ $(LOCALFLAGS) $(LOCALINCS) $(CLIENT_OBJECTS_IPV4)

# IPv6 Server Target
server_ipv6: $(SERVER_OBJECTS_IPV6)
	$(CC) -o $@ $(LOCALFLAGS) $(LOCALINCS) $(SERVER_OBJECTS_IPV6)

# IPv4 Server Target
server_ipv4: $(SERVER_OBJECTS_IPV4)
	$(CC) -o $@ $(LOCALFLAGS) $(LOCALINCS) $(SERVER_OBJECTS_IPV4)

# Rule for building the IPv6 client object file
client_ipv6.o: client.c chat.h
	$(CC) $(LOCALFLAGS) $(LOCALINCS) -c -DIPV6_CHAT -o $@ client.c

# Rule for building the IPv4 client object file
client_ipv4.o: client.c chat.h
	$(CC) $(LOCALFLAGS) $(LOCALINCS) -c -o $@ client.c

# Rule for building the IPv6 server object file
server_ipv6.o: server.c chat.h
	$(CC) $(LOCALFLAGS) $(LOCALINCS) -c -DIPV6_CHAT -o $@ server.c

# Rule for building the IPv4 server object file
server_ipv4.o: server.c chat.h
	$(CC) $(LOCALFLAGS) $(LOCALINCS) -c -o $@ server.c

clean:
	rm -f *.o client_ipv* server_ipv*
