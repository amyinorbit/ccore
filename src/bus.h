//===--------------------------------------------------------------------------------------------===
// ccbus - The 4th? 5th? Who knows? iteration of a thread-safe I/O bus
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent/FlyJsim
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/bus.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUS_LINE_CAPACITY (128)

typedef struct ccbus_s {
    pthread_mutex_t mt;
    uint64_t head;
    uint64_t tail;
    ccbus_msg_t data[BUS_LINE_CAPACITY];
} ccbus_t;


#ifdef __cplusplus
} /* extern "C" */
#endif
