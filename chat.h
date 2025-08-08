/* *
 * Name: chat.h                                                     *
 *                                                                  *
 * Description: chat include file                                   *
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

#ifndef __CHAT_H
#define __CHAT_H

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXCHR 256
#define MAXCON 5
#define ACK_S "OK"
#define MSG_C "exit\n"

#ifdef IPV6_CHAT
typedef struct sockaddr_in6 internet_domain_sockaddr;
#else
typedef struct sockaddr_in internet_domain_sockaddr;
#endif

#endif
