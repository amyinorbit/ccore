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
#ifdef __cplusplus
extern "C" {
#endif

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
#define max(a, b) _Generic((a), uint32_t: max_u32, int32_t: max_i32, float: max_f32)(a, b)

/// Returns the minimum of [a] and [b].
#define min(a, b) _Generic((a), uint32_t: min_u32, int32_t: min_i32, float: min_f32)(a, b)

/// Clamps [v] to the range [hi, lo]
#define clamp(v, a, b) _Generic((v), uint32_t: clamp_u32, int32_t: clamp_i32, float: clamp_f32)(v, a, b)

static inline void vec3f_copy(const float v[3], float out[3]) {
    out[0] = v[0]; out[1] = v[1]; out[2] = v[2];
}

static inline void vec3d_copy(const double v[3], double out[3]) {
    out[0] = v[0]; out[1] = v[1]; out[2] = v[2];
}

static inline float vec3f_dot(const float a[3], const float b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline double vec3d_dot(const double a[3], const double b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline void vec3f_cross(const float a[3], const float b[3], float out[3]) {
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}

static inline void vec3d_cross(const double a[3], const double b[3], double out[3]) {
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}

static inline float vec3f_mag(const float a[3]) {
    return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

static inline float vec3d_mag(const double a[3]) {
    return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

static inline void vec3f_unit(const float v[3], float out[3]) {
    float m = vec3f_mag(v);
    if(m == 0 || fabsf(m) < 5 * FLT_EPSILON) return;
    out[0] = v[0]/m;
    out[1] = v[1]/m;
    out[2] = v[2]/m;
}

static inline void vec3f_unit_inplace(float v[3]) {
    float m = vec3f_mag(v);
    if(m == 0 || fabsf(m) < 5 * FLT_EPSILON) return;
    v[0] /= m;
    v[1] /= m;
    v[2] /= m;
}

static inline void vec3d_unit(const double v[3], double out[3]) {
    double m = vec3d_mag(v);
    if(m == 0 || fabs(m) < 5 * DBL_EPSILON) return;
    out[0] = v[0]/m;
    out[1] = v[1]/m;
    out[2] = v[2]/m;
}

static inline void vec3d_unit_inplace(double v[3]) {
    double m = vec3d_mag(v);
    if(m == 0 || fabs(m) < 5 * DBL_EPSILON) return;
    v[0] /= m;
    v[1] /= m;
    v[2] /= m;
}

static inline void vec3f_add(float a[3], const float b[3]) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}

static inline void vec3d_add(double a[3], const double b[3]) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}

static inline void vec3f_sub(float a[3], const float b[3]) {
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
}

static inline void vec3d_sub(double a[3], const double b[3]) {
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
}

static inline void vec3f_mul(float v[3], float m) {
    v[0] *= m;
    v[1] *= m;
    v[2] *= m;
}


static inline void vec3d_mul(double v[3], double m) {
    v[0] *= m;
    v[1] *= m;
    v[2] *= m;
}

static inline void vec3f_div(float v[3], float m) {
    v[0] /= m;
    v[1] /= m;
    v[2] /= m;
}


static inline void vec3d_div(double v[3], double m) {
    v[0] /= m;
    v[1] /= m;
    v[2] /= m;
}

static inline void vec3f_lerp(const float a[3], const float b[3], float t, float out[3]) {
    out[0] = lerp_f32(a[0], b[0], t);
    out[1] = lerp_f32(a[1], b[1], t);
    out[2] = lerp_f32(a[2], b[2], t);
}

static inline void vec3d_lerp(const double a[3], const double b[3], double t, double out[3]) {
    out[0] = lerp_f64(a[0], b[0], t);
    out[1] = lerp_f64(a[1], b[1], t);
    out[2] = lerp_f64(a[2], b[2], t);
}

static inline float vec3f_angle(const float a[3], const float b[3]) {
    return acos(vec3f_dot(a, b) / (vec3f_mag(a) * vec3f_mag(b)));
}

static inline double vec3d_angle(const double a[3], const double b[3]) {
    return acos(vec3d_dot(a, b) / (vec3d_mag(a) * vec3d_mag(b)));
}

#ifdef __cplusplus
} /* extern "C" */
#endif
