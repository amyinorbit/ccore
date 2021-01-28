//===--------------------------------------------------------------------------------------------===
// math.h - math utilities somehow not part of the standard library
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <math.h>
#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define CC_ROUNDING_ERROR (1e-10)

static inline uint32_t max_u32(uint32_t a, uint32_t b) { return a > b ? a : b; }
static inline uint32_t min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static inline int32_t max_i32(int32_t a, int32_t b) { return a > b ? a : b; }
static inline int32_t min_i32(int32_t a, int32_t b) { return a < b ? a : b; }
static inline float max_f32(float a, float b) { return a > b ? a : b; }
static inline float min_f32(float a, float b) { return a < b ? a : b; }

static inline uint32_t clamp_u32(uint32_t v, uint32_t min, uint32_t max) {
    return v > min ? (v < max ? v : max) : min;
}
static inline int32_t clamp_i32(int32_t v, int32_t min, int32_t max) {
    return v > min ? (v < max ? v : max) : min;
}
static inline float clamp_f32(float v, float min, float max) {
    return v > min ? (v < max ? v : max) : min;
}

static inline uint32_t lerp_u32(uint32_t a, uint32_t b, float t) { return a*(1.f-t) + b*t; }
static inline int32_t lerp_i32(int32_t a, int32_t b, float t) { return a*(1.f-t) + b*t; }
static inline float lerp_f32(float a, float b, float t) { return a*(1.f-t) + b*t; }
static inline double lerp_f64(double a, double b, double t) { return a*(1.0-t) + b*t; }

/// Returns the maximum of [a] and [b].
#define cc_max(a, b) _Generic((a), uint32_t: max_u32, int32_t: max_i32, float: max_f32)(a, b)

/// Returns the minimum of [a] and [b].
#define cc_min(a, b) _Generic((a), uint32_t: min_u32, int32_t: min_i32, float: min_f32)(a, b)

/// Clamps [v] to the range [hi, lo]
#define cc_clamp(v, a, b) _Generic((v), uint32_t: clamp_u32, int32_t: clamp_i32, float: clamp_f32)(v, a, b)

typedef union {
    struct { double x, y; };
    double data[2];
} vec2_t;

typedef union {
    vec2_t xy;
    struct { double x, y, z; };
    double data[3];
} vec3_t;

#define CC_VEC2(xx, yy) ((vec2_t){.x = (xx), .y = (yy)})

#define CC_VEC3_2(xy) ((vec3_t){.x = (xy).x, .y = (xy).y, .z = 0})
#define CC_VEC3(xx, yy, zz) ((vec3_t){.x = (xx), .y = (yy), .z = (zz)})

#define CC_VEC2_NULL CC_VEC2(NAN, NAN)
#define CC_VEC3_NULL CC_VEC3(NAN, NAN, NAN)

#define CC_VEC_IS_NULL(v) (isnan((v).x))

unsigned vec3_int_sph(vec3_t o, vec3_t v, vec3_t c, double r, bool confined, vec3_t out[2]);
vec2_t vec2_int_vec2(vec2_t o1, vec2_t v1, vec2_t o2, vec2_t v2);

static inline double vec3_dot(vec3_t a, vec3_t b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return CC_VEC3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

static inline double vec3_sqmag(vec3_t a) {
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

static inline double vec3_mag(vec3_t a) {
    return sqrt(vec3_sqmag(a));
}

static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    return CC_VEC3(a.x+b.x, a.y+b.y, a.z+b.z);
}

static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return CC_VEC3(a.x-b.x, a.y-b.y, a.z-b.z);
}

static inline vec3_t vec3_mul(vec3_t a, double b) {
    return CC_VEC3(a.x*b, a.y*b, a.z*b);
}

static inline vec3_t vec3_div(vec3_t a, double b) {
    return CC_VEC3(a.x/b, a.y/b, a.z/b);
}

static inline vec3_t vec3_unit(vec3_t v) {
    double m = vec3_mag(v);
    if(m == 0 || fabs(m) < 5 * DBL_EPSILON) return v;
    return vec3_div(v, m);
}

static inline vec3_t vec3_lerp(vec3_t a, vec3_t b, double t) {
    return CC_VEC3(
        lerp_f64(a.x, b.x, t),
        lerp_f64(a.y, b.y, t),
        lerp_f64(a.z, b.z, t)
    );
}

static inline double vec3_angle(vec3_t a, vec3_t b) {
    return acos(vec3_dot(a, b) / (vec3_mag(a) * vec3_mag(b)));
}

static inline vec2_t vec2_hdg_mag(double hdg, double mag) {
    return CC_VEC2(mag * sin(hdg), mag * cos(hdg));
}

static inline double vec2_dot(vec2_t a, vec2_t b) {
    return a.x*b.x + a.y*b.y;
}

static inline double vec2_pdot(vec2_t a, vec2_t b) {
    return a.x*b.y - a.y*b.x;
}

static inline vec2_t vec2_neg(vec2_t v) {
    return CC_VEC2(-v.x, -v.y);
}

static inline double vec2_sqmag(vec2_t a) {
    return a.x*a.x + a.y*a.y;
}

static inline double vec2_mag(vec2_t a) {
    return sqrt(vec2_sqmag(a));
}

static inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    return CC_VEC2(a.x+b.x, a.y+b.y);
}

static inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return CC_VEC2(a.x-b.x, a.y-b.y);
}

static inline vec2_t vec2_mul(vec2_t a, double b) {
    return CC_VEC2(a.x*b, a.y*b);
}

static inline vec2_t vec2_div(vec2_t a, double b) {
    return CC_VEC2(a.x/b, a.y/b);
}

static inline vec2_t vec2_unit(vec2_t v) {
    double m = vec2_mag(v);
    if(m == 0 || fabs(m) < 5 * DBL_EPSILON) return v;
    return vec2_div(v, m);
}

static inline vec2_t vec2_lerp(vec2_t a, vec2_t b, double t) {
    return CC_VEC2(
        lerp_f64(a.x, b.x, t),
        lerp_f64(a.y, b.y, t)
    );
}

static inline double vec2_angle(vec2_t a, vec2_t b) {
    return acos(vec2_dot(a, b) / (vec2_mag(a) * vec2_mag(b)));
}


#ifdef __cplusplus
} /* extern "C" */
#endif
