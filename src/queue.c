/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  RNode Linux
 *
 *  Copyright (c) 2025 Belousov Oleg aka R1CBU
 */

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"
#include "util.h"
#include "rnode.h"
#include "csma.h"
#include "sx126x.h"

typedef struct item_t {
    uint8_t         *data;
    size_t          len;
    struct item_t   *next;
} item_t;

static struct item_t    *head = NULL;
static struct item_t    *tail = NULL;
static pthread_mutex_t  mux;

static bool             tx_enable = false;
static uint64_t         tx_delay;
static uint64_t         rssi_delay;

static bool send_packet() {
    bool res = false;

    pthread_mutex_lock(&mux);

    if (head != NULL) {
        item_t *item = head;

        pthread_mutex_unlock(&mux);

        printf("Queue: send packet\n");
        rnode_to_air(item->data, item->len);

        pthread_mutex_lock(&mux);

        if (head == tail) {
            head = tail  = NULL;
        } else {
            head = head->next;
        }

        free(item->data);
        free(item);

        res = true;
    }

    pthread_mutex_unlock(&mux);
    csma_update_airtime();

    return res;
}

static void * queue_worker(void *p) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (true) {
        if (tx_enable) {
            uint64_t now = get_time();

            if (now > tx_delay) {
                uint32_t delay = csma_get_cw();

                send_packet();
                tx_delay = now + delay;
            } else if (now > rssi_delay) {
                rssi_delay = now + 1000;

                csma_update_current_rssi();
            }
        } else {
            usleep(1000);
        }
    }
}

void queue_init() {
    pthread_mutex_init(&mux, NULL);

    pthread_t thread;

    pthread_create(&thread, NULL, queue_worker, NULL);
    pthread_detach(thread);
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

    printf("Queue: add packet\n");
}

void queue_medium_state(bool free) {
    tx_enable = free;

    if (free) {
        uint32_t delay = csma_get_cw();

        tx_delay = get_time() + delay;
    }
}
