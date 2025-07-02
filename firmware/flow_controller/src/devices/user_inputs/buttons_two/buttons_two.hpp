/*─────────────────────────────────────────────────────────────*/
/*  buttons_two.hpp – dual-button handler (Up / Down)          */
/*─────────────────────────────────────────────────────────────*/

#pragma once
#include <Arduino.h>
#include "../../../include/_include.hpp"     // LED helpers, pin defs
#include "../../_devices.hpp"
#include "../../../ctrl/_ctrl.hpp"           // MainCtrl::onModeChanged

#ifdef ENABLE_BUTTONS_TWO

class ButtonsTwo {
public:
    bool  begin();
    void  poll();

    /* OLED sync helpers */
    bool        pageChanged() const { return mPageEdge; }
    uint8_t     currentPage() const { return static_cast<uint8_t>(mPage); }

private:
    enum class Page : uint8_t { MODE, SETPOINT, MEASURE, CALSCALAR };

    /* timings (ms) */
    static constexpr uint32_t PAGE_HOLD_MS = 500;
    static constexpr uint32_t PUMP_HOLD_MS = 5000;
    static constexpr uint32_t DEBOUNCE_MS  = 20;

    /* dual-press bookkeeping */
    uint32_t mDualStart{0};
    bool     mDualActive{false};
    bool     mPumpLatched{false};

    /* editable state */
    Page     mPage        {Page::MODE};
    int32_t  mFlowVal     {0};      // µL / min  (step 25)
    int16_t  mRpmVal      {0};      // rpm       (step 1)
    int16_t  mCalIdx      {0};      // ±%
    uint8_t  mLastMask    {0};
    bool     mPageEdge    {false};
    bool     mPumpEnabled {false};

    /* helpers */
    void updateLED();
    void announce(const char* tag, float v) const;
};

#endif /* ENABLE_BUTTONS_TWO */
