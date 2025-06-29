#pragma once
#include <Arduino.h>          // delay()
#include <cmath>              // fabsf()
#include <vector>
#include "egc_types.hpp"      // IFlowSensor, IPumpDriver, EgcParams

namespace egc {

/*  Affine least-squares helper (y = a·x + b)  */
ScaleAffine fitAffine(const std::vector<std::pair<float,float>>& pts);

/*  Open-loop pulse + analytic Ki-curve solve   */
bool runCalibration(const CalConfig& cfg,
                    IFlowSensor&     sensor,
                    IPumpDriver&     pump,
                    EgcParams&       out);

} // namespace egc
