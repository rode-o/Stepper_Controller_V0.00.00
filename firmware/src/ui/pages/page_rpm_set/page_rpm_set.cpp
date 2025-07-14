#include "page_rpm_set.hpp"
#include "../../common/common.hpp"
#include <algorithm>      // std::min / std::max

using namespace UI;

/* ── local constants ──────────────────────────────────────── */
namespace {
constexpr float STEP    = 0.1f;   // rpm per tap
constexpr float MAX_RPM = 30.0f;  // safety clamp
}

/* ── draw page ─────────────────────────────────────────────── */
void Pages::drawRpmSet(Adafruit_SH1107& d,
                       const volatile SystemState& s)
{
    d.clearDisplay();

    /* header -------------------------------------------------- */
    d.setFont();
    d.setCursor(0, 0);
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

/* ── ▲ / ▼ handler (±0.1 rpm per tap) ─────────────────────── */
bool Pages::handleRpmButton(UI::Btn b)
{
    float rpm = State::read().setRpm;
    bool  used = false;

    if (b == UI::Btn::UP)   { rpm = std::min(rpm + STEP, MAX_RPM); used = true; }
    if (b == UI::Btn::DOWN) { rpm = std::max(rpm - STEP, 0.0f);    used = true; }

    if (used) {
        State::setRpm(rpm);                       // commit new target
        if (State::read().ctrlMode != ControlMode::OPEN)
            State::setCtrlMode(ControlMode::OPEN);  // force open-loop
    }
    return used;
}
