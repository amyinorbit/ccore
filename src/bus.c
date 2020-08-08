//===--------------------------------------------------------------------------------------------===
// ccbus - single-line bus
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent/FlyJsim
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "bus.h"
#include <ccore/log.h>
#include <ccore/memory.h>

ccbus_t *ccbus_new() {
    ccbus_t *bus = cc_alloc(sizeof(ccbus_t));
    pthread_mutex_init(&bus->mt, NULL);
    bus->head = 0;
    bus->tail = 0;
    return bus;
}

void ccbus_delete(ccbus_t *bus) {
    CCASSERT(bus);
    pthread_mutex_destroy(&bus->mt);
}

void ccbus_lock(ccbus_t *bus) {
    CCASSERT(bus);
    pthread_mutex_lock(&bus->mt);
}

void ccbus_unlock(ccbus_t *bus) {
    CCASSERT(bus);
    pthread_mutex_unlock(&bus->mt);
}

static ccbus_msg_t *ccbus_new_msg(ccbus_t *bus) {
    uint64_t new_head = (bus->head + 1) % BUS_LINE_CAPACITY;
    if(new_head == bus->tail) {
        CCWARN("Bus %p overflow", bus);
        return NULL;
    }
    ccbus_msg_t *msg = bus->data + bus->head;
    bus->head = new_head;
    return msg;
}

void ccbus_send_i64(ccbus_t *bus, uint32_t label, int64_t data) {
    CCASSERT(bus);
    ccbus_lock(bus);
    ccbus_msg_t *msg = ccbus_new_msg(bus);
    if(msg) {
        msg->label = label;
        msg->data.i64 = data;
    }
    ccbus_unlock(bus);
}

void ccbus_send_u64(ccbus_t *bus, uint32_t label, uint64_t data) {
    CCASSERT(bus);
    ccbus_lock(bus);
    ccbus_msg_t *msg = ccbus_new_msg(bus);
    if(msg) {
        msg->label = label;
        msg->data.u64 = data;
    }
    ccbus_unlock(bus);
}

void ccbus_send_f64(ccbus_t *bus, uint32_t label, double data) {
    CCASSERT(bus);
    ccbus_lock(bus);
    ccbus_msg_t *msg = ccbus_new_msg(bus);
    if(msg) {
        msg->label = label;
        msg->data.f64 = data;
    }
    ccbus_unlock(bus);
}

void ccbus_send_f32(ccbus_t *bus, uint32_t label, float data) {
    CCASSERT(bus);
    ccbus_lock(bus);
    ccbus_msg_t *msg = ccbus_new_msg(bus);
    if(msg) {
        msg->label = label;
        msg->data.f32 = data;
    }
    ccbus_unlock(bus);
}


bool ccbus_receive(ccbus_t *bus, ccbus_msg_t *msg) {
    CCASSERT(bus);
    ccbus_lock(bus);
    if(bus->head == bus->tail) {
        ccbus_unlock(bus);
        return false;
    }

    *msg = bus->data[bus->tail];
    bus->tail = (bus->tail + 1) % BUS_LINE_CAPACITY;
    ccbus_unlock(bus);
    return true;
}

bool ccbus_receive_fast(ccbus_t *bus, ccbus_msg_t *msg) {
    CCASSERT(bus);
    if(bus->head == bus->tail) {
        ccbus_unlock(bus);
        return false;
    }

    *msg = bus->data[bus->tail];
    bus->tail = (bus->tail + 1) % BUS_LINE_CAPACITY;
    return true;
}
