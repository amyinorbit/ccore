//===--------------------------------------------------------------------------------------------===
// filesystem.c - thin file system abstraction layer
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/filesystem.h>
#include <ccore/log.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h> // TODO: remove reliance on MingW, use windows.h API?

static size_t path_component_length(const char *comp) {
    size_t length = strlen(comp);
    while(length && comp[length-1] == CCFS_PATH_SEP) {
        length -= 1;
    }
    return length;
}

size_t ccfs_path_concat(char *out, size_t size, ...) {
    CCASSERT(out);
    size_t head = 0;

    va_list components;
    va_start(components, size);

    const char *comp = NULL;
    bool is_first = true;
    while((comp = va_arg(components, const char*))) {
        size_t comp_length = path_component_length(comp);
        if(head + comp_length + 1 >= size) break;

        if(is_first) {
            is_first = false;
        } else {
            out[head++] = CCFS_PATH_SEP;
        }
        strncpy(out + head, comp, comp_length);
        head += comp_length;
    }
    va_end(components);
    out[head] = '\0';
    return head;
}

bool ccfs_path_exists(const char *path) {
    return access(path, F_OK) != -1;
}

void ccfs_path_rtrim_i(char *path, int num_dirs) {
    int length = strlen(path);
    char *head = path + length-1;

    for(int dirs = 0; dirs < num_dirs+1 && head != path; --head) {
        char c = *head;
        if(c != '\\' && c != CCFS_PATH_SEP) continue;
        dirs += 1;
    }
    *(head+2) = '\0';
}
