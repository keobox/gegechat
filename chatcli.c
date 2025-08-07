/* *
 * Name: chatcli.c                                                  *
 *                                                                  *
 * Version: 1.5.2                                                   *
 *                                                                  *
 * Description: chat client                                         *
 *                                                                  *
 * To compile: cc chatcli.c -o name                                 *
 *                                                                  *
 * Copyright (C) 2007 Cesare Placanica                              *
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
#include <sys/wait.h>

/* ipv6 aware with mapped address */

void usage(char *cmd) { printf("USAGE:\n%s <hostname>\n", cmd); }

int main(int argc, char *argv[]) {
    int sd, cont, status, pid, errnum;
    char bufferIn[MAXCHR];
    char bufferOut[MAXCHR];
    struct sockaddr_in6 srv;
    struct hostent *hp;

    if (argc != 2) {
        usage(argv[0]);
        exit(0);
    }

    status = 0;
    memset((char *)&srv, 0, sizeof(srv));
    sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("C: socket error");
        exit(0);
    }
    hp = getipnodebyname(argv[1], AF_INET6, AI_DEFAULT, &errnum);
    if (hp == NULL) {
        printf("C: host not available\n");
        exit(1);
    }
    memset(&srv, 0, sizeof(srv));
    srv.sin6_family = hp->h_addrtype;
    srv.sin6_port = htons(5900);
    memcpy((void *)&srv.sin6_addr, (void *)hp->h_addr, hp->h_length);
    freehostent(hp);
    hp = NULL;
    if (connect(sd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("C: connect error");
        exit(2);
    } else {
        printf("connected...\n");
        printf("\nWelcome to GegeChat\n\n");
        cont = 1;
        pid = fork();
        if (pid < 0) {
            perror("C: fork error");
            exit(3);
        }

        do {
            if (pid == 0) {
                /* child reading task */
                memset(bufferIn, 0, MAXCHR);
                if (recv(sd, bufferIn, MAXCHR, 0) < 0) {
                    perror("C: child recv error");
                } else {
                    if (strcmp(bufferIn, ACK_S) == 0) {
                        printf("C: child terminated\n");
                        exit(4);
                    } else {
                        printf("\n%s", bufferIn);
                    }
                }
            } else {
                /* parent writing task */
                printf("C: Msg: ");
                memset(bufferOut, 0, MAXCHR);
                fgets(bufferOut, sizeof(bufferOut), stdin);
                if (send(sd, bufferOut, strlen(bufferOut), 0) < 0) {
                    perror("C: parent send error");
                }
                if (strcmp(bufferOut, MSG_C) == 0) {
                    cont = 0;
                }
            }
        } while (cont);
        if (wait(&status) < 0) {
            printf("C: parent wait status 0x%X\n", status);
            perror("C: parent wait error");
        } else {
            printf("C: disconnect from server\n");
            close(sd);
        }
    } /* else */
    return 0;
} /* main */
