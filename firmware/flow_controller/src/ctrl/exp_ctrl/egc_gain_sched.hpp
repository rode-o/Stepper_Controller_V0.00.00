#pragma once
#include "egc_types.hpp"
#include "egc_filter.hpp"
#include <cmath>

namespace egc {

class GainScheduler {
public:
    explicit GainScheduler(const ExpParams& p) : P(p), lpf_static(p.alpha_static) {}

    /* main entry: give raw error, get Ki + e_dyn + α_dyn */
    void update(float err_raw,
                float& e_dyn_out,
                float& Ki_out,
                float& alpha_dyn_out)
    {
        /* 3  Static LPF */
        float e_static = lpf_static.update(err_raw);

        /* 4  Primary exponential gain */
        float Ki_primary = expFunc(e_static,
                                   P.A, P.B, P.K, P.c);

        /* 7  α_dyn from secondary exp */
        alpha_dyn = expFunc(e_static, P.A2, P.B2, P.K2, P.c2);
        alpha_dyn = constrain(alpha_dyn, 0.05f, 0.95f);

        /* 8  Dynamic LPF on error */
        lpf_dyn.setAlpha(alpha_dyn);
        float e_dyn = lpf_dyn.update(err_raw);

        /* 9  Final Ki using e_dyn */
        float Ki = expFunc(e_dyn, P.A, P.B, P.K, P.c);

        /* outputs */
        Ki_out         = Ki;
        e_dyn_out      = e_dyn;
        alpha_dyn_out  = alpha_dyn;
    }

    void reset() { lpf_static.reset(); lpf_dyn.reset(); }

private:
    static float expFunc(float t, float A,float B,float K,float c)
    {
        float denom = B*(t-c);
        if (fabsf(denom) < 1e-6f) denom = (denom>=0?1:-1)*1e-6f;
        return A + (K-A)*expf(-1.0f/denom);
    }
    const ExpParams& P;
    LPF lpf_static, lpf_dyn{0.5f};
    float alpha_dyn{0.5f};
};

} // namespace egc
