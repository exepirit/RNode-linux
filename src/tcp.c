/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  RNode Linux
 *
 *  Copyright (c) 2025 Belousov Oleg aka R1CBU
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>

#include "util.h"
#include "kiss.h"
#include "rnode.h"

static struct sockaddr_in   address;
static int                  server_fd;
static int                  client_fd = 0;
static uint8_t              buf_in[MTU] = {0};

void tcp_init(uint32_t port) {
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int on = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "TCP bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        syslog(LOG_ERR, "TCP listen");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "TCP listening on port %d", port);
}

void tcp_read() {
    int addrlen = sizeof(address);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        syslog(LOG_ERR, "TCP accept");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Client connected");

    while (true) {
        int res = read(client_fd, buf_in, sizeof(buf_in));

        if (res <= 0) {
            syslog(LOG_INFO, "Client disconnected");
            close(client_fd);
            break;
        }

        kiss_decode(buf_in, res);
    }
}

void tcp_send(char *buf, size_t len) {
    if (client_fd) {
        send(client_fd, buf, len, 0);
    }
}
