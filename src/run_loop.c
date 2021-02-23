//===--------------------------------------------------------------------------------------------===
// run_loop.c - Threaded run-loop system
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/run_loop.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <ccore/time.h>
#include <ccore/string.h>
#include <pthread.h>

#define USEC (1)
#define MSEC (1000*USEC)
#define SEC (1000*MSEC)

#define MAX_RLOOP_WAIT (1 * SEC)

typedef struct rl_entry_t {
    void (*main)(double, void *);
    void *data;
    
    uint64_t acc;
    int64_t interval;
    struct rl_entry_t *next, *prev;
} rl_entry_t;

struct cc_run_loop_t {
    bool is_running;
    bool stop;
    char name[20];
    
    rl_entry_t programs;

    pthread_t thread;
    pthread_mutex_t loops_mt;
    pthread_mutex_t mt;
    pthread_cond_t cv;
};

static void cond_wait_until(pthread_cond_t *cv, pthread_mutex_t *mt, uint64_t t) {
    struct timespec abs;
    abs.tv_sec = t / 1000000UL;
    abs.tv_nsec = (t % 1000000UL) * 1000UL;
    pthread_cond_timedwait(cv, mt, &abs);
}

static void *loop_thread(void *data) {
    cc_run_loop_t *rl = data;
    
    uint64_t t = cc_microtime();
    uint64_t interval = 0;
    pthread_mutex_lock(&rl->mt);
    
#ifdef __APPLE__
    pthread_setname_np(rl->name);
#else
    pthread_setname_np(pthread_self(), rl->name);
#endif
    
    
    rl->is_running = true;
    
    while(!rl->stop) {
        cond_wait_until(&rl->cv, &rl->mt, t + interval);
        if(rl->stop) break;
        uint64_t new_t = cc_microtime();
        uint64_t delta = new_t - t;
        t = new_t;
        pthread_mutex_unlock(&rl->mt);
    
        interval = UINT64_MAX;
        
        pthread_mutex_lock(&rl->loops_mt);
        
        for(rl_entry_t *prog = rl->programs.next; prog != &rl->programs; prog = prog->next) {
            prog->acc -= delta;
            if(prog->acc <= 0) {
                prog->main((double)delta/1e6, prog->data);
                prog->acc += prog->interval;
            }
        
            if(prog->acc < interval) interval = prog->acc;
        }
        pthread_mutex_unlock(&rl->loops_mt);
        
        interval = interval < MAX_RLOOP_WAIT ? interval : MAX_RLOOP_WAIT;
        
        pthread_mutex_lock(&rl->mt);
        pthread_cond_broadcast(&rl->cv);
    }
    
    rl->is_running = false;
    pthread_mutex_unlock(&rl->mt);
    
    return NULL;
}

cc_run_loop_t *cc_run_loop_new(const char *name) {
    CCASSERT(name);
    
    cc_run_loop_t *loop = cc_alloc(sizeof(cc_run_loop_t));
    
    pthread_cond_init(&loop->cv, NULL);
    pthread_mutex_init(&loop->mt, NULL);
    pthread_mutex_init(&loop->loops_mt, NULL);
    
    string_copy(loop->name, name, sizeof(name));
    loop->stop = true;
    loop->is_running = false;
    
    loop->programs.next = &loop->programs;
    loop->programs.prev = &loop->programs;

    pthread_mutex_lock(&loop->mt);
    pthread_create(&loop->thread, NULL, &loop_thread, loop);
    loop->stop = false;
    pthread_mutex_unlock(&loop->mt);
    CCDEBUG("run loop `%s` started", loop->name);
    return loop;
}


void cc_run_loop_delete(cc_run_loop_t *rl) {
    CCASSERT(rl);
    CCASSERT(!rl->stop);
    pthread_mutex_lock(&rl->mt);
    rl->stop = true;
    pthread_mutex_unlock(&rl->mt);
    pthread_join(rl->thread, NULL);
    
    rl_entry_t *rl_entry = rl->programs.next;
    while(rl_entry != &rl->programs) {
        rl_entry_t *to_del = rl_entry;
        rl_entry = rl_entry->next;
        cc_free(to_del);
    }
    
    pthread_cond_destroy(&rl->cv);
    pthread_mutex_destroy(&rl->mt);
    pthread_mutex_destroy(&rl->loops_mt);
    CCDEBUG("run loop `%s` stopped", rl->name);
    cc_free(rl);
    
}

cc_run_loop_handle_t cc_run_loop_register(
    cc_run_loop_t *rl,
    void (*ticker)(double, void *),
    double freq,
    void *data
) {
    CCASSERT(rl);
    CCASSERT(ticker);
    CCASSERT(freq > 0);
    
    rl_entry_t *entry = cc_alloc(sizeof(rl_entry_t));
    entry->main = ticker;
    entry->interval = 1e6 / freq;
    entry->acc = rand() % entry->interval;
    entry->data = data;
    
    pthread_mutex_lock(&rl->loops_mt);
    entry->prev = rl->programs.prev; // Tail of the DL list
    entry->next = &rl->programs;

    entry->next->prev = entry;
    entry->prev->next = entry;
    pthread_mutex_unlock(&rl->loops_mt);
    
    CCDEBUG("exec %p added to run_loop `%s`", entry, rl->name);
    return entry;
}

void cc_run_loop_unregister(cc_run_loop_t *rl, cc_run_loop_handle_t handle) {
    CCASSERT(rl);
    CCASSERT(handle);
    
    rl_entry_t *entry = handle;
    CCASSERT(entry->next || entry->prev);
    
    pthread_mutex_lock(&rl->loops_mt);
    
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = entry->next = NULL;
    pthread_mutex_unlock(&rl->loops_mt);
    CCDEBUG("exec %p removed to run_loop `%s`", entry, rl->name);
    
    cc_free(entry);
}
