//===--------------------------------------------------------------------------------------------===
// debug.c - Stack trace and other utilitiess
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/debug.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdio.h>

#define MAX_STACK_DEPTH 32

void cc_print_stack() {
    void *array[MAX_STACK_DEPTH];
    size_t size = backtrace(array, MAX_STACK_DEPTH);
    fprintf(stderr, "=== stack trace ===\n");
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}
