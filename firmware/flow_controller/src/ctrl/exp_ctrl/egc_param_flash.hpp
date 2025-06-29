#pragma once
#include "egc_types.hpp"

/*  Fill these fields with the JSON numbers printed by Calibrate.ino   */
/*  scale:  y = a·raw + b   (units: µL/min)                            */
/*  gain :  exponential-Ki parameters                                 */

static constexpr egc::EgcParams EGC_PARAMS = {
    /* scale (a, b)  */ { 0.993f, 2.1f },        //  ← scale_a, scale_b
    /* gain  (A,B,K,c,t_ref, α_static, A2,B2,K2,c2) */
    {
        .A = 0.00f,                              //  ← JSON "A"
        .B = 0.00025f,                           //  ← JSON "B"
        .K = 0.38f,                              //  ← JSON "K"
        .c = 0.0f,
        .t_ref = 57.4f,                          //  ← JSON "t_ref"
        .alpha_static = 0.20f,                   //  pick 0.10-0.25
        /* secondary α-dyn curve (reasonable defaults) */
        .A2 = 0.05f,
        .B2 = 0.00012f,
        .K2 = 0.95f,
        .c2 = 0.0f
    },
    /* Kp, Kd (optional) */
    0.0f, 0.0f
};
