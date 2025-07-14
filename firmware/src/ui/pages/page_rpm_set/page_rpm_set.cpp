#include "page_rpm_set.hpp"
#include "../../common/common.hpp"

using namespace UI;

namespace {         // local constants
constexpr float STEP_FINE  = 0.1f;   // rpm / short-press
constexpr float STEP_COARSE= 1.0f;   // rpm / long-press
constexpr float MAX_RPM    = 30.0f;  // safety clamp (adjust)
}

void Pages::drawRpmSet(Adafruit_SH1107& d,
                       const volatile SystemState& s)
{
    d.clearDisplay();

    /* header -------------------------------------------------- */
    d.setFont(); d.setCursor(0, 0);
    d.print(F("RPM SET  "));
    d.print((s.ctrlMode == ControlMode::OPEN) ? F("O") : F("C"));

    /* large set-rpm ------------------------------------------ */
    char buf[18];
    snprintf(buf, sizeof buf, "%.1f rpm", s.setRpm);
    d.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    d.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    d.setCursor((d.width() - bw) / 2, 18);
    d.print(buf);
    d.setTextSize(1);

    /* actual rpm --------------------------------------------- */
    d.setCursor(0, 52);
    d.printf("Actual: %.1f", s.rpmCmd);

    drawCommonOverlay(d, s);
}

bool Pages::handleRpmButton(UI::Btn b)
{
    auto& st = State::write();       // mutable
    bool used = false;

    switch (b) {
        case UI::Btn::UP:
            st.setRpm = std::min(st.setRpm + STEP_FINE,  MAX_RPM); used = true; break;
        case UI::Btn::DOWN:
            st.setRpm = std::max(st.setRpm - STEP_FINE,  0.f);     used = true; break;
        case UI::Btn::OK_LONG:                         // coarse step
            st.setRpm = std::min(st.setRpm + STEP_COARSE, MAX_RPM); used = true; break;
        case UI::Btn::OK:                              // toggle to OPEN
            if (st.ctrlMode != ControlMode::OPEN) {
                st.ctrlMode = ControlMode::OPEN;
                used = true;
            }
            break;
        default: break;
    }
    return used;          // tell caller whether we consumed the event
}
