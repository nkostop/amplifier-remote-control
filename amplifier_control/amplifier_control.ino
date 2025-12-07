/*
  Remote Control
  by nkostop

  Remote Control for audio using Front Panel Push/Led Switch,
  beefcake relay for switch on and off,
  IR remote Control and Thermistors for temperature measurements.

*/

#include <Arduino.h>
#include <avr/sleep.h>


/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols are active.
 * This must be done before the #include <IRremote.h>
 */
//#define DECODE_LG           1
//#define DECODE_NEC          1
// etc. see IRremote.h
//
//#define DISABLE_LED_FEEDBACK_FOR_RECEIVE // saves 108 bytes program space
#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 670 bytes program space if all other protocols are active
#define AMPLIFIER
//#define IR_MEASURE_TIMING

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules.
#define MARK_EXCESS_MICROS    20 // 20 is recommended for the cheap VS1838 modules

/*
 * First define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"

#include <IRremote.h>

// ============ Power State ============
bool powerStatus = false;
bool thermalProtectionActive = false;
bool acceptData = true;
int powerButtonState;

// ============ Thermal State ============
float T1, T2;                 // Current temperature readings
int thermalTimer = 0;         // Counter for shutdown delay
int lowThermalTimer = 0;      // Counter for low temp warning

// ============ Sleep State ============
unsigned int loopToSleep = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ALTERNATIVE_IR_FEEDBACK_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(POWER_PIN, INPUT_PULLUP);
  pinMode(POWER_LED_PIN, OUTPUT);
  powerStatus = false;

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

// In case the interrupt driver crashes on setup, give a clue
// to the user what's going on.
    Serial.println(F("Enabling IRin..."));

    /*
     * Start the receiver, enable feedback LED and (if not 3. parameter specified) take LED feedback pin from the internal boards definition
     */
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);


    // infos for receive
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
}

/*
 * wakeUp Function
 *
 * Interrupts fired when remote control is sending a signal
 * or front panel switch is pushed
 */
void wakeUp(){
  Serial.println("Interupt fired!");
}

/*
 * GoingToSleep Function
 *
 * Getting arduino (uno, mini, mini pro etc) to sleep
 * Attaching interrupts to call wakeUp when 
 * remote control is sending a signal
 * or front panel switch is pushed
 */
void GoingToSleep(){
  Serial.println("Going to sleep!");
  sleep_enable();
  attachInterrupt(0,wakeUp,FALLING);
  attachInterrupt(1,wakeUp,FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  digitalWrite(ALTERNATIVE_IR_FEEDBACK_LED_PIN, HIGH);
  delay(1000);
  sleep_cpu();
  sleep_disable();
  detachInterrupt(0);
  detachInterrupt(1);
  Serial.println("Just woke up!");
}

/*
 * PowerUp Function
 *
 * Power on the equipment
 */
void PowerUp(){
  Serial.println("Powering Up...");
  powerStatus = true;
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(POWER_LED_PIN, HIGH);
}

/*
 * PowerDown Function
 *
 * Power off the equipment
 */
void PowerDown(){
  Serial.println("Powering Down...");
  powerStatus = false;
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(POWER_LED_PIN, LOW);
}
/*
 * FrontPanelButton Function
 *
 * Managing the state of the button and the LED
 */
void FrontPowerButton() {
  //PowerButton
   powerButtonState = digitalRead(POWER_PIN);
   if(powerButtonState == 0){
    if (powerStatus) {
      PowerDown();
     } else {
      PowerUp();
     }
     delay(500);
   }

  //  LED
   if (powerStatus) {
    digitalWrite(POWER_LED_PIN, HIGH);
   } else {
    digitalWrite(POWER_LED_PIN, LOW);
   }
}

/*
 * IrReceiverHandle Function
 *
 * Power off the equipment
 */
 void IrReceiverHandle() {
   /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     *
     * E.g. command is in IrReceiver.decodedIRData.command
     * address is in command is in IrReceiver.decodedIRData.address
     * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
     */
    if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            IrReceiver.decodedIRData.flags = false; // yes we have recognized the flag :-)
            Serial.println(F("Overflow detected"));
        } else {
            // Print a short summary of received data
            IrReceiver.printIRResultShort(&Serial);
        }

        Serial.println();
        /*
         * !!!Important!!! Enable receiving of the next value,
         * since receiving has stopped after the end of the current received data packet.
         */
        IrReceiver.resume();

        /*
         * Finally check the received data and perform actions according to the received address and commands
         */
        if (IrReceiver.decodedIRData.address == IR_ADDRESS) {
          int command = IrReceiver.decodedIRData.command;
          switch (command) {
            case ACCEPT_DATA_CODE:
              acceptData = true;
              Serial.println("Accepting data from remote");
              // statements
              break;
            case DENY_DATA_CODE_1:
              // Deny all data
              // statements
              acceptData = false;
              Serial.println("Stop accepting data from remote");
              break;
            case DENY_DATA_CODE_2:
              // Deny all data
              // statements
              acceptData = false;
              Serial.println("Stop accepting data from remote");
              break;
            default:
              // statements
              break;
          }
          if(acceptData){
             switch (command) {
              Serial.println("Accepting Commands");
              Serial.println(command);
              case POWER_CODE:
                IrReceiver.stop();
                if(powerStatus){
                 PowerDown();
                } else {
                  PowerUp();
                }
                delay(500);
                IrReceiver.start();
                break;
              default:
                // statements
                break;
            }
          }
        }
    }
 }

 /*
 * TemperatureCheck Function
 *
 * Thermistor checking function
 */
