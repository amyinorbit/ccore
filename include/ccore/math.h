//===--------------------------------------------------------------------------------------------===
// math.h - math utilities somehow not part of the standard library
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

static inline int max_i(int a, int b) { return a > b ? a : b; }
static inline int min_i(int a, int b) { return a < b ? a : b; }

#ifdef __cplusplus
} /* extern "C" */
#endif
