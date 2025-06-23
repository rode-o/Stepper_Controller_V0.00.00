#ifndef SERIAL_RPT_HPP
#define SERIAL_RPT_HPP

#include "../../../include/system_state/system_state.hpp"

namespace SerialRpt {
void emitJSON(const volatile SystemState& st);
} // namespace SerialRpt

#endif /* SERIAL_RPT_HPP */
