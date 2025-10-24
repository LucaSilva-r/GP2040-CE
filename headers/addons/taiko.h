#ifndef _Taiko_H
#define _Taiko_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"
#include "types.h"

#ifndef TAIKO_ENABLED
#define TAIKO_ENABLED 0
#endif

// Default ADC pins (disabled by default)
#ifndef TAIKO_SENSOR1_PIN
#define TAIKO_SENSOR1_PIN -1
#endif

#ifndef TAIKO_SENSOR2_PIN
#define TAIKO_SENSOR2_PIN -1
#endif

#ifndef TAIKO_SENSOR3_PIN
#define TAIKO_SENSOR3_PIN -1
#endif

#ifndef TAIKO_SENSOR4_PIN
#define TAIKO_SENSOR4_PIN -1
#endif

// Default button mappings (B1=A, B2=B, B3=X, B4=Y)
#ifndef TAIKO_SENSOR1_BUTTON
#define TAIKO_SENSOR1_BUTTON GAMEPAD_MASK_B1
#endif

#ifndef TAIKO_SENSOR2_BUTTON
#define TAIKO_SENSOR2_BUTTON GAMEPAD_MASK_B2
#endif

#ifndef TAIKO_SENSOR3_BUTTON
#define TAIKO_SENSOR3_BUTTON GAMEPAD_MASK_B3
#endif

#ifndef TAIKO_SENSOR4_BUTTON
#define TAIKO_SENSOR4_BUTTON GAMEPAD_MASK_B4
#endif

// Default thresholds (matching STM32 values)
#ifndef TAIKO_SENSOR1_THRESHOLD_LIGHT
#define TAIKO_SENSOR1_THRESHOLD_LIGHT 1400
#endif

#ifndef TAIKO_SENSOR2_THRESHOLD_LIGHT
#define TAIKO_SENSOR2_THRESHOLD_LIGHT 600
#endif

#ifndef TAIKO_SENSOR3_THRESHOLD_LIGHT
#define TAIKO_SENSOR3_THRESHOLD_LIGHT 700
#endif

#ifndef TAIKO_SENSOR4_THRESHOLD_LIGHT
#define TAIKO_SENSOR4_THRESHOLD_LIGHT 1400
#endif

#ifndef TAIKO_SENSOR1_THRESHOLD_HEAVY
#define TAIKO_SENSOR1_THRESHOLD_HEAVY 3600
#endif

#ifndef TAIKO_SENSOR2_THRESHOLD_HEAVY
#define TAIKO_SENSOR2_THRESHOLD_HEAVY 2600
#endif

#ifndef TAIKO_SENSOR3_THRESHOLD_HEAVY
#define TAIKO_SENSOR3_THRESHOLD_HEAVY 2700
#endif

#ifndef TAIKO_SENSOR4_THRESHOLD_HEAVY
#define TAIKO_SENSOR4_THRESHOLD_HEAVY 3600
#endif

// Default timing (matching STM32 values)
#ifndef TAIKO_DEBOUNCE_MILLIS
#define TAIKO_DEBOUNCE_MILLIS 45
#endif

#ifndef TAIKO_KEY_TIMEOUT_MILLIS
#define TAIKO_KEY_TIMEOUT_MILLIS 30
#endif

// Anti-ghosting defaults
#ifndef TAIKO_ANTI_GHOSTING_SIDES
#define TAIKO_ANTI_GHOSTING_SIDES 1
#endif

#ifndef TAIKO_ANTI_GHOSTING_CENTER
#define TAIKO_ANTI_GHOSTING_CENTER 1
#endif

// Module Name
#define TaikoName "Taiko"

#define ADC_MAX ((1 << 12) - 1) // 4095
#define ADC_PIN_OFFSET 26
#define TAIKO_SENSOR_COUNT 4

// Taiko sensor instance
typedef struct
{
    Pin_t adc_pin;              // GPIO pin number (26-29)
    Pin_t adc_channel;          // ADC channel (0-3)
    uint32_t button_mask;       // Gamepad button mask to trigger
    uint16_t threshold_light;   // Light hit threshold (ADC value)
    uint16_t threshold_heavy;   // Heavy hit threshold (ADC value)
    uint32_t last_press_time;   // Last press timestamp in milliseconds
    bool is_pressed;            // Current button state
} taiko_sensor;

class Taiko : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Taiko Setup
    virtual void process();     // Taiko Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return TaikoName; }
private:
    uint16_t readSensor(int sensor_index);
    uint32_t getCurrentTimeMillis();
    taiko_sensor sensors[TAIKO_SENSOR_COUNT];
    uint32_t global_debounce_time;
    uint32_t debounce_millis;
    uint32_t key_timeout_millis;
    bool anti_ghosting_sides;
    bool anti_ghosting_center;
};

#endif  // _Taiko_H
