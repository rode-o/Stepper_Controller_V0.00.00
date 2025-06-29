#include "sh1107.hpp"
#include "../../../ctrl/exp_ctrl/egc_calibration_config.hpp"   // CAL_TOTAL_MS
#include "../../../min_main.hpp"     // SystemState, etc.

#ifdef ENABLE_SH1107

/* ───── helpers ───────────────────────────────────────── */

/* CP-437 ± symbol */
static inline void printPlusMinus(Adafruit_SH1107& d)
{ d.print((char)241); }

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
    if (s.calibrating) {          // modal progress bar overrides pages
        drawCalProgress();
        mDisp.display();
        return;
    }

    switch (mPage) {
        case 0: drawSetFlowPage   (s); break;
        case 1: drawMeasuredPage  (s); break;
        case 2: drawCalScalarPage (s); break;
        default: drawInitCalPage  (s); break;
    }
    mDisp.display();
}

/* ───── page helpers ─────────────────────────────────── */
void Sh1107Display::drawSetFlowPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top: raw flow preview */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Meas "));
    mDisp.print(s.r_flow, 0);
    mDisp.print(F(" uL/min"));

    /* centre: set-point */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f uL/min", s.setpoint);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom: cal-scalar */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal "));
    printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0);
    mDisp.print('%');
}

void Sh1107Display::drawMeasuredPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top: set-point */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    mDisp.print(s.setpoint, 0);
    mDisp.print(F(" uL/min"));

    /* centre: filtered flow */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f uL/min", s.f_flow);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom: cal-scalar */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal "));
    printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0);
    mDisp.print('%');
}

void Sh1107Display::drawCalScalarPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top: set-point */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    mDisp.print(s.setpoint, 0);
    mDisp.print(F(" uL/min"));

    /* centre: ±cal-scalar */
    char buf[16];
    buf[0] = 241;   // ±
    snprintf(buf + 1, sizeof(buf) - 1, "%.0f %%", s.calScalar);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom: raw flow preview */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Meas "));
    mDisp.print(s.r_flow, 0);
    mDisp.print(F(" uL/min"));
}

void Sh1107Display::drawInitCalPage(const volatile SystemState&)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    const char* msg = "Init Cal?";
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(msg, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(msg);
    mDisp.setTextSize(1);

    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Hold 5s to run"));
}

/* ───── calibration progress bar ─────────────────────── */
void Sh1107Display::drawCalProgress()
{
    extern volatile uint32_t gCalibStart;

    mDisp.clearDisplay();
    mDisp.setFont();

    mDisp.setCursor(0, 0);
    mDisp.print(F("Calibrating…"));

    uint32_t elapsed = millis() - gCalibStart;
    if (elapsed > CAL_TOTAL_MS) elapsed = CAL_TOTAL_MS;
    float pct = elapsed * 100.0f / CAL_TOTAL_MS;

    constexpr int BAR_W = 100, BAR_H = 6;
    int filled = static_cast<int>(BAR_W * pct / 100.0f);
    int x = (mDisp.width()  - BAR_W) / 2;
    int y = (mDisp.height() - BAR_H) / 2;

    mDisp.drawRect(x, y, BAR_W, BAR_H, SH110X_WHITE);
    if (filled > 2)
        mDisp.fillRect(x + 1, y + 1, filled - 2, BAR_H - 2, SH110X_WHITE);

    char buf[8]; snprintf(buf, sizeof(buf), "%3.0f%%", pct);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2, y + BAR_H + 4);
    mDisp.print(buf);
}

#endif // ENABLE_SH1107