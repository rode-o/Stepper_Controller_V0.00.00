#include "CommandParser.h"
#include "config.h"

/* ------------------------------------------------------------------ */
/*  Poll characters from the serial stream and dispatch on EOL        */
/* ------------------------------------------------------------------ */
void CommandParser::pollSerial(Stream& s)
{
    while (s.available()) {
        char c = s.read();
        if (c == '\n' || c == '\r') {
            if (_buffer.length()) {
                handleCommand(_buffer, s);
                _buffer = "";
            }
        } else {
            _buffer += c;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Parse and execute a single command line                           */
/* ------------------------------------------------------------------ */
void CommandParser::handleCommand(const String& line, Stream& s)
{
    char cmd   = toupper(line.charAt(0));
    long value = 0;
    if (line.length() > 2)               // skip the space after the letter
        value = line.substring(2).toInt();

    switch (cmd)
    {
        case 'V':                         // V <rpm>  — live RPM change
            _motor.setRPM(value);
            s.printf("OK RPM %ld\n", value);
            break;

        case 'R':                         // R 1 ==> CW  |  R 0 ==> CCW
            _motor.runContinuous(true, value > 0);
            s.println("OK RUN");
            break;

        case 'G':                         // G <steps>  — relative micro-steps
            _motor.moveRelative(value);
            s.printf("OK MOVE %ld\n", value);
            break;

        case 'A': {                       // A <deg>    — rotate degrees
            float deg   = value;
            long steps  = (long)((deg / 360.0f) * STEPS_PER_REV * MICROSTEP);
            _motor.moveRelative(steps);
            s.printf("OK DEG %ld\n", value);
            break;
        }

        case 'E':                         // E          — enable coils
            _motor.enable();
            s.println("OK ENABLE");
            break;

        case 'D':                         // D          — disable / stop
        case 'S':                         // S          — stop (alias)
            _motor.runContinuous(false);  // stop any continuous motion
            _motor.disable();             // de-energise coils
            s.println("OK STOP");
            break;

        case '?':                         // ?          — status query
            s.printf("POS %ld RPM %.1f ACT %.1f BUSY %d\n",
                     _motor.getPosition(),
                     _motor.getRPMTarget(),
                     _motor.getRPMActual(),
                     _motor.isBusy());
            break;

        default:
            s.println("ERR");
            break;
    }
}
