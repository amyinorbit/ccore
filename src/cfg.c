//===--------------------------------------------------------------------------------------------===
// cfg - Really just a thin wrapper around table_t
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/cfg.h>
#include <ccore/table.h>
#include <ccore/memory.h>
#include <ccore/filesystem.h>
#include <ccore/log.h>
#include <ccore/string.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#define CC_CFG_DEFAULT_CAPACITY (32)

typedef struct {
    char *key;
    enum {CFG_NIL, CFG_STR, CFG_NUM, CFG_BOOL} kind;
    char *str;
    double f64;
    int i64;
    bool b;
} cc_cfg_entry_t;

typedef struct cc_cfg_s {
    size_t count;
    size_t capacity;
    cc_cfg_entry_t *entries;
} cc_cfg_t;


static cc_cfg_entry_t *find_entry(const cc_cfg_t *cfg, const char *key) {
    for(size_t i = 0; i < cfg->count; ++i) {
        if(strcmp(key, cfg->entries[i].key)) continue;
        return &cfg->entries[i];
    }
    CCWARN("No entry for key `%s`", key);
    return NULL;
}

static void ensure(cc_cfg_t *cfg) {
    if(cfg->count + 1 < cfg->capacity) return;
    cfg->capacity = cfg->capacity ? cfg->capacity * 2 : CC_CFG_DEFAULT_CAPACITY;
    cfg->entries = cc_realloc(cfg->entries, cfg->capacity * sizeof(cc_cfg_entry_t));
}

static cc_cfg_entry_t *find_entry_or_add(cc_cfg_t *cfg, const char *key) {
    cc_cfg_entry_t *entry = NULL;
    for(size_t i = 0; i < cfg->count; ++i) {
        if(strcmp(key, cfg->entries[i].key)) continue;
        entry = &cfg->entries[i];
        break;
    }
    if(entry) return entry;
    ensure(cfg);
    entry = &cfg->entries[cfg->count++];
    entry->key = string_duplicate(key);
    return entry;
}

static size_t read_line(FILE *f, char **line, size_t *capacity) {
    size_t size = 0;
    int c = 0;

    while((c = fgetc(f)) != EOF) {
        if(!size && isspace(c)) continue;
        if(c == '\r') continue;
        if(c == '\n') break;

        if(size + 2 > *capacity) {
            *capacity = *capacity ? *capacity * 2 : 128;
            *line = cc_realloc(*line, *capacity);
        }
        (*line)[size] = c;
        (*line)[size+1] = '\0';
        size += 1;
    }

    return size;
}

static inline bool line_is_comment(const char *line, size_t size) {
    CCASSERT(line);
    if(!size) return true;
    if(line[0] == '#') return true;
    if(size >= 2 && line[0] == '/' && line[1] == '/') return true;
    return false;
}

static char *trim(char *str) {
    while(*str && isspace(*str)) {
        str += 1;
    }
    return str;
}

static bool parse_number(char **value, cc_cfg_entry_t *entry) {
    char *src = *value;
    bool success = true;

    if(*src == '-' || *src == '+') src += 1;
    if(!isdigit(*src) && *src != '.') {
        CCERROR("invalid configuration: missing number for key `%s`", entry->key);
        success = false;
        goto done;
    }

    while(isdigit(*src)) src+= 1;
    if(*src == '.') {
        src += 1;
        while(isdigit(*src)) src+= 1;
    }

    entry->kind = CFG_NUM;
    entry->f64 = atof(*value);
    entry->i64 = entry->f64;
    entry->b = entry->i64 != 0;
    entry->str = NULL;

done:
    *value = src;
    return success;
}

static bool parse_string(char **value, cc_cfg_entry_t *entry, char delim) {
    char *src = *value + 1;
    const char *start = src;
    bool success = true;

    while(*src != delim) {
        if(!*src) {
            CCERROR("invalid configuration: missing closing quote for key `%s`", entry->key);
            success = false;
            goto done;
        }
        src += 1;
    }

    *src = '\0';
    src += 1;

    entry->kind = CFG_STR;
    entry->f64 = 0;
    entry->i64 = 0;
    entry->b = true;
    entry->str = string_duplicate(start);
done:
    *value = src;
    return success;
}

static bool parse_bool(char **value, cc_cfg_entry_t *entry) {
    char *src = *value;

    if(!strncmp(src, "true", 4)) {
        *value = src + 4;
        entry->kind = CFG_BOOL;
        entry->str = NULL;
        entry->i64 = 1;
        entry->f64 = 1;
        entry->b = true;
        return true;
    }

    if(!strncmp(src, "false", 5)) {
        *value = src + 5;
        entry->kind = CFG_BOOL;
        entry->str = NULL;
        entry->i64 = 0;
        entry->f64 = 0;
        entry->b = false;
        return true;
    }
    return false;
}

