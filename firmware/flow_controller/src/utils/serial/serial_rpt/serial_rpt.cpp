#include <Arduino.h>
#include "serial_rpt.hpp"

namespace SerialRpt
{
    void emitJSON(const volatile SystemState& st)
    {
        Serial.print('{');

        Serial.print(F("\"t\":"));       Serial.print(st.currentTimeMs);
        Serial.print(F(",\"sp\":"));     Serial.print(st.setpoint, 2);
        Serial.print(F(",\"pv\":"));     Serial.print(st.flow, 2);
        Serial.print(F(",\"rpm\":"));    Serial.print(st.rpmCmd, 1);
        Serial.print(F(",\"sps\":"));    Serial.print(st.spsCmd, 0);
        Serial.print(F(",\"err%\":"));   Serial.print(st.errorPercent, 0);
        Serial.print(F(",\"vol_uL\":")); Serial.print(st.volume_uL, 0);
        Serial.print(F(",\"mass_g\":")); Serial.print(st.mass_g,   3);
        Serial.print(F(",\"on\":"));     Serial.print(State::isPumpEnabled() ? 1 : 0);

        Serial.println('}');
    }
}   // namespace SerialRpt
