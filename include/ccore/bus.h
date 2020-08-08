//===--------------------------------------------------------------------------------------------===
// ccbus - A single-line thread-safe bus
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent/FlyJsim
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ccbus_msg_s {
    uint32_t label;
    union {
        int64_t i64;
        uint64_t u64;
        double f64;
        double f32;
    } data;
} ccbus_msg_t;

typedef struct ccbus_s ccbus_t;

ccbus_t *ccbus_new();
void ccbus_delete(ccbus_t *bus);

void ccbus_lock(ccbus_t *bus);
void ccbus_unlock(ccbus_t *bus);

void ccbus_send_i64(ccbus_t *bus, uint32_t label, int64_t data);
void ccbus_send_u64(ccbus_t *bus, uint32_t label, uint64_t data);
void ccbus_send_f64(ccbus_t *bus, uint32_t label, double data);
void ccbus_send_f32(ccbus_t *bus, uint32_t label, float data);

bool ccbus_receive(ccbus_t *bus, ccbus_msg_t *msg);
bool ccbus_receive_fast(ccbus_t *bus, ccbus_msg_t *msg);

#ifdef __cplusplus
} /* extern "C" */
#endif
