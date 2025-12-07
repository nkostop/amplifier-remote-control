# Simple amplifier control

Remote control for audio amplifier using front panel push/LED switch, relay for power control, IR remote, and thermistors for temperature monitoring.

## Hardware
- Arduino Mini Pro 5V (https://www.sparkfun.com/products/11113)
- IR Receiver Diode - TSOP38238 (https://www.sparkfun.com/products/10266)
- Infrared Remote Control (https://www.sparkfun.com/products/14865)
- Thermistors EPCOS B57045K103K (https://gr.mouser.com/ProductDetail/EPCOS-TDK/B57045K103K) - [Datasheet](http://grobotronics.com/images/companies/1/B57045.pdf)
- Momentary metal switch with LED (standard)

## Libraries

Install these libraries via Arduino IDE Library Manager:

| Library | Version | Description |
|---------|---------|-------------|
| IRremote | 4.x | IR remote control decoding |

Built-in libraries (no installation needed):
- `avr/sleep.h` - AVR sleep mode utilities

## Configuration

All configuration is centralized in `PinDefinitionsAndMore.h`:

### Pin Definitions

| Setting | Default | Description |
|---------|---------|-------------|
| `IR_RECEIVE_PIN` | 3 | IR receiver data pin |
| `ALTERNATIVE_IR_FEEDBACK_LED_PIN` | 13 | LED feedback for IR reception |
| `POWER_PIN` | 2 | Front panel power button input |
| `RELAY_PIN` | 12 | Relay control output |
| `POWER_LED_PIN` | 8 | Front panel LED output |
| `THERMISTOR1_PIN` | 0 | First thermistor analog input (A0) |
| `THERMISTOR2_PIN` | 1 | Second thermistor analog input (A1) |

### IR Remote Codes

| Setting | Default | Description |
|---------|---------|-------------|
| `IR_ADDRESS` | 0x00 | Expected IR remote address |
| `POWER_CODE` | 0x46 | Power toggle command |
| `ACCEPT_DATA_CODE` | 0x43 | Enable IR command processing |
| `DENY_DATA_CODE_1` | 0x40 | Disable IR command processing |
| `DENY_DATA_CODE_2` | 0x44 | Disable IR command processing (alt) |

### Thermistor Calibration

| Setting | Default | Description |
|---------|---------|-------------|
| `THERMISTOR_WIRING_NUMBER` | 1 | Wiring mode (0 or 1, see code comments) |
| `THERMISTOR_SERIES_RESISTOR` | 10000.0 | Series resistor value in ohms |
| `THERMISTOR_NOMINAL` | 10000.0 | Thermistor resistance at 25°C |
| `THERMISTOR_B_COEFFICIENT` | 4300.0 | Beta coefficient from datasheet |
| `THERMISTOR_T0_KELVIN` | 298.15 | Reference temperature in Kelvin (25°C) |

### Thermal Protection

| Setting | Default | Description |
|---------|---------|-------------|
| `THERMAL_SHUTDOWN_TEMP` | 74.0°C | Temperature to trigger thermal protection |
| `THERMAL_RESTART_TEMP` | 65.0°C | Temperature to restore power after shutdown |
| `THERMAL_LOW_TEMP` | 50.0°C | Low temperature warning threshold (LED flash) |
| `THERMAL_SHUTDOWN_DELAY` | 1 | Loop cycles before shutdown triggers |

### Timing

| Setting | Default | Description |
|---------|---------|-------------|
| `LOW_THERMAL_CHECK_INTERVAL` | 30 | Loop cycles between low temp warnings |
| `SLEEP_LOOP_COUNT` | 100 | Loop cycles before entering sleep mode |

## Instructions
Follow schematics PDF from repo, download libraries from inside Arduino IDE.
See also: https://learn.sparkfun.com/tutorials/ir-control-kit-hookup-guide
