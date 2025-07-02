#include <Arduino.h>
#include "serial_rpt.hpp"
#include "../../../main.hpp"          // SystemState definition

namespace SerialRpt
{
    void emitJSON(const volatile SystemState& st)
    {
        Serial.print('{');

        /* core timing */
        Serial.print(F("\"t\":"));           Serial.print(st.currentTimeMs);

        /* set-point (dual-mode) */
        Serial.print(F(",\"sp\":"));
        if (st.ctrlMode == ControlMode::CLOSED) {
            Serial.print(st.setFlow_uLmin, 0);
            Serial.print(F(",\"unit\":\"uL/min\""));
        } else {    // OPEN
            Serial.print(st.setRpm, 0);
            Serial.print(F(",\"unit\":\"rpm\""));
        }

        /* flows */
        Serial.print(F(",\"r_flw\":"));      Serial.print(st.r_flow,   0);   // raw
        Serial.print(F(",\"f_flw\":"));      Serial.print(st.f_flow,   0);   // filtered

        /* drive commands */
        Serial.print(F(",\"rpm\":"));        Serial.print(st.rpmCmd, 1);
        Serial.print(F(",\"sps\":"));        Serial.print(st.spsCmd, 0);
        Serial.print(F(",\"top\":"));        Serial.print(st.topCmd);

        /* calibration scalar */
        Serial.print(F(",\"cal%\":"));       Serial.print(st.calScalar, 0);

        /* totals */
        Serial.print(F(",\"vol_uL\":"));     Serial.print(st.volume_uL, 0);
        Serial.print(F(",\"mass_g\":"));     Serial.print(st.mass_g, 3);

        /* flags */
        Serial.print(F(",\"pumpOn\":"));     Serial.print(st.pumpEnabled ? 1 : 0);

        Serial.println('}');
    }
}   // namespace SerialRpt
