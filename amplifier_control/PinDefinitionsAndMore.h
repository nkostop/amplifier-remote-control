/*
 *  PinDefinitionsAndMore.h
 *
 *  Contains pin definitions for IRremote examples for various platforms
 */
 


#if defined(AMPLIFIER)

// IR Configuration
#define IR_RECEIVE_PIN      3
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 13
#define ACCEPT_DATA_CODE 0x43
#define DENY_DATA_CODE_1 0x40
#define DENY_DATA_CODE_2 0x44
#define POWER_CODE 0x46
#define IR_ADDRESS 0x00

// Pin Definitions
#define POWER_PIN       2
#define RELAY_PIN       12
#define POWER_LED_PIN   8
#define THERMISTOR1_PIN 0
#define THERMISTOR2_PIN 1

// Thermistor Configuration
#define THERMISTOR_WIRING_NUMBER 1
#define THERMISTOR_SERIES_RESISTOR 10000.0
#define THERMISTOR_NOMINAL 10000.0
#define THERMISTOR_B_COEFFICIENT 4300.0
#define THERMISTOR_T0_KELVIN 298.15

// Thermal Protection Thresholds (°C)
#define THERMAL_SHUTDOWN_TEMP 74.0
#define THERMAL_RESTART_TEMP 65.0
#define THERMAL_LOW_TEMP 50.0
#define THERMAL_SHUTDOWN_DELAY 1

// Timing Configuration
#define LOW_THERMAL_CHECK_INTERVAL 30
#define SLEEP_LOOP_COUNT 100

#endif

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
