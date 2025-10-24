#include "addons/taiko.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "helper.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "pico/time.h"

bool Taiko::available() {
    return Storage::getInstance().getAddonOptions().taikoAddonOptions.enabled;
}

void Taiko::setup() {
    const TaikoAddonOptions& options = Storage::getInstance().getAddonOptions().taikoAddonOptions;

    // Initialize timing configuration
    debounce_millis = options.debounceMillis > 0 ? options.debounceMillis : TAIKO_DEBOUNCE_MILLIS;
    key_timeout_millis = options.keyTimeoutMillis > 0 ? options.keyTimeoutMillis : TAIKO_KEY_TIMEOUT_MILLIS;
    global_debounce_time = 0;

    // Initialize anti-ghosting configuration
    anti_ghosting_sides = options.antiGhostingSides;
    anti_ghosting_center = options.antiGhostingCenter;

    // Array of pin configurations from proto
    int32_t pins[TAIKO_SENSOR_COUNT] = {
        options.sensor1Pin,
        options.sensor2Pin,
        options.sensor3Pin,
        options.sensor4Pin
    };

    // Array of button mask configurations
    uint32_t buttons[TAIKO_SENSOR_COUNT] = {
        options.sensor1Button > 0 ? options.sensor1Button : TAIKO_SENSOR1_BUTTON,
        options.sensor2Button > 0 ? options.sensor2Button : TAIKO_SENSOR2_BUTTON,
        options.sensor3Button > 0 ? options.sensor3Button : TAIKO_SENSOR3_BUTTON,
        options.sensor4Button > 0 ? options.sensor4Button : TAIKO_SENSOR4_BUTTON
    };

    // Array of light thresholds
    uint16_t thresholds_light[TAIKO_SENSOR_COUNT] = {
        options.sensor1ThresholdLight > 0 ? options.sensor1ThresholdLight : TAIKO_SENSOR1_THRESHOLD_LIGHT,
        options.sensor2ThresholdLight > 0 ? options.sensor2ThresholdLight : TAIKO_SENSOR2_THRESHOLD_LIGHT,
        options.sensor3ThresholdLight > 0 ? options.sensor3ThresholdLight : TAIKO_SENSOR3_THRESHOLD_LIGHT,
        options.sensor4ThresholdLight > 0 ? options.sensor4ThresholdLight : TAIKO_SENSOR4_THRESHOLD_LIGHT
    };

    // Array of heavy thresholds
    uint16_t thresholds_heavy[TAIKO_SENSOR_COUNT] = {
        options.sensor1ThresholdHeavy > 0 ? options.sensor1ThresholdHeavy : TAIKO_SENSOR1_THRESHOLD_HEAVY,
        options.sensor2ThresholdHeavy > 0 ? options.sensor2ThresholdHeavy : TAIKO_SENSOR2_THRESHOLD_HEAVY,
        options.sensor3ThresholdHeavy > 0 ? options.sensor3ThresholdHeavy : TAIKO_SENSOR3_THRESHOLD_HEAVY,
        options.sensor4ThresholdHeavy > 0 ? options.sensor4ThresholdHeavy : TAIKO_SENSOR4_THRESHOLD_HEAVY
    };

    // Initialize all sensors
    for (int i = 0; i < TAIKO_SENSOR_COUNT; i++) {
        sensors[i].adc_pin = pins[i];
        sensors[i].button_mask = buttons[i];
        sensors[i].threshold_light = thresholds_light[i];
        sensors[i].threshold_heavy = thresholds_heavy[i];
        sensors[i].last_press_time = 0;
        sensors[i].is_pressed = false;

        // Initialize ADC channel if pin is valid
        if (isValidPin(sensors[i].adc_pin)) {
            sensors[i].adc_channel = sensors[i].adc_pin - ADC_PIN_OFFSET;
            adc_gpio_init(sensors[i].adc_pin);
        } else {
            sensors[i].adc_channel = -1;
        }
    }
}

void Taiko::process() {
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    uint32_t current_time = getCurrentTimeMillis();

    // Anti-ghosting state flags
    // In STM32 code:
    // - sides = sensor 0 OR sensor 3 (left/right edges)
    // - center = sensor 1 OR sensor 2 (center left/right)
    bool sides_active = sensors[0].is_pressed || sensors[3].is_pressed;
    bool center_active = sensors[1].is_pressed || sensors[2].is_pressed;

    bool global_debounce_elapsed = (current_time - global_debounce_time) > debounce_millis;

    // Process each sensor
    for (int i = 0; i < TAIKO_SENSOR_COUNT; i++) {
        // Skip if pin is not configured
        if (!isValidPin(sensors[i].adc_pin)) {
            continue;
        }

        // Calculate time elapsed since this sensor was last pressed
        uint32_t sensor_elapsed = current_time - sensors[i].last_press_time;

        // First, maintain any buttons that are currently pressed
        // This MUST happen every frame because gamepad->state.buttons gets reset to 0
        if (sensors[i].is_pressed) {
            if (sensor_elapsed <= key_timeout_millis) {
                // Keep the button pressed until timeout expires
                gamepad->state.buttons |= sensors[i].button_mask;
            } else {
                // Timeout expired, release the button
                sensors[i].is_pressed = false;
            }
        }

        // Only check for new hits if the button is not currently pressed
        if (!sensors[i].is_pressed) {
            // Read ADC value for this sensor
            uint16_t adc_value = readSensor(i);

            // Anti-ghosting check based on sensor position
            bool anti_ghost_ok = true;
            if (anti_ghosting_sides || anti_ghosting_center) {
                // Sensor 0 and 3 are "sides", sensor 1 and 2 are "center"
                if (i == 0 || i == 3) {
                    // Side sensor: check if center is active
                    if (anti_ghosting_center && center_active) {
                        anti_ghost_ok = false;
                    }
                } else {
                    // Center sensor: check if sides are active
                    if (anti_ghosting_sides && sides_active) {
                        anti_ghost_ok = false;
                    }
                }
            }

            // Determine if this is a valid hit
            // Valid if: anti-ghosting OK AND (light threshold met with debounce OR heavy threshold met)
            bool is_light_hit = (adc_value > sensors[i].threshold_light) && global_debounce_elapsed;
            bool is_heavy_hit = (adc_value > sensors[i].threshold_heavy);
            bool is_hit = anti_ghost_ok && (sensor_elapsed > debounce_millis) && (is_light_hit || is_heavy_hit);

            if (is_hit) {
                // New hit detected - press the button and record timestamp
                gamepad->state.buttons |= sensors[i].button_mask;
                sensors[i].is_pressed = true;
                sensors[i].last_press_time = current_time;
                global_debounce_time = current_time;
            }
        }
    }
}

uint16_t Taiko::readSensor(int sensor_index) {
    if (sensor_index < 0 || sensor_index >= TAIKO_SENSOR_COUNT) {
        return 0;
    }

    taiko_sensor& sensor = sensors[sensor_index];
    if (sensor.adc_channel < 0 || sensor.adc_channel > 3) {
        return 0;
    }

    adc_select_input(sensor.adc_channel);
    return adc_read();
}

uint32_t Taiko::getCurrentTimeMillis() {
    // RP2040 time function - returns milliseconds since boot
    return to_ms_since_boot(get_absolute_time());
}