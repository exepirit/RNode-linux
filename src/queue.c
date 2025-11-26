/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  RNode Linux
 *
 *  Copyright (c) 2025 Belousov Oleg aka R1CBU
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct item_t {
    uint8_t         *data;
    size_t          len;
    struct item_t   *next;
} item_t;

static struct item_t    *head = NULL;
static struct item_t    *tail = NULL;
static pthread_mutex_t  mux;

void queue_init() {
    pthread_mutex_init(&mux, NULL);
}

void queue_push(const uint8_t *buf, size_t len) {
    item_t *item = malloc(sizeof(item_t));

    item->data = malloc(len);
    item->len = len;

    memcpy(item->data, buf, len);

    pthread_mutex_lock(&mux);

    if (head == NULL && tail == NULL) {
        head = tail = item;
    } else {
        tail->next = item;
        tail = item;
    }

    pthread_mutex_unlock(&mux);
}
