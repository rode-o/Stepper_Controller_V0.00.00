#include "page_cal_wizard.hpp"

namespace UI::Pages {

void drawCalWizard(Adafruit_SH1107& d,
                   const volatile SystemState& s)
{
    /* delegate 100 % of the drawing to the wizard object */
    gCalWizard.draw(d, s);
}

void tickCalWizard()
{
    gCalWizard.tick();
}

bool handleCalButton(UI::Btn b)
{
    return gCalWizard.handleButton(b);
}

} // namespace UI::Pages
