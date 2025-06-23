#pragma once
/* buttons_two.hpp – dual-button handler
 *
 *  • Short press             : inc / dec values
 *  • Dual-press 1-5 s + release  : cycle OLED pages
 *  • Dual-press ≥5 s              : toggle pump, RGB green/red
 *  • UP long-press ≥1 s           : toggle systemOn flag
 *
 *  Paths fixed for nested build:
 *     ../../../include/_include.hpp
 *     ../../_devices.hpp
 * ─────────────────────────────────────────────────────────── */
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
    enum class Mode : uint8_t { SETPOINT, MEASURE, ERROR };

    /* tweak to taste */
    static constexpr uint32_t PAGE_HOLD_MS = 500;   // 1 s
    static constexpr uint32_t PUMP_HOLD_MS = 5000;   // 5 s
    static constexpr uint32_t DEBOUNCE_MS  = 20;     // single-button inc/dec

    /* dual-press tracking */
    uint32_t mDualStart{0};
    bool     mDualActive{false};
    bool     mPumpLatched{false};

    /* other state */
    Mode     mMode{Mode::SETPOINT};
    int16_t  mNumberVal{0};
    int16_t  mLetterIdx{0};

    uint8_t  mLastMask{0};
    bool     mPageEdge{false};

    bool     mPumpEnabled{false};

    /* helpers */
    void announce(const char* tag, float v) const;
    void updateLED();
};

#endif // ENABLE_BUTTONS_TWO
