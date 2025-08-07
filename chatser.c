/*
 *                                                                  *
 * Name: chatser.c                                                  *
 *                                                                  *
 * Version: 1.5.2                                                   *
 *                                                                  *
 * Description: chat server                                         *
 *                                                                  *
 * To compile: cc chatser.c -o name                                 *
 *                                                                  *
 * Copyright (C) 2000 Cesare Placanica                              *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License      *
 * as published by the Free Software Foundation; either version 2   *
 * of the License, or (at your option) any later version.           *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "chat.h"
#include <stdlib.h>

/* ipv6 aware with mapped address */

int nClient = 0;
char buffer[MAXCHR];
char message[MAXCHR];

int openSocket(struct sockaddr_in6 *addr) {
    int sd;

    memset(addr, 0, sizeof(*addr));
    sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("S: openSocket socket error");
        return -1;
    } else {
        printf("S: openSocket socket OK\n");
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(5900);
        addr->sin6_addr = in6addr_any;
        if (bind(sd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
            perror("S: openSocket bind error");
            return -1;
        } else {
            printf("S: openSocket bind OK\n");
            printf("S: passive socket opened\n");
            return sd;
        }
    }
}

int freeConnections(int *fd) {
    int fdlib = 0;

    while ((fd[fdlib] >= 0) && (fdlib < MAXCON)) {
        fdlib++;
    }
    if (fdlib < MAXCON) {
        return fdlib;
    } else {
        return -1;
    }
}

void dispatch(int *fd, int i) {
    int k;

    memset(message, 0, MAXCHR);
    sprintf(message, "C%d: %s", i + 1, buffer);
    for (k = 0; k < MAXCON; k++) {
        if ((k != i) && (fd[k] > -1)) {
            if (send(fd[k], message, strlen(message), 0) < 0) {
            }
            perror("S: dispatch send error");
        }
    }
}

int communication(int *fd, int i) {
    int out = 0;

    memset(buffer, 0, MAXCHR);
    if (recv(fd[i], buffer, MAXCHR, 0) < 0) {
        perror("S: communication recv error");
    } else {
        printf("S: %s", buffer);
        if (nClient > 1) {
            dispatch(fd, i);
        }
        if (strcmp(buffer, MSG_C) == 0) {
            if (send(fd[i], ACK_S, sizeof(ACK_S), 0) < 0) {
                perror("S: communication send error");
            } else {
                printf("S: send ACK to client %d\n", i + 1);
                out = -1;
            }
        }
    }
    return out;
}

int main() {
    int sockfd, newsockfd;
    int nfds;
    socklen_t cliLen;
    int i;
    int fd[MAXCON];
    fd_set rfds;
    fd_set afds;
    struct sockaddr_in6 serAddr;
    struct sockaddr_in6 cliAddr;

    if ((sockfd = openSocket(&serAddr)) < 0) {
        exit(0);
    }
    if (listen(sockfd, MAXCON) < 0) {
        perror("S: listen error");
        exit(1);
    } else {
        printf("S: listening...\n");
    }

    nfds = FD_SETSIZE;

    for (i = 0; i < MAXCON; i++) {
        fd[i] = -1;
    }

    /* PASSIVE SOCKET MASK INITIALIZATION */
    FD_ZERO(&afds);

    /* PASSIVE SOCKET MASK SET */
    FD_SET(sockfd, &afds);

    /* CONNECTIONS MANAGEMENT LOOP */
    while (1) {
        /* COPIES DUMMY MASK IN THE READ MASK */
        memcpy((char *)&rfds, (char *)&afds, sizeof(rfds));

        /* BLOCKING SELECT */
        if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <
            0) {
            perror("S: main select error");
        }

        /* NEW CONNECTIONS MANAGEMENT */
        if (FD_ISSET(sockfd, &rfds)) {
            if ((i = freeConnections(fd)) < 0) {
                printf("S: non ci sono canali liberi\n");
            } else {
                cliLen = sizeof(cliAddr);
                memset((char *)&cliAddr, 0, sizeof(cliAddr));
                newsockfd =
                    accept(sockfd, (struct sockaddr *)&cliAddr, &cliLen);
                if (newsockfd < 0) {
                    perror("S: main accept error");
                } else {
                    FD_SET(newsockfd, &afds);
                    fd[i] = newsockfd;
                    nClient += 1;
                    printf("S: client %d connected", i + 1);
                    printf(" nClient %d\n", nClient);
                }
            }
        }

        /* CLIENTS CONNECTED MANAGEMENT */
        for (i = 0; i < MAXCON; i++) {
            if (fd[i] > -1) {
                if (FD_ISSET(fd[i], &rfds)) {
                    if (communication(fd, i) < 0) {
                        FD_CLR(fd[i], &afds);
                        close(fd[i]);
                        fd[i] = -1;
                        nClient -= 1;
                        printf("S: client %d disconnected", i + 1);
                        printf(" nClient %d\n", nClient);
                    }
                }
            }
        } /* for */
    } /* while */
    return 0;
} /* main */
