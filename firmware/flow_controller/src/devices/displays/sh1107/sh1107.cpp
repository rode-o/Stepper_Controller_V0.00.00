/*─────────────────────────────────────────────────────────────*/
/*  sh1107.cpp – OLED page driver                              */
/*─────────────────────────────────────────────────────────────*/

#include "sh1107.hpp"
#include "../../../main.hpp"          // SystemState, etc.

#ifdef ENABLE_SH1107

/* ───── public interface ─────────────────────────────── */
bool Sh1107Display::begin()
{
    Wire.setClock(400'000);
    if (!mDisp.begin(I2C_ADDR, /*reset=*/true)) return false;

    mDisp.setRotation(1);
    mDisp.setTextColor(SH110X_WHITE);
    mDisp.setTextWrap(false);
    mDisp.clearDisplay();
    mDisp.display();
    return true;
}

void Sh1107Display::advancePage() { mPage = (mPage + 1) % PAGES; }

/* ───── dispatcher ───────────────────────────────────── */
void Sh1107Display::show(const volatile SystemState& s)
{
    switch (mPage) {
        case 0: drawModePage     (s); break;
        case 1: drawSetPointPage (s); break;
        case 2: drawMeasuredPage (s); break;
        default:drawCalScalarPage(s); break;   // page 3
    }
    mDisp.display();
}

/* ───── page 0 : CTRL MODE ───────────────────────────── */
void Sh1107Display::drawModePage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    mDisp.setCursor(0, 0);
    mDisp.print(F("CTRL MODE"));

    const char* modeStr =
        (s.ctrlMode == ControlMode::CLOSED) ? "CLOSED" : "OPEN";

    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(modeStr, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width()  - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(modeStr);
    mDisp.setTextSize(1);

    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Tap \x18/\x19 to toggle"));   // up/down arrows
}

/* ───── page 1 : SET-POINT / RPM ─────────────────────── */
void Sh1107Display::drawSetPointPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top-line : saved set-point */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    if (s.ctrlMode == ControlMode::CLOSED) {
        mDisp.print(s.setFlow_uLmin, 0);
        mDisp.print(F(" uL/min"));
    } else {
        mDisp.print(s.setRpm, 0);
        mDisp.print(F(" rpm"));
    }

    /* centre : editable set-point (same number, large) */
    char buf[24];
    if (s.ctrlMode == ControlMode::CLOSED)
        snprintf(buf, sizeof(buf), "%.0f uL/min", s.setFlow_uLmin);
    else
        snprintf(buf, sizeof(buf), "%.0f rpm", s.setRpm);

    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width()  - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom : ±Cal scalar */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal "));
    printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0);
    mDisp.print('%');
}

/* ───── page 2 : MEASURED (raw flow) ─────────────────── */
void Sh1107Display::drawMeasuredPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top : target */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    if (s.ctrlMode == ControlMode::CLOSED) {
        mDisp.print(s.setFlow_uLmin, 0);
        mDisp.print(F(" uL/min"));
    } else {
        mDisp.print(s.setRpm, 0);
        mDisp.print(F(" rpm"));
    }

    /* centre : raw measured flow */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f uL/min", s.r_flow);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width()  - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom : ±Cal scalar */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal "));
    printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0);
    mDisp.print('%');
}

/* ───── page 3 : ±Cal scalar ─────────────────────────── */
void Sh1107Display::drawCalScalarPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top : target */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    if (s.ctrlMode == ControlMode::CLOSED) {
        mDisp.print(s.setFlow_uLmin, 0);
        mDisp.print(F(" uL/min"));
    } else {
        mDisp.print(s.setRpm, 0);
        mDisp.print(F(" rpm"));
    }

    /* centre : ±Cal % */
    char buf[16];
    buf[0] = 241;   // ±
    snprintf(buf + 1, sizeof(buf) - 1, "%.0f %%", s.calScalar);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width()  - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom : raw flow preview */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Meas "));
    mDisp.print(s.r_flow, 0);
    mDisp.print(F(" uL/min"));
}

#endif /* ENABLE_SH1107 */
