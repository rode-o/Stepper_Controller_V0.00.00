#include "common.hpp"

/*  _include.hpp is already pulled in by common.hpp,
    so the extra include is unnecessary (and had the wrong path). */

void UI::Pages::drawCommonOverlay(Adafruit_SH1107& d,
                                  const volatile SystemState& s)
{
    /* ── mode glyph ────────────────────────────────────────── */
    d.setTextSize(2);
    d.setCursor(d.width() - 12, 0);
    d.print((s.ctrlMode == ControlMode::CLOSED) ? 'C' : 'O');
    d.setTextSize(1);

    /* ── pump icon + delivered volume (bottom-right) ───────── */
    const int16_t x = d.width()  - 16;
    const int16_t y = d.height() - 16;

    char buf[16];
    snprintf(buf, sizeof buf, "%luuL",
             static_cast<unsigned long>(s.volume_uL));

    int16_t bx, by; uint16_t bw, bh;
    d.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    d.setCursor(x - 2 - bw, y + 2);
    d.print(buf);

    if (State::isPumpEnabled())
        d.fillRect(x, y, 12, 12, SH110X_WHITE);
    else
        d.drawRect(x, y, 12, 12, SH110X_WHITE);
}
