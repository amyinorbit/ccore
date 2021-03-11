//===--------------------------------------------------------------------------------------------===
// run_loop.h - Event/run loop for ARINC modules
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cc_run_loop_t cc_run_loop_t;
typedef void * cc_run_loop_handle_t;

cc_run_loop_t *cc_run_loop_new(const char *name);

void cc_run_loop_delete(cc_run_loop_t *rl);

cc_run_loop_handle_t cc_run_loop_register(
    cc_run_loop_t *rl,
    void (*ticker)(uint64_t, void *),
    double freq,
    void *data
);
    
void cc_run_loop_unregister(cc_run_loop_t *rl, cc_run_loop_handle_t handle);

#ifdef __cplusplus
} // extern "C"
#endif
