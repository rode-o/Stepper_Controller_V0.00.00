/*─────────────────────────────────────────────────────────────*/
/*  sh1107.cpp – page render + sliding animation               */
/*─────────────────────────────────────────────────────────────*/
#include "sh1107.hpp"
#include "../../../main.hpp"

#ifdef ENABLE_SH1107

/* -------- init ------------------------------------------------------ */
bool Sh1107Display::begin()
{
    Wire.setClock(400'000);
    if (!mDisp.begin(I2C_ADDR, /*reset=*/true)) return false;

    mDisp.setRotation(1);                 // keep your preferred orientation
    mDisp.setTextColor(SH110X_WHITE);
    mDisp.setTextWrap(false);
    mDisp.clearDisplay();
    mDisp.display();
    return true;
}

void Sh1107Display::advancePage() { mPage = (mPage + 1) % PAGES; }

/* -------- tiny glyphs ---------------------------------------------- */
void Sh1107Display::drawModeIcon(const volatile SystemState& s)
{
    mDisp.setTextSize(2);
    mDisp.setCursor(mDisp.width() - 12, 0);
    mDisp.print((s.ctrlMode == ControlMode::CLOSED) ? 'C' : 'O');
    mDisp.setTextSize(1);
}

void Sh1107Display::drawPumpIcon(const volatile SystemState& s)
{
    /* icon position */
    const int16_t xIcon = mDisp.width()  - 16;   // 12 px glyph + 2 px margin
    const int16_t yIcon = mDisp.height() - 16;

    /* cumulative volume string (e.g. “37550uL”) */
    char buf[16];
    snprintf(buf, sizeof buf, "%luuL", (unsigned long)s.volume_uL);

    mDisp.setTextSize(1);                        // 6 × 8 px font
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);

    /* print text flush-right, 2 px left of the icon */
    mDisp.setCursor(xIcon - 2 - bw, yIcon + 2);
    mDisp.print(buf);

    /* pump-state glyph */
    if (State::isPumpEnabled())
        mDisp.fillRect(xIcon, yIcon, 12, 12, SH110X_WHITE);   // ■
    else
        mDisp.drawRect(xIcon, yIcon, 12, 12, SH110X_WHITE);   // ▢
}

/* -------- dispatcher (no animation) ------------------------------- */
void Sh1107Display::show(const volatile SystemState& s) 
{
    renderCurrentPage(s);
    mDisp.display();
}

/* -------- render helper ------------------------------------------- */
void Sh1107Display::renderCurrentPage(const volatile SystemState& s)
{
    switch (mPage) {
        case 0: drawSetPointPage (s); break;
        case 1: drawMeasuredPage (s); break;
        default:drawCalScalarPage(s); break;
    }
    drawPumpIcon(s);
}

/* -------- animated page change ------------------------------------ */
void Sh1107Display::animatedTo(uint8_t dest,
                               const volatile SystemState& s)
{
    dest %= PAGES;
    if (dest == mPage) { show(s); return; }

    /* render current and destination frames into temp buffers */
    static uint8_t srcBuf[1024];
    static uint8_t dstBuf[1024];

    renderCurrentPage(s);
    memcpy(srcBuf, mDisp.getBuffer(), sizeof srcBuf);

    uint8_t old = mPage;
    mPage = dest;
    renderCurrentPage(s);
    memcpy(dstBuf, mDisp.getBuffer(), sizeof dstBuf);
    mPage = old;

    /* rotation 1/3 → buffer is column-major (8 bytes per column) */
    bool colMajor = (mDisp.getRotation() & 1);

    constexpr uint8_t steps = 8;          // 8 frames ≈ 60 fps
    uint8_t* live = mDisp.getBuffer();

    for (uint8_t k = 0; k <= steps; ++k) {

        if (!colMajor) {                                /* row-major */
            uint8_t srcLines = 64 - k * 8;              // 8 rows / step
            memcpy(live,               srcBuf, srcLines * 16);
            memcpy(live + srcLines*16, dstBuf, (64 - srcLines) * 16);

        } else {                                        /* col-major */
            uint8_t srcCols = 128 - k * 16;             // 16 cols / step
            /* one column = 8 bytes in rotation-1 layout                */
            for (uint16_t c = 0; c < 128; ++c) {
                uint8_t* dstPtr = live   + c * 8;
                uint8_t* srcPtr = (c < srcCols)
                                ? srcBuf + c * 8
                                : dstBuf + c * 8;
                memcpy(dstPtr, srcPtr, 8);
            }
        }
        mDisp.display();
        delay(16);
    }
    mPage = dest;
}