void ThermalProtection(){
 if(T1 > THERMAL_SHUTDOWN_TEMP || T2 > THERMAL_SHUTDOWN_TEMP){
   // Start thermal timer and check thermal counter
   if(thermalTimer > THERMAL_SHUTDOWN_DELAY){
      Serial.println("Thermal Protection ON!!!");
      digitalWrite(RELAY_PIN, LOW);
      thermalProtectionActive = true;
   }
   thermalTimer++;
 } else {
   thermalTimer = 0;
 }
 if (thermalProtectionActive && powerStatus){
    digitalWrite(POWER_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(POWER_LED_PIN, LOW);
    delay(1000);
    if(T1 < THERMAL_RESTART_TEMP && T2 < THERMAL_RESTART_TEMP){
      Serial.println("Thermal Protection OFF :)");
      digitalWrite(RELAY_PIN, HIGH);
      thermalProtectionActive = false;
      thermalTimer = 0;
    }
 }
}

/*
 * LowTempChecker Function
 *
 * Thermistor checking function
 */
void LowTempChecker(){
 if(T1 < THERMAL_LOW_TEMP || T2 < THERMAL_LOW_TEMP){
   // Start thermal timer and check thermal counter
   if(lowThermalTimer < LOW_THERMAL_CHECK_INTERVAL){
      lowThermalTimer++;
   } else {
     Serial.println("Low Temperature...");
      lowThermalTimer = 0;
      // Flash LED 4 times to indicate low temperature
      for(int i = 0; i < 4; i++){
        digitalWrite(POWER_LED_PIN, LOW);
        delay(50);
        digitalWrite(POWER_LED_PIN, HIGH);
        delay(50);
      }
   }
 }
}

/*
 * ReadThermistorC Function
 *
 * Thermistor reading function
 */
float ReadThermistorC(int pin) {
  int adc = analogRead(pin);

  // guard against extreme / broken readings
  if (adc <= 0 || adc >= 1023) {
    return NAN;
  }

  float resistance = 0;

  if (THERMISTOR_WIRING_NUMBER == 0){
    resistance = THERMISTOR_SERIES_RESISTOR * (1023.0 / adc - 1.0);
  } else if (THERMISTOR_WIRING_NUMBER == 1) {
    // +5V -> R1 (10k) -> analog pin -> thermistor -> GND
    resistance = THERMISTOR_SERIES_RESISTOR * (float)adc / (1023.0 - (float)adc);
  } else {
    Serial.println("Wrong thermistor wiring chosen");
  }

  // --- Beta equation for EPCOS B57045K103K ---
  float steinhart = resistance / THERMISTOR_NOMINAL;    // R/R0
  steinhart = log(steinhart);                           // ln(R/R0)
  steinhart /= THERMISTOR_B_COEFFICIENT;                // 1/B * ln(R/R0)
  steinhart += 1.0 / THERMISTOR_T0_KELVIN;              // + 1/T0
  steinhart = 1.0 / steinhart;                          // invert -> Kelvin
  steinhart -= 273.15;                                  // Kelvin -> °C

  return steinhart;
}

/*
 * TemperatureCheck Function
 *
 * Thermistor checking function
 */
void TemperatureCheck(){
  T1 = ReadThermistorC(THERMISTOR1_PIN);
  T2 = ReadThermistorC(THERMISTOR2_PIN);

  Serial.print("Temperature 1: ");
  Serial.print(T1);
  Serial.println(" C");

  Serial.print("Temperature 2: ");
  Serial.print(T2);
  Serial.println(" C");

  LowTempChecker();
  ThermalProtection();
}


// the loop function runs over and over again forever
void loop() {
  FrontPowerButton();
  IrReceiverHandle();
  if (powerStatus){
    TemperatureCheck();
  }
  delay(50);
  // Looping some cycles before going to sleep
  loopToSleep++;
  if(loopToSleep == SLEEP_LOOP_COUNT){
    loopToSleep = 0;
    // Going to sleep only on power off status
    if (!powerStatus){
      GoingToSleep();
    }
  }
}
