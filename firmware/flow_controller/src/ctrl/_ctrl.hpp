#pragma once

/* interface comes first */
#include "iface_ctrl/iface_ctrl.hpp"      // defines IController

/* concrete implementations */
#include "closed_ctrl/closed_ctrl.hpp"
#include "main_ctrl/main_ctrl.hpp"
#include "open_ctrl/open_ctrl.hpp"

/* shared math / filters */
#include "utils_ctrl/utils_ctrl.hpp"
