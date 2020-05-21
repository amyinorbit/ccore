//===--------------------------------------------------------------------------------------------===
// events.c - Implementation of a thread-safe message queue
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/queue.h>
#include <ccore/log.h>

void ccqueue_init(ccqueue_t *q) {
    CCASSERT(q);
    pthread_cond_init(&q->cv, NULL);
    pthread_mutex_init(&q->mutex, NULL);
    q->head = 0;
    q->tail = 0;
}

void ccqueue_deinit(ccqueue_t *q) {
    CCASSERT(q);
    pthread_cond_destroy(&q->cv);
    pthread_mutex_destroy(&q->mutex);
    q->head = 0;
    q->tail = 0;
}

void ccqueue_clear(ccqueue_t *q) {
    pthread_mutex_lock(&q->mutex);
    q->head = q->tail = 0;
    pthread_mutex_unlock(&q->mutex);
}

bool ccqueue_push(ccqueue_t *q, ccmsg_t event) {
    pthread_mutex_lock(&q->mutex);
    uint8_t new_head = (q->head + 1) % CC_QUEUE_MAX;
    if(new_head == q->tail)  {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }
    q->data[q->head] = event;
    q->head = new_head;
    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->cv);
    return true;
}

ccmsg_t ccqueue_wait(ccqueue_t *q) {
    pthread_mutex_lock(&q->mutex);
    while(q->head == q->tail) {
        pthread_cond_wait(&q->cv, &q->mutex);
    }

    ccmsg_t out = q->data[q->tail];
    q->tail = (q->tail + 1) % CC_QUEUE_MAX;
    pthread_mutex_unlock(&q->mutex);
    return out;
}
