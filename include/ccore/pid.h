//===--------------------------------------------------------------------------------------------===
// cfg - small configuration parser
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/log.h>
#include <ccore/cfg.h>
#include <ccore/math.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cc_pid_s {
    double prev;

    double error;
    double deriv;
    double integ;

    double kp;
    double ki;
    double kd;

    double integ_clamp;

    double rise_tau;
} cc_pid_t;

static inline void cc_pid_reset(cc_pid_t *pid);
static inline void cc_pid_update(cc_pid_t *pid, double setpoint, double value, double dt);
static inline double cc_pid_get_output(cc_pid_t *pid);

static inline bool cc_pid_load_cfg(cc_pid_t *pid, cc_cfg_t *cfg) {
    CCASSERT(pid);
    CCASSERT(cfg);
    if(!cc_cfg_get_double(cfg, "pid_kp", &pid->kp)) return false;
    if(!cc_cfg_get_double(cfg, "pid_ki", &pid->ki)) return false;
    if(!cc_cfg_get_double(cfg, "pid_kd", &pid->kd)) return false;
    if(!cc_cfg_get_double(cfg, "pid_rise", &pid->rise_tau)) return false;
    cc_pid_reset(pid);
    return true;
}

static inline void cc_pid_init(
    cc_pid_t *pid,
    double kp,
    double ki,
    double kd,
    double integ_clamp,
    double rise
) {
    CCASSERT(pid);
    cc_pid_reset(pid);
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integ_clamp = integ_clamp;
    pid->rise_tau = rise;
}

static inline void cc_pid_reset(cc_pid_t *pid) {
    CCASSERT(pid);
    pid->prev = NAN;
    pid->error = NAN;
    pid->deriv = NAN;
    pid->integ = NAN;
}


static inline void cc_pid_update(cc_pid_t *pid, double value, double error, double dt) {
    CCASSERT(pid);

    // If the PID was reset, start integrating at 0.
    if(isnan(pid->integ)) pid->integ = 0;
    pid->integ = cc_clamp(pid->integ + error * dt, -pid->integ_clamp, pid->integ_clamp);

    if(!isnan(pid->prev)) {
        double dv = (value - pid->prev) / dt;
        if(isnan(pid->deriv)) {
            pid->deriv = dv;
        } else {
            pid->deriv += (dv - pid->deriv) * (dt/pid->rise_tau);
        }
    }
    pid->error = error;
    pid->prev = value;
}

static inline double cc_pid_get_output(cc_pid_t *pid) {
    CCASSERT(pid);
    if(isnan(pid->prev) || isnan(pid->integ)) return NAN;
    return pid->kp * pid->error + pid->ki * pid->integ + pid->kd * pid->deriv;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
