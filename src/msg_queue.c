//===--------------------------------------------------------------------------------------------===
// events.c - Implementation of a thread-safe message queue
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/msg_queue.h>
#include <ccore/log.h>

void ccqueue_init(ccqueue_t *q) {
    CCASSERT(q);
    q->head = 0;
    q->tail = 0;
}

void ccqueue_deinit(ccqueue_t *q) {
    CCASSERT(q);
    q->head = 0;
    q->tail = 0;
}

void ccqueue_clear(ccqueue_t *q) {
    CCASSERT(q);
    q->head = q->tail = 0;
}

bool ccqueue_push(ccqueue_t *q, ccmsg_t event) {
    CCASSERT(q);
    uint8_t new_head = (q->head + 1) % CC_QUEUE_MAX;
    if(new_head == q->tail)  {
        return false;
    }
    q->data[q->head] = event;
    q->head = new_head;
    return true;
}

bool ccqueue_pull(ccqueue_t *q, ccmsg_t *msg) {
    CCASSERT(q);
    CCASSERT(msg);
    if(q->head == q->tail) return false;

    *msg = q->data[q->tail];
    q->tail = (q->tail + 1) % CC_QUEUE_MAX;
    return true;
}