#ifdef CC_DEBUG_CFG_LOAD
static void debug_entry(const cc_cfg_entry_t *entry) {
    CCASSERT(entry);
    switch(entry->kind) {
    case CFG_NIL: CCDEBUG("(%s: <nil>)", entry->key); break;
    case CFG_STR: CCDEBUG("(%s: `%s`)", entry->key, entry->str); break;
    case CFG_BOOL: CCDEBUG("(%s: %s)", entry->key, entry->b ? "true" : "false"); break;
    case CFG_NUM: CCDEBUG("(%s: %f)", entry->key, entry->f64); break;
    }
}
#endif

static bool parse_value(char **value, cc_cfg_entry_t *entry) {
    switch(**value) {

    case '.': case '+': case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return parse_number(value, entry);

    case 't':
    case 'f':
        return parse_bool(value, entry);

    case '\'':
    case '\"':
        return parse_string(value, entry, **value);

    case '\0':
        CCERROR("invalid configuration: missing value for key `%s`", entry->key);
        return false;

    default:
        CCERROR(
            "invalid configuration: `%s` is not a valid value for key `%s`",
            *value,
            entry->key
        );
        return false;
    }
    CCUNREACHABLE();
}

static bool parse(cc_cfg_t *cfg, FILE *f) {
    CCASSERT(cfg);
    CCASSERT(f);

    bool success = true;
    char *line = NULL;
    size_t capacity = 0;

    while(!feof(f)) {
        size_t size = read_line(f, &line, &capacity);
        if(!size) continue;
        if(line_is_comment(line, size)) continue;

        char *sep = strchr(line, ':');
        if(!sep) {
            CCERROR("invalid configuration file: missing ':'");
            success = false;
            goto done;
        }

        *sep = '\0';
        const char *key = line;
        char *value = trim(sep+1);

        // CCDEBUG("key: %s, value: %s", key, value);
        cc_cfg_entry_t *entry = find_entry_or_add(cfg, key);
        CCASSERT(entry);
        if(!parse_value(&value, entry)) {
            success = false;
            goto done;
        }

        // Check that there isn't any text left after!
        value = trim(value);
        if(*value && !line_is_comment(value, strlen(value))) {
            CCERROR("invalid configuration file: additional text found for key `%s`", key);
            success = false;
            goto done;
        }
#ifdef CC_DEBUG_CFG_LOAD
        debug_entry(entry);
#endif
    }

done:
    if(line) cc_free(line);
    return success;
}

cc_cfg_t *cc_cfg_load(const char *path) {
    CCASSERT(path);
    FILE *file = ccfs_file_open(path, CCFS_READ);
    if(!file) return NULL;

    cc_cfg_t *cfg = cc_alloc(sizeof(cc_cfg_t));
    cfg->entries = NULL;
    cfg->capacity = 0;
    cfg->count = 0;

    parse(cfg, file);
    fclose(file);
    return cfg;
}
void cc_cfg_delete(cc_cfg_t *cfg) {
    CCASSERT(cfg);
    for(size_t i = 0; i < cfg->count; ++i) {
        cc_cfg_entry_t *entry = &cfg->entries[i];
        if(entry->kind == CFG_STR) cc_free(entry->str);
        cc_free(entry->key);
    }
    cc_free(cfg->entries);
    cc_free(cfg);
}

bool cc_cfg_key_exists(const cc_cfg_t *cfg, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    return find_entry(cfg, key) != NULL;
}

bool cc_cfg_get_bool(const cc_cfg_t *cfg, bool *out, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    CCASSERT(out);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    const cc_cfg_entry_t *entry = find_entry(cfg, key);
    if(!entry) return false;
    switch(entry->kind) {
    case CFG_NIL: *out = false; break;
    case CFG_NUM: *out = entry->f64 != 0; break;
    case CFG_BOOL: *out = entry->b; break;
    default: return false;
    }
    return true;
}

bool cc_cfg_get_int(const cc_cfg_t *cfg, int *out, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    CCASSERT(out);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    const cc_cfg_entry_t *entry = find_entry(cfg, key);
    if(!entry) return false;
    if(entry->kind != CFG_NUM) return false;
    *out = entry->i64;
    return true;
}

bool cc_cfg_get_float(const cc_cfg_t *cfg, float *out, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    CCASSERT(out);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    const cc_cfg_entry_t *entry = find_entry(cfg, key);
    if(!entry) return false;
    if(entry->kind != CFG_NUM) return false;
    *out = entry->f64;
    return true;

}

bool cc_cfg_get_double(const cc_cfg_t *cfg, double *out, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    CCASSERT(out);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    const cc_cfg_entry_t *entry = find_entry(cfg, key);
    if(!entry) return false;
    if(entry->kind != CFG_NUM) return false;
    *out = entry->f64;
    return true;
}

bool cc_cfg_get_str(const cc_cfg_t *cfg, const char **out, const char *fmt, ...) {
    CCASSERT(cfg);
    CCASSERT(fmt);
    CCASSERT(out);
    
    char key[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);
    
    const cc_cfg_entry_t *entry = find_entry(cfg, key);
    if(!entry) return false;
    if(entry->kind != CFG_STR) return false;
    *out = entry->str;
    return true;
}
