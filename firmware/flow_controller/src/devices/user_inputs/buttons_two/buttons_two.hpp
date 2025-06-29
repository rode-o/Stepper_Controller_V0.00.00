#pragma once
/*  buttons_two.hpp – dual-button handler
 *
 *  Short press  UP / DN               :  ±10 µL min⁻¹ (SETPOINT) | ±1 % (CAL-SCALAR)
 *  Dual-press 0.5–5 s (release)       :  page cycle  (SET → MEAS → CAL-SCALAR → CAL → …)
 *  Dual-press ≥5 s                    :  pump toggle (any page except CAL)  OR
 *                                        run calibration (CAL page)
 *  Long-press  UP ≥1 s                :  systemOn toggle
 */

#include <Arduino.h>
#include "../../../include/_include.hpp"
#include "../../_devices.hpp"

#ifdef ENABLE_BUTTONS_TWO

class ButtonsTwo {
public:
    bool begin();
    void poll();
    bool pageChanged() const { return mPageEdge; }

private:
    enum class Mode : uint8_t { SETPOINT, MEASURE, CALSCALAR, CALIB };

    /* timings (ms) */
    static constexpr uint32_t PAGE_HOLD_MS = 500;
    static constexpr uint32_t PUMP_HOLD_MS = 5000;
    static constexpr uint32_t DEBOUNCE_MS  = 20;

    /* dual-press bookkeeping */
    uint32_t mDualStart{0};
    bool     mDualActive{false};
    bool     mPumpLatched{false};

    /* editable state */
    Mode     mMode        {Mode::SETPOINT};
    int32_t  mNumberVal   {0};    // µL/min
    int16_t  mLetterIdx   {0};    // cal-scalar %
    uint8_t  mLastMask    {0};
    bool     mPageEdge    {false};
    bool     mPumpEnabled {false};

    void announce(const char* tag, float v) const;
    void updateLED();
};

#endif /* ENABLE_BUTTONS_TWO */
