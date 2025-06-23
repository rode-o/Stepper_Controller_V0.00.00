#include "sh1107.hpp"

#ifdef ENABLE_SH1107

/* ───── helpers ───────────────────────────────────────── */
static inline float mL_to_uL(float mL) { return mL * 1000.0f; }

/* CP-437 symbols */
static inline void printPlusMinus(Adafruit_SH1107& d){ d.print((char)241);} // ±

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

void Sh1107Display::show(const volatile SystemState& s)
{
    switch (mPage) {
        case 0: drawSetFlowPage  (s); break;
        case 1: drawMeasuredPage (s); break;
        default:drawErrorCalPage (s); break;
    }
    mDisp.display();
}

/* ───── private draw helpers ─────────────────────────── */
void Sh1107Display::drawSetFlowPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top preview: measured flow */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Meas "));
    mDisp.print(mL_to_uL(s.flow), 0);
    mDisp.print(F(" uL/min"));

    /* centre: set-point (editable) */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f uL/min", s.setpoint);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2, (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom preview: error calibration */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Err "));
    printPlusMinus(mDisp);
    mDisp.print(s.errorPercent, 0);
    mDisp.print('%');
}

void Sh1107Display::drawMeasuredPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top preview: set-point */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    mDisp.print(s.setpoint, 0);
    mDisp.print(F(" uL/min"));

    /* centre: measured flow */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0f uL/min", mL_to_uL(s.flow));
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2, (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom preview: error calibration */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Err "));
    printPlusMinus(mDisp);
    mDisp.print(s.errorPercent, 0);
    mDisp.print('%');
}

void Sh1107Display::drawErrorCalPage(const volatile SystemState& s)
{
    mDisp.clearDisplay();
    mDisp.setFont();

    /* top preview: set-point */
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    mDisp.print(s.setpoint, 0);
    mDisp.print(F(" uL/min"));

    /* centre: error % */
    char buf[16];
    buf[0] = 241;                                 // ± symbol
    snprintf(buf + 1, sizeof(buf) - 1, "%.0f %%", s.errorPercent);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2, (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    /* bottom preview: measured flow */
    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Meas "));
    mDisp.print(mL_to_uL(s.flow), 0);
    mDisp.print(F(" uL/min"));
}

#endif // ENABLE_SH1107
