//===--------------------------------------------------------------------------------------------===
// cfg - small configuration parser
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <ccore/log.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cc_cfg_s cc_cfg_t;

cc_cfg_t *cc_cfg_load(const char *path);
void cc_cfg_delete(cc_cfg_t *cfg);

bool cc_cfg_key_exists(const cc_cfg_t *cfg, const char *fmt, ...);
bool cc_cfg_get_bool(const cc_cfg_t *cfg, bool *out, const char *fmt, ...);
bool cc_cfg_get_int(const cc_cfg_t *cfg, int *out, const char *fmt, ...);
bool cc_cfg_get_float(const cc_cfg_t *cfg, float *out, const char *fmt, ...);
bool cc_cfg_get_double(const cc_cfg_t *cfg, double *out, const char *fmt, ...);
bool cc_cfg_get_str(const cc_cfg_t *cfg, const char **out, const char *fmt, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
