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

| Setting | Default | Description |
|---------|---------|-------------|
| `THERMAL_SHUTDOWN_TEMP` | 74°C | Temperature to trigger thermal protection |
| `THERMAL_RESTART_TEMP` | 65°C | Temperature to restore power after shutdown |
| `THERMAL_LOW_TEMP` | 50°C | Low temperature warning threshold |
| `SLEEP_LOOP_COUNT` | 100 | Loop cycles before sleep mode |

Pin assignments and thermistor calibration values can also be adjusted in the header file.

## Instructions
Follow schematics PDF from repo, download libraries from inside Arduino IDE.
See also: https://learn.sparkfun.com/tutorials/ir-control-kit-hookup-guide
