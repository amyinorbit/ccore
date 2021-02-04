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

bool cc_cfg_get_bool(const cc_cfg_t *cfg, const char *field, bool *out);
bool cc_cfg_get_int(const cc_cfg_t *cfg, const char *field, int *out);
bool cc_cfg_get_float(const cc_cfg_t *cfg, const char *field, float *out);
bool cc_cfg_get_double(const cc_cfg_t *cfg, const char *field, double *out);
bool cc_cfg_get_str(const cc_cfg_t *cfg, const char *field, const char **out);

#ifdef __cplusplus
} /* extern "C" */
#endif
