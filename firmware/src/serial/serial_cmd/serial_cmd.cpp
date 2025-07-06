/*─────────────────────────────────────────────────────────────
 *  serial_cmd.cpp – host-command parser & dispatcher
 *───────────────────────────────────────────────────────────*/
#include "serial_cmd.hpp"
#include "../../devices/pump_drivers/_pump_drivers.hpp"   // PumpDrv::stop()
#include "../../devices/rgb/rgb.hpp"                      // LEDColour / RGB

using namespace State;                                    // g_state helpers

/* ── constants ──────────────────────────────────────────── */
static constexpr uint8_t BUF_LEN   = 100;
static constexpr int     RATE_MIN  = 200;
static constexpr int     RATE_MAX  = 1300;

/* ── local vars ─────────────────────────────────────────── */
namespace {
char    buf[BUF_LEN + 1];
uint8_t idx = 0;

/* ---------- helpers ------------------------------------- */
inline void handleQuery()                 // host sends "?" to read flow
{
    Serial.println(isPumpEnabled() ? read().f_flow : 0);
}

inline void setFlowRate(int rate)
{
    if (rate < RATE_MIN || rate > RATE_MAX) return;
    setFlow(rate);                        // persist – controllers will act
}

inline void setFlowState(bool on)
{
    /* update stored state */
    setPumpEnabled(on);

    /* refresh LED immediately */
    LEDColour c = on ? LED_GREEN : LED_RED;
    setLEDColour(c);                      // record in State
    RGB::setColour(c);                    // drive NeoPixel(s)

    /* safe shut-down when turning OFF */
    if (!on) PumpDrv::stop();
}

/* ---------- dispatcher ---------------------------------- */
void dispatch(char *cmd)
{
    if (strchr(cmd, '?')) { handleQuery(); return; }

    char *key = strtok(cmd, "=");
    if (!key) return;

    if (strcmp(key, "FLOWRATE") == 0) {
        setFlowRate(atoi(strtok(nullptr, "=")));
    }
    else if (strcmp(key, "FLOWSTATE") == 0) {
        char *s = strtok(nullptr, "=");
        if (s) setFlowState(strcmp(s, "ON") == 0);
    }
    /* anything else is ignored */
}
} // anonymous namespace

/* ── public API ─────────────────────────────────────────── */
void SerialCmd::begin(uint32_t baud)
{
    Serial.begin(baud);
    idx = 0;
}

void SerialCmd::poll()
{
    while (Serial.available()) {
        char c = Serial.read();
        buf[idx] = c;

        if (c == '\n' || c == '\r') {
            buf[idx] = '\0';
            dispatch(buf);
            idx = 0;
        } else if (++idx >= BUF_LEN) {
            idx = 0;                      // overflow guard
        }
    }
}
