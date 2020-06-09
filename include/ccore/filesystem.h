//===--------------------------------------------------------------------------------------------===
// filesystem.h - thin file system abstraction layer
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CCFS_USE_PLATORM_SEP
#ifdef WIN32
#define CCFS_PATH_SEP '\\'
#else
#define CCFS_PATH_SEP '/'
#endif
#else
#define CCFS_PATH_SEP '/'
#endif

typedef enum { CCFS_READ, CCFS_WRITE, CCFS_APPEND } ccfs_mode_t;

/// Concatenates a null-terminated list of path components [...] and returns the path size.
size_t ccfs_path_concat(char *out, size_t size, ...);

/// Returns whether [path] exists.
bool ccfs_path_exists(const char *path);

/// Trims [path] in place by [num_dirs] path components.
void ccfs_path_rtrim_i(char *path, int num_dirs);

/// Attemts to open [path] using [mode].
FILE* ccfs_file_open(const char *path, ccfs_mode_t mode);

#ifdef __cplusplus
} /* extern "C" */
#endif
