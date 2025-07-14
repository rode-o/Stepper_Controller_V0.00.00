#include "display_mgr.hpp"

namespace UI {

bool DisplayMgr::begin()
{
    if (!mDisp.begin()) return false;
    renderCurrent();
    return true;
}

void DisplayMgr::setPage(uint8_t p)
{
    mPage = p % Pages::NUM_PAGES;   // wrap using the central constant
    renderCurrent();
}

void DisplayMgr::nextPage()
{
    setPage(mPage + 1);
}

void DisplayMgr::tick()
{
    /* pages with their own sub-state machines */
    if      (mPage == 2) Pages::tickCalWizard();
    else if (mPage == 3) Pages::tickRpmSet();

    renderCurrent();
}

void DisplayMgr::redraw()
{
    renderCurrent();
}

void DisplayMgr::renderCurrent()
{
    auto& d = mDisp.raw();                     // convenience alias

    switch (mPage) {
        case 0: Pages::drawSet       (d, ::State::read()); break;
        case 1: Pages::drawMeasured  (d, ::State::read()); break;
        case 2: Pages::drawCalWizard (d, ::State::read()); break;
        case 3: Pages::drawRpmSet    (d, ::State::read()); break;
        default: break;   // should never happen
    }

    /* common overlay (mode glyph + pump/volume) */
    Pages::drawCommonOverlay(d, ::State::read());

    mDisp.display();
}

} // namespace UI