/* ------------------------------------------------------------------ */
/*  PAGE 0 – SET / RPM                                                */
/* ------------------------------------------------------------------ */
void Sh1107Display::drawSetPointPage(const volatile SystemState& s)
{
    mDisp.clearDisplay(); mDisp.setFont();
    mDisp.setCursor(0, 0);
    mDisp.print(F("Meas "));
    mDisp.print(s.f_flow, 0); mDisp.print(F(" uL/min"));
    drawModeIcon(s);

    char buf[24];
    if (s.ctrlMode == ControlMode::CLOSED)
        snprintf(buf, sizeof buf, "%.0f uL/min", s.setFlow_uLmin);
    else
        snprintf(buf, sizeof buf, "%.1f rpm", s.setRpm);

    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal ")); printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0); mDisp.print('%');
}

/* ------------------------------------------------------------------ */
/*  PAGE 1 – MEASURED                                                 */
/* ------------------------------------------------------------------ */
void Sh1107Display::drawMeasuredPage(const volatile SystemState& s)
{
    mDisp.clearDisplay(); mDisp.setFont();
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    if (s.ctrlMode == ControlMode::CLOSED) {
        mDisp.print(s.setFlow_uLmin, 0); mDisp.print(F(" uL/min"));
    } else {
        mDisp.print(s.setRpm, 0);        mDisp.print(F(" rpm"));
    }

    const int16_t glyphW = 12;
    const int16_t arrowW = 6 * 2;
    int16_t xArrows = mDisp.width() - glyphW - arrowW - 2;
    mDisp.setCursor(xArrows, 0);
    mDisp.print((char)0x18); mDisp.print((char)0x19);
    drawModeIcon(s);

    char buf[20];
    snprintf(buf, sizeof buf, "%.0f uL/min", s.f_flow);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Cal ")); printPlusMinus(mDisp);
    mDisp.print(s.calScalar, 0); mDisp.print('%');
}

/* ------------------------------------------------------------------ */
/*  PAGE 2 – CAL %                                                    */
/* ------------------------------------------------------------------ */
void Sh1107Display::drawCalScalarPage(const volatile SystemState& s)
{
    mDisp.clearDisplay(); mDisp.setFont();
    mDisp.setCursor(0, 0);
    mDisp.print(F("Set "));
    if (s.ctrlMode == ControlMode::CLOSED) {
        mDisp.print(s.setFlow_uLmin, 0); mDisp.print(F(" uL/min"));
    } else {
        mDisp.print(s.setRpm, 0);        mDisp.print(F(" rpm"));
    }
    drawModeIcon(s);

    char buf[16]; buf[0] = 241;
    snprintf(buf + 1, sizeof buf - 1, "%.0f %%", s.calScalar);
    mDisp.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    mDisp.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    mDisp.setCursor((mDisp.width() - bw) / 2,
                    (mDisp.height() - bh) / 2 - 4);
    mDisp.print(buf);
    mDisp.setTextSize(1);

    mDisp.setCursor(0, mDisp.height() - 8);
    mDisp.print(F("Meas "));
    mDisp.print(s.f_flow, 0); mDisp.print(F(" uL/min"));
}

#endif /* ENABLE_SH1107 */
