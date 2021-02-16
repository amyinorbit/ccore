//===--------------------------------------------------------------------------------------------===
// time.c - Time utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#if WIN32
#include <ccore/time.h>
#include <windows.h>
#else /* !WIN32 */
#include <sys/time.h>
#endif /* !WIN32 */
#include <stdlib.h>

uint64_t cc_microtime(void)
{
#if WIN32
    LARGE_INTEGER val, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&val);
    return (((double)val.QuadPart / (double)freq.QuadPart) * 1000000.0);
#else    /* !IBM */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000000llu) + tv.tv_usec);
#endif    /* !IBM */
}
