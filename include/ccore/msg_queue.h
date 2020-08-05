//===--------------------------------------------------------------------------------------------===
// msg_queue.h - non thread-safe, ring-buffer based messge queue
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <ccore/message.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CC_QUEUE_MAX
#define CC_QUEUE_MAX (128)
#endif

/// Message-passing queue. This is intened for inter-thread communication.
/// Allows a thread to publish messages to be picked up sequentially by other threads.
/// This is a one-direction structure and does not allow data to be sent back "upstream"
/// once a message has been processed by the consumer.
typedef struct ccqueue_s {
    uint8_t head, tail;
    ccmsg_t data[CC_QUEUE_MAX];
} ccqueue_t;

/// Initialises [queue] to a safe state.
void ccqueue_init(ccqueue_t *queue);

/// Flushes any events left in [queue] and deallocate any memory used by it.
void ccqueue_deinit(ccqueue_t *eq);

/// Pushes [msg] onto [queue] and returns true if there is space left, returns false otherwise.
bool ccqueue_push(ccqueue_t *queue, ccmsg_t msg);

/// Returns true if an message can be pulled from [queue] into [msg], false otherwise.
bool ccqueue_pull(ccqueue_t *queue, ccmsg_t *msg);

///
void ccqueue_clear(ccqueue_t *eq);


#ifdef __cplusplus
} /* extern "C" */
#endif
