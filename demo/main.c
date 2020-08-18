//===--------------------------------------------------------------------------------------------===
// demo - demo of ccore stuff
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/tpool.h>
#include <ccore/log.h>
#include <unistd.h>

void worker_1(void *data) {
    long duration = (long)data;
    sleep(duration);
}

int main() {
    ccpool_start(4);

    for(int i = 0; i < 10; ++i) {
        ccpool_submit(worker_1, (void*)(long)i);
    }

    ccpool_wait();
    CCINFO("done first set of tasks");
    for(int i = 0; i < 10; ++i) {
        ccpool_submit(worker_1, NULL);
    }
    ccpool_wait();
    CCINFO("done second set of tasks");

    ccpool_stop();
}
