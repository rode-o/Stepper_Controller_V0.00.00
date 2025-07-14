#include <Arduino.h>
#include "serial_rpt.hpp"
#include "../../main.hpp"          // SystemState definition

namespace SerialRpt
{
    void emitJSON(const volatile SystemState& st)
    {
        Serial.print('{');

        /* ── identity & time ───────────────────────────────────── */
        Serial.print(F("\"id\":")); Serial.print(st.deviceId);
        Serial.print(F(",\"t\":"));  Serial.print(st.currentTimeMs);

        /* ── set-point (dual-mode) ─────────────────────────────── */
        Serial.print(F(",\"sp\":"));
        Serial.print(st.setFlow_uLmin, 0);           // always flow now
        Serial.print(F(",\"unit\":\"uL/min\""));

        /* ── measured flows ───────────────────────────────────── */
        Serial.print(F(",\"r_flw\":")); Serial.print(st.r_flow, 0);  // raw
        Serial.print(F(",\"f_flw\":")); Serial.print(st.f_flow, 0);  // filtered

        /* ── drive command snapshot ───────────────────────────── */
        Serial.print(F(",\"rpm_cmd\":")); Serial.print(st.rpmCmd, 1);
        Serial.print(F(",\"rpm_sp\":"));  Serial.print(st.setRpm, 1);
        Serial.print(F(",\"sps\":"));     Serial.print(st.spsCmd, 0);
        Serial.print(F(",\"top\":"));     Serial.print(st.topCmd);

        /* ── calibration parameters ───────────────────────────────── */
        Serial.print(F(",\"cal_pct\":")); Serial.print(st.calScalar, 2);             
        Serial.print(F(",\"vpr\":"));     Serial.print(st.vpr_uL_rev, 1);

        /* ── running totals ───────────────────────────────────── */
        Serial.print(F(",\"vol_uL\":"));  Serial.print(st.volume_uL, 0);
        Serial.print(F(",\"mass_g\":"));  Serial.print(st.mass_g, 3);

        /* ── flags ────────────────────────────────────────────── */
        Serial.print(F(",\"pumpOn\":"));  Serial.print(st.pumpEnabled ? 1 : 0);

        Serial.println('}');
    }
}   // namespace SerialRpt
