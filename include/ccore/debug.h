//===--------------------------------------------------------------------------------------------===
// debug.h - Debugging utilities (requires GLIBC/Unix-compliant libc implementation)
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#if WIN32
#include <windows.h>
void cc_print_stack_sw64(PCONTEXT ctx);
#endif
void cc_print_stack(int skip_frames);
void cc_except_init(void);
void cc_except_fini(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
