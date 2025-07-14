#pragma once
#include <Adafruit_SH110X.h>
#include "../../include/_include.hpp"

#include "../common/common.hpp"
#include "page_set/page_set.hpp"
#include "page_measured/page_measured.hpp"
#include "page_cal_wizard/page_cal_wizard.hpp"
#include "page_rpm_set/page_rpm_set.hpp"

namespace UI::Pages {
    constexpr uint8_t NUM_PAGES = 4;      // handy for DisplayMgr
}
