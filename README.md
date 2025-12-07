# Simple amplifier control

Remote control for audio amplifier using front panel push/LED switch, relay for power control, IR remote, and thermistors for temperature monitoring.

## Architecture

```
                                    +------------------+
                                    |   IR Remote      |
                                    +--------+---------+
                                             |
                                             | IR Signal
                                             v
+-------------------+              +-------------------+              +-------------------+
|  Front Panel      |              |  Arduino Mini Pro |              |  Amplifier        |
|  Button + LED     |<------------>|  5V               |------------->|  (via Relay)      |
+-------------------+   Pin 2,8    +-------------------+    Pin 12    +-------------------+
                                             ^
                                             | Pin 3
                                             |
                                   +-------------------+
                                   |  IR Receiver      |
                                   |  TSOP38238        |
                                   +-------------------+

                                   +-------------------+
                                   |  Thermistor 1     |-----> A0
                                   +-------------------+

                                   +-------------------+
                                   |  Thermistor 2     |-----> A1
                                   +-------------------+
```

### System Flow

```
                    +-------------+
                    |    IDLE     |
                    | (Sleep Mode)|
                    +------+------+
                           |
            IR Signal or Button Press
                           |
                           v
                    +------+------+
                    |  POWER ON   |
                    +------+------+
                           |
                           v
         +---------->+-----+------+<----------+
         |           | MONITORING |           |
         |           +-----+------+           |
         |                 |                  |
    Temp < 65°C      Temp > 74°C         Normal
         |                 |             Operation
         |                 v                  |
         |           +-----+------+           |
         +-----------|  THERMAL   |-----------+
                     | PROTECTION |
                     +------------+
                     (Relay OFF, LED blinks)
```

## Hardware
- Arduino Nano (tested) or Arduino Mini Pro 5V (https://www.sparkfun.com/products/11113)
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

### Thermistor Wiring

The thermistors form a voltage divider with a fixed series resistor. Two wiring modes are supported:

**Mode 0** - Thermistor on high side:
```
+5V ---- [Thermistor] ----+---- [10kΩ Resistor] ---- GND
                          |
                       Analog Pin (A0/A1)
```

**Mode 1 (default, tested)** - Thermistor on low side:
```
+5V ---- [10kΩ Resistor] ----+---- [Thermistor] ---- GND
                             |
                          Analog Pin (A0/A1)
```

Set `THERMISTOR_WIRING_NUMBER` to match your circuit (0 or 1).

### Thermistor Calibration

| Setting | Default | Description |
|---------|---------|-------------|
| `THERMISTOR_WIRING_NUMBER` | 1 | Wiring mode (0 or 1, see diagrams above) |
| `THERMISTOR_SERIES_RESISTOR` | 10000.0 | Series resistor value in ohms (10kΩ) |
| `THERMISTOR_NOMINAL` | 10000.0 | Thermistor resistance at 25°C (10kΩ NTC) |
| `THERMISTOR_B_COEFFICIENT` | 4300.0 | Beta coefficient from datasheet |
| `THERMISTOR_T0_KELVIN` | 298.15 | Reference temperature in Kelvin (25°C) |

**Note:** If using a different thermistor, update `THERMISTOR_NOMINAL` and `THERMISTOR_B_COEFFICIENT` from its datasheet. The B coefficient is critical for accurate temperature readings.

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

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/nkostop/amplifier-remote-control.git
```

### 2. Install Arduino IDE

Download and install version **2.3.6** from https://www.arduino.cc/en/software

### 3. Install required libraries

Open Arduino IDE, go to **Sketch > Include Library > Manage Libraries** and install:
- `IRremote` by shirriff (version 4.x)

### 4. Configure the project

1. Open `amplifier_control/amplifier_control.ino` in Arduino IDE
2. Edit `PinDefinitionsAndMore.h` to match your hardware setup (pin assignments, thermistor values, thermal thresholds)

### 5. Connect hardware

Follow the schematic in `RemoteControl.pdf` for wiring details.

**Quick pin reference:**
| Component | Arduino Pin |
|-----------|-------------|
| IR Receiver (TSOP38238) | D3 |
| Front Panel Button | D2 |
| Front Panel LED | D8 |
| Relay | D12 |
| Thermistor 1 | A0 |
| Thermistor 2 | A1 |

### 6. Upload firmware

1. Select **Tools > Board > Arduino Nano**
2. Select **Tools > Processor > ATmega328P (Old Bootloader)**
3. Select the correct **Tools > Port**
4. Click **Upload**

### 7. Verify operation

Open **Tools > Serial Monitor** (115200 baud) to see debug output and temperature readings.

## Resources

- [IR Control Kit Hookup Guide](https://learn.sparkfun.com/tutorials/ir-control-kit-hookup-guide)
- [Schematic](RemoteControl.pdf)
