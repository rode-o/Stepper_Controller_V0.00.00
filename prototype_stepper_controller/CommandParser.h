
#pragma once
#include <Arduino.h>
#include "StepperController.h"

class CommandParser {
public:
    explicit CommandParser(StepperController& motor) : _motor(motor) {}

    void pollSerial(Stream& s);

private:
    StepperController& _motor;
    String _buffer;
    void handleCommand(const String& line, Stream& s);
};
