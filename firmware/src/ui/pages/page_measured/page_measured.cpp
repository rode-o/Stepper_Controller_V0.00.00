#include "page_measured.hpp"
#include "../../common/common.hpp"          // drawCommonOverlay

void UI::Pages::drawMeasured(Adafruit_SH1107& d,
                             const volatile SystemState& s)
{
    /* ─── heading : set-point read-back ─────────────────────────── */
    d.clearDisplay();
    d.setFont();
    d.setCursor(0, 0);
    d.print(F("Set "));
    /* always show flow, regardless of mode */
    d.print(s.setFlow_uLmin, 0);
    d.print(F(" uL/min"));

    /* up / down arrow glyphs (right-aligned) ---------------------- */
    char arrows[] = { char(0x18), char(0x19), '\0' };   // "▲▼"
    int16_t ax, ay; uint16_t aw, ah;
    d.getTextBounds(arrows, 0, 0, &ax, &ay, &aw, &ah);
    const int16_t glyphW = 12;         // width of ‘C’ / ‘O’ glyph
    d.setCursor(d.width() - glyphW - aw - 2, 0);
    d.print(arrows);

    /* ─── centred measured flow ─────────────────────────────────── */
    char buf[20];
    snprintf(buf, sizeof buf, "%.0f uL/min", s.f_flow);

    d.setTextSize(2);                        // enlarge temporarily
    int16_t bx, by; uint16_t bw, bh;
    d.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);

    d.setCursor((d.width()  - bw) / 2,
                (d.height() - bh) / 2);
    d.print(buf);
    d.setTextSize(1);                        // restore size

    /* ─── overlay (mode glyph + pump icon/volume) ──────────────── */
    drawCommonOverlay(d, s);
}
