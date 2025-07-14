#include "page_set.hpp"
#include "../../common/common.hpp"          // drawCommonOverlay

void UI::Pages::drawSet(Adafruit_SH1107& d,
                        const volatile SystemState& s)
{
    /* ─── heading : current measured flow ───────────────────────── */
    d.clearDisplay();
    d.setFont();
    d.setCursor(0, 0);
    d.print(F("Meas "));
    d.print(s.f_flow, 0);
    d.print(F(" uL/min"));

    /* ─── centred set-point (always flow) ───────────────────────── */
    char buf[24];
    snprintf(buf, sizeof buf, "%.0f uL/min", s.setFlow_uLmin);

    d.setTextSize(2);                        // enlarge temporarily
    int16_t bx, by; uint16_t bw, bh;
    d.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);

    d.setCursor((d.width()  - bw) / 2,
                (d.height() - bh) / 2);
    d.print(buf);
    d.setTextSize(1);                        // restore size

    /* ─── overlay : mode glyph + pump / delivered vol ──────────── */
    drawCommonOverlay(d, s);
}
