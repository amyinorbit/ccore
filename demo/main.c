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
#include <ccore/math.h>
#include <unistd.h>
#include <stdio.h>

void worker_1(void *data) {
    CCUNUSED(data);
    sleep(1);
}

#define DBG_VEC2(v) printf(#v " = {%f. %f}\n", (v).x, (v).y)

int main() {
    ccpool_start(4);


    vec2_t a = CC_VEC2(0, 0);
    vec2_t b = CC_VEC2(10, 0);

    vec2_t u = CC_VEC2(10, 10);
    vec2_t v = CC_VEC2(10, -10);

    vec2_t i = vec2_int_vec2(a, u, b, v);
    DBG_VEC2(i);



    ccpool_stop();
}
