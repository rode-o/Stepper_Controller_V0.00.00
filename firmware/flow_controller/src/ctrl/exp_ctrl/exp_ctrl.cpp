/*
 * File: exp_ctrl.cpp
 * Brief: PID controller whose gains follow
 *        f(x) = A + (K − A) · exp( −1 / (B · (x − c)) ).
 *
 * Error conditioning (2-pole cascade, hidden in TwoPoleFilter):
 *   raw → adaptive slope-matched LPF → fixed-α EMA → PID
 */

#include "exp_ctrl.hpp"
#include "../../include/_include.hpp"        // EXP_KP_*, EXP_KI_*, EXP_KD_*
#include "../../core/_core.hpp"
#include "../../devices/_devices.hpp"  // initPump/stopPump/runSequence
#include <Arduino.h>

#ifdef ENABLE_EXP_CTRL

/* ───────── external integrator symbols (pid.cpp) ───────── */
extern float integralTerm;
extern float g_lastIntegralIncrement;

/* ───────── local persistent state ───────── */
static TwoPoleFilter s_errFilter;
static float         s_lastKi     = 0.0f;
static float         g_errSmooth  = 0.0f;

/* ───────── forward-declared gain helpers ───────── */
static float getExpKp(float x);
static float getExpKi(float x);
static float getExpKd(float x);

/* ───────────────────────────────────────────────────────── */
void initExpController(SystemState& state)
{
    state = {};
    initPID();
    initTwoPoleFilter(s_errFilter);
    s_lastKi     = 0.0f;
    g_errSmooth  = 0.0f;

    Serial.println(F("[EXP_CONTROL] reset OK"));
}

/* ───────────────────────────────────────────────────────── */
void updateExpController(
        SystemState& state,
        float        flow,
        float        flowSetpoint,
        float        errorPercent,
        bool         systemOn,
        float&       desiredVoltage,
        float&       pidFraction,
        bool&        bubbleDetected,
        float&       pTermOut,
        float&       iTermOut,
        float&       dTermOut)
{
    /* 0. Safety: system OFF */
    if (!systemOn) {
        stopPump();
        pidFraction = desiredVoltage = pTermOut = iTermOut = dTermOut = 0.0f;
        return;
    }

    /* 1. Raw error */
    float errRaw = flowSetpoint - flow;

    /* 2. Two-pole filtering */
    float errSmooth = updateTwoPoleFilter(s_errFilter, errRaw);

    /* 3. Exponential gains */
    float absE = fabsf(errSmooth);
    float kp = getExpKp(absE);
    float ki = getExpKi(absE);
    float kd = getExpKd(absE);

    /* rescale integrator when Ki jumps */
    if (fabsf(s_lastKi - ki) > 1e-9f) {
        if (fabsf(s_lastKi) > 1e-9f && fabsf(ki) > 1e-9f)
            integralTerm *= s_lastKi / ki;
        s_lastKi = ki;
    }

    setPIDGains(kp, ki, kd);
    state.pGain = kp;  state.iGain = ki;  state.dGain = kd;

    /* 4. PID step */
    pidFraction = updatePIDNormal(errSmooth, pTermOut, iTermOut, dTermOut);

    /* clamp & anti-windup */
    if (pidFraction > 1.0f) {
        integralTerm -= g_lastIntegralIncrement;
        pidFraction   = 1.0f;
    } else if (pidFraction < 0.0f) {
        pidFraction = 0.0f;
    }

    /* 5. Voltage map within driver limits */
    desiredVoltage = pidFraction * DRV8825_V_MAX;
    if (desiredVoltage > 0.0f && desiredVoltage < DRV8825_V_MIN)
        desiredVoltage = DRV8825_V_MIN;
    if (desiredVoltage > DRV8825_V_MAX)
        desiredVoltage = DRV8825_V_MAX;

    /* 6. Drive pump */
    runSequence(desiredVoltage);

    /* 7. Publish extra state */
    state.filteredError = errSmooth;
    state.currentAlpha  = s_errFilter.dyn.currentAlpha;
    state.pidOutput     = pidFraction;
    state.desiredVoltage= desiredVoltage;
}

/* ───────── exponential gain helpers ───────── */
static float expCurve(float x, float A, float K, float B, float C)
{
    float denom = B * (x - C);
    if (fabsf(denom) < 1e-9f) return A;
    float v = A + (K - A) * expf(-1.0f / denom);
    v = constrain(v, A, K);
    return v;
}

static float getExpKp(float x){ return expCurve(x, EXP_KP_A, EXP_KP_K, EXP_KP_B, EXP_KP_C); }
static float getExpKi(float x){ return expCurve(x, EXP_KI_A, EXP_KI_K, EXP_KI_B, EXP_KI_C); }
static float getExpKd(float x){ return expCurve(x, EXP_KD_A, EXP_KD_K, EXP_KD_B, EXP_KD_C); }


#endif