//===--------------------------------------------------------------------------------------------===
// math.c - maths/geometry utilities
//
// Created by Amy Parent <developer@amyparent.com>
// Copyright (c) 2020 Amy Parent
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/math.h>

static inline double pow2(double a) { return a*a; }
static inline bool vec2_parallel(vec2_t a, vec2_t b) {
    return (a.y == 0 && b.y == 0) || (a.x/a.y == b.x/b.y);
}

static inline bool vec2_equal(vec2_t a, vec2_t b) {
    return a.x == b.x && a.y == b.y;
}

unsigned vec3_int_sph(vec3_t o, vec3_t v, vec3_t c, double r, bool confined, vec3_t out[2]) {
    // First we compute Delta, to find out whether there are any solutions
    vec3_t u = vec3_unit(v);
    double dist = vec3_mag(v);
    vec3_t o_to_c = vec3_sub(o, c);
    double u_o_c = vec3_dot(u, o_to_c);
    double delta = pow2(u_o_c) - vec3_sqmag(o_to_c) + pow2(r);
    
    if(out) {
        out[0] = CC_VEC3_NULL;
        out[1] = CC_VEC3_NULL;
    }

    if(delta < 0) return 0;

    double sq_delta = sqrt(delta);
    
    if(delta == 0) {
        // A single root (intersection) exists:
        double d = - u_o_c;
        if((d >= 0 && d <= dist) || !confined) {
            if(out) out[0] = vec3_add(o, vec3_mul(u, d));
            return 1;
        }
        return 0;
    }

    static const double mult[2] = {-1, 1};
    unsigned valid = 0;
    for(int i = 0; i < 2; ++i) {
        double d = -u_o_c + mult[i] * sq_delta;
        
        if((d >= 0 && d <= dist) || !confined) {
            if(out) out[valid] = vec3_add(o, vec3_mul(u, d));
            valid += 1;
        }
    }
    return valid;
}

vec2_t vec2_int_vec2(vec2_t a, vec2_t u, vec2_t b, vec2_t v) {
    if(vec2_parallel(u, v)) return CC_VEC2_NULL;
    if(vec2_equal(a, b)) return a;

    vec2_t w = vec2_sub(b, a);
    vec2_t mw = vec2_sub(a, b);

    double ta = vec2_pdot(w, v)/vec2_pdot(u, v);
    double tb = vec2_pdot(mw, u)/vec2_pdot(v, u);
    // TODO: replace with more lenient intercept logic
    if(ta < 0 || ta > 1.0 || tb < 0 || tb > 1.0) return CC_VEC2_NULL;

    vec2_t i1 = vec2_add(a, vec2_mul(u, ta));
    //vec2_t i2 = vec2_add(b, vec2_mul(v, tb));
    return i1;
}
