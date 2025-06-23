//_include.hpp

#pragma once

#include "config.hpp"
#include "pins.hpp"
#include "fw_version.hpp"
#include "system_state/system_state.hpp"


//________________core__________________
    //#define ENABLE_PID
    //#define ENABLE_FILTER
    //#define ENABLE_GAIN

//_________________ctrl_________________
    #define ENABLE_MIN_CTRL
    //#define ENABLE_EXP_CTRL
    //#define ENABLE_CONSTANT_VOLTAGE_CTRL

//_______________devices________________

    //_________display__________________
    #define ENABLE_SH1107
    //#define ENABLE_SSD1306

    //_________flow_sensor______________
    #define ENABLE_SFL3S_0600F
    //#define ENABLE_CUSTOM

    //_________pump_drivers_____________
    #define ENABLE_DRV8825
    //#define ENABLE_MP_LOWDRIVER   

    //________input_devices_____________
    #define ENABLE_BUTTONS_TWO
    //#define ENABLE_BUTTONS_SIX
    
//________________utils_________________

    //___________ serial________________
    //#define ENABLE_SERIAL_CMD
    #define ENABLE_SERIAL_RPT