#pragma once

/*  ↱  we are in  src/ui/display_mgr/…                */
/*  .. ─────────────── go up twice to reach project root */
#include "../../devices/displays/sh1107/sh1107.hpp"
#include "../pages/_pages.hpp"          // exposes Pages::NUM_PAGES
#include "../../include/_include.hpp"

/*─────────────────────────────────────────────────────────────*/
/*  DisplayMgr – UI-layer orchestrator for the SH1107 OLED     */
/*─────────────────────────────────────────────────────────────*/
namespace UI {

class DisplayMgr
{
public:
    bool begin();                // initialise OLED (+ first render)
    void nextPage();             // cyclic ++  (ButtonsTwo.OK handler)
    void setPage(uint8_t p);     // direct jump
    void tick();                 // call @ 60-100 Hz for wizards etc.
    void redraw();               // force a repaint

private:
    void renderCurrent();

    Sh1107Display mDisp;
    uint8_t       mPage {0};     // current page index (0 … Pages::NUM_PAGES-1)
};

} // namespace UI
