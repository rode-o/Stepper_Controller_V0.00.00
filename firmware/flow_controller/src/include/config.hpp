#pragma once
#include <Arduino.h>

/*
 * File: config.h
 * Brief: Central configuration for display, pump driver, flow sensor,
 *        PID parameters, and other system-wide settings.
 */

// ---------------------------------------------------------------------------
// Constant Voltage Control      (legacy; unused in min_ctrl.cpp)
// ---------------------------------------------------------------------------
static const float kConstantVoltage = 80.0f;

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
static const uint8_t SSD1306_DISPLAY_ADDR = 0x3C;

// ---------------------------------------------------------------------------
// Bartels Pump Driver
// ---------------------------------------------------------------------------
static const uint8_t  BARTELS_DRIVER_ADDR   = 0x59;
static const uint8_t  BARTELS_PAGE_REGISTER = 0xFF;
static const uint8_t  BARTELS_CONTROL_DATA[] = {0x00, 0x3B, 0x01, 0x01};

static const float BARTELS_FREQ         = 300.0f;
static const float BARTELS_ABSOLUTE_MAX = 150.0f;
static const float BARTELS_MAX_VOLTAGE  = 150.0f;
static const float BARTELS_MIN_VOLTAGE  = 0.0f;

// ---------------------------------------------------------------------------
// Flow Sensor
// ---------------------------------------------------------------------------
static const uint8_t SLF_FLOW_SENSOR_ADDR     = 0x08;
static const uint8_t SLF_START_CMD            = 0x36;
static const uint8_t SLF_CALIBRATION_CMD_BYTE = 0x08;
static const uint8_t SLF_STOP_CMD             = 0x3F;
static const uint8_t SLF_STOP_BYTE            = 0xF9;

static const float SLF_SCALE_FACTOR_FLOW = 10000.0f;
static const float SLF_SCALE_FACTOR_TEMP = 200.0f;
static const float SLF_RUN_DURATION      = 604800.0f;   // s (7 days)

// ---------------------------------------------------------------------------
// Flow / Error Ranges
// ---------------------------------------------------------------------------
static const float FLOW_SP_MIN       = 0.0f;
static const float FLOW_SP_MAX       = 2.0f;
static const float ERROR_PERCENT_MIN = -50.0f;
static const float ERROR_PERCENT_MAX =  50.0f;
static const float FLOW_STEP_SIZE    = 0.05f;

// ---------------------------------------------------------------------------
// (Legacy) Exponential-gain parameters – UNUSED in min_ctrl.cpp
// ---------------------------------------------------------------------------
#define EXP_KP_A  0.0f
#define EXP_KP_K  0.0f
#define EXP_KP_B  0.0f
#define EXP_KP_C  0.0f

#define EXP_KI_A  0.001f
#define EXP_KI_K  0.23f
#define EXP_KI_B  100.0f
#define EXP_KI_C  0.0f

#define EXP_KD_A  0.0f
#define EXP_KD_K  0.0f
#define EXP_KD_B  0.0f
#define EXP_KD_C  0.0f

// ---------------------------------------------------------------------------
// Filter / Slope-matching parameters – UNUSED in new pipeline
// ---------------------------------------------------------------------------
static const float FILTER_T_REF          = 0.05f;
static const float FILTER_SECONDARY_A2   = 0.0f;
static const float FILTER_SECONDARY_K2   = 0.5f;
static const float FILTER_B2_GUESS       = 3.0f;
#define EMA_ALPHA 0.85f

// ---------------------------------------------------------------------------
// PID anti-windup / deriv filter (shared by other modes)
// ---------------------------------------------------------------------------
static const float PID_ANTIWINDUP_GAIN    = 0.1f;
static const float PID_DERIV_FILTER_ALPHA = 0.8f;

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------
static const float       FLUID_TIME_CONSTANT  = 0.05f;
static const float       LOOP_FREQ_FACTOR     = 15.0f;
static const unsigned long MAIN_LOOP_DELAY_MS =
    (unsigned long)((FLUID_TIME_CONSTANT / LOOP_FREQ_FACTOR) * 1000.0f);

constexpr uint32_t LOOP_INTERVAL_MS = 10;

// ---------------------------------------------------------------------------
// 24 V rail monitor
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_DETECT_24V   = A1;
constexpr float   R_SENSE_TOP      = 30'000.0f;
constexpr float   R_SENSE_BOTTOM   = 7'500.0f;

// ---------------------------------------------------------------------------
// Valve & LED GPIO map
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_VALVE_RUNNING_BUFF = 10;
constexpr uint8_t PIN_VALVE_CARTRIDGE_BYP= 11;
constexpr uint8_t PIN_VALVE_FLOW_SOURCE  = 12;
constexpr uint8_t VALVE_PIN_HIGH         = 255;
constexpr uint8_t VALVE_PIN_LOW          = 0;

constexpr uint8_t LED_24V_GOOD       = 45;
constexpr uint8_t LED_RUNNING_BUFF   = 39;
constexpr uint8_t LED_CARTRIDGE_BYP  = 37;
constexpr uint8_t LED_FLOW_SOURCE    = 35;
constexpr uint8_t LED_FLOWRATE_GOOD  = 43;
constexpr uint8_t LED_PUMP_INDICATOR = 41;

// ---------------------------------------------------------------------------
// DRV8825 pump pins
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_PUMP_ENA  = 4;
constexpr uint8_t PIN_PUMP_OPTO = 5;
constexpr uint8_t PIN_PUMP_DIR  = 6;
constexpr uint8_t PIN_PUMP_STEP = 7;

// ---------------------------------------------------------------------------
// Mechanical / stepper constants
// ---------------------------------------------------------------------------
constexpr uint8_t  ROLLERS     = 6;
constexpr uint8_t  VPR         = 42;      // ★ µL per rev (was 13)
constexpr uint8_t  TPS         = 2;       // gearbox teeth / rev
constexpr uint16_t SPR         = 200;     // full steps / rev
constexpr uint16_t MICROSTEP   = 32;      // ★ 1/32-step

// ---------------------------------------------------------------------------
// PID default gains (scalar mode) – match min_ctrl.cpp
// ---------------------------------------------------------------------------
constexpr float PID_KP = 0.20f;           // ★
constexpr float PID_KI = 0.05f;           // ★
constexpr float PID_KD = 0.00f;           // ★

// ---------------------------------------------------------------------------
// Flow / valve timing parameters
// ---------------------------------------------------------------------------
constexpr uint32_t VALVE_HIGH_TIME_MS    = 1'000;
constexpr uint16_t RATE_MIN_UL_MIN       = 200;
constexpr uint16_t RATE_MAX_UL_MIN       = 1'800;   // ★ was 1 300
constexpr uint8_t  DESIRED_TOLERANCE_PCT = 10;

// ---------------------------------------------------------------------------
// Human-readable serial tokens
// ---------------------------------------------------------------------------
#define VALVE_STR                 "VALVE"
#define VALVE_RUNNINGBUF_STR      "RUNBUF"
#define VALVE_CARTRIDGEBYPASS_STR "CARTBYP"
#define VALVE_FLOWSOURCE_STR      "FLOWSRC"
#define FLOWRATE_STR              "FLOWRATE"
#define FLOWSTATE_STR             "FLOWSTATE"
#define FLOWDIR_STR               "FLOWDIR"
#define ON_STR        "ON"
#define OFF_STR       "OFF"
#define FORWARD_STR   "FORWARD"
#define REVERSE_STR   "REVERSE"

// ---------------------------------------------------------------------------
// Function Prototypes
// ---------------------------------------------------------------------------
void loadDefaults();
