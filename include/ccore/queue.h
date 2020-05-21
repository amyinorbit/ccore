//===--------------------------------------------------------------------------------------------===
// events.h - mostly just defines for the different kind of events
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <pthread.h>
#include <stdbool.h>
#include <ccore/message.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CC_QUEUE_MAX
#define CC_QUEUE_MAX (128)
#endif

/// Thread-safe message-passing queue. This is intened for inter-thread communication.
/// Allows a thread to publish messages to be picked up sequentially by other threads.
/// This is a one-direction structure and does not allow data to be sent back "upstream"
/// once a message has been processed by the consumer.
typedef struct ccqueue_s {
    pthread_cond_t cv;
    pthread_mutex_t mutex;

    uint8_t head, tail;
    ccmsg_t data[CC_QUEUE_MAX];
} ccqueue_t;

/// Initialises [queue] to a safe state.
void ccqueue_init(ccqueue_t *queue);

/// Flushes any events left in [queue] and deallocate any memory used by it.
void ccqueue_deinit(ccqueue_t *eq);

/// Pushes [event] onto [queue] and notifies threads waiting on [queue] and
////returns true immediately. If there is no space available in [queue], returns false.
bool ccqueue_push(ccqueue_t *queue, ccmsg_t event);

/// Pauses the calling thread until an event is pushed to [queue], then returns that event.
/// If there are already events in [queue], returns the first one immediately.
ccmsg_t ccqueue_wait(ccqueue_t *queue);

///
void ccqueue_clear(ccqueue_t *eq);


#ifdef __cplusplus
} /* extern "C" */
#endif
