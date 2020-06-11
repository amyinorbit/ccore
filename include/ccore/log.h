//===--------------------------------------------------------------------------------------------===
// log.h - Logging utilities for the UNS
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG

#define CCINFO(fmt, ...)
#define CCWARN(fmt, ...)
#define CCERROR(fmt, ...)

#define CCASSERT(expr)

#else

#define VA_ARGS(...) , ##__VA_ARGS__
#define CCINFO(fmt, ...) cc_log(LOG_INFO, __FUNCTION__, __LINE__, fmt VA_ARGS(__VA_ARGS__))
#define CCWARN(fmt, ...) cc_log(LOG_WARN, __FUNCTION__, __LINE__, fmt VA_ARGS(__VA_ARGS__))
#define CCERROR(fmt, ...) cc_log(LOG_ERROR, __FUNCTION__, __LINE__, fmt VA_ARGS(__VA_ARGS__))

#define CCASSERT(expr) cc_assert(expr, #expr, __FILE__, __FUNCTION__, __LINE__)

#endif

#define CCUNREACHABLE() (__builtin_unreachable())

typedef enum log_level_e {
    LOG_INFO, LOG_WARN, LOG_ERROR
} log_level_t;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

/// Sets the function used to print log messages. Defaults to fprintf(stderr, ...).
void cc_set_printer(void (*handler)(const char *));

/// Prints a log message at [level], in function [unit] at [line].
void cc_log(log_level_t level, const char *unit, int line, const char *fmt, ...);
void cc_assert(bool expr, const char *readable, const char *file, const char *unit, int line);
void cc_print(const char *str);

#ifdef __cplusplus
} /* extern "C" */
#endif
