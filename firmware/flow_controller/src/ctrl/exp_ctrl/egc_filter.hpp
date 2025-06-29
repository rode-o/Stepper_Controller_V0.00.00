#pragma once
#include <cmath>

namespace egc {

/*──── First‑order LPF with runtime α ────*/
class LPF {
public:
    explicit LPF(float alpha = 0.1f) : a(alpha) {}
    void  setAlpha(float alpha) { a = alpha; }
    float update(float x)       { state = a*x + (1-a)*state; return state; }
    void  reset(float x = 0)    { state = x; }
private:
    float a, state{};
};

} // namespace egc
