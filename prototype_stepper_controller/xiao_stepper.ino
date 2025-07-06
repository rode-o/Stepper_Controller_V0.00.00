
#include "config.h"
#include "StepperController.h"
#include "CommandParser.h"

StepperController motor(STEP_PIN, DIR_PIN, EN_PIN);
CommandParser parser(motor);

void setup() {
    Serial.begin(115200);
    motor.begin(STEPS_PER_REV, MICROSTEP);
    Serial.println("DRV8825 READY");
}

void loop() {
    parser.pollSerial(Serial);
    motor.service();
}
