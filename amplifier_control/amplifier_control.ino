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

// variables for enabling relays
bool acceptData = true;
int powerStatus = 0;
unsigned int loopToSleep = 0;

// variables for raspberry connection
int RaspPowerPin = 10;

//Variables for front side button
int PowerPin = 2;
int RelayPin = 12;
int PowerLed = 8;
int powerButtonState;


// Variables for thermistors
const float R1 = 10000.0;          // series resistor (10 kΩ)
const float THERMISTOR_NOMINAL = 10000.0; // 10 kΩ at 25 °C
const float B_COEFFICIENT = 4300.0;       // from B57045K103K datasheet
const float T0_KELVIN = 25.0 + 273.15;    // 25 °C in Kelvin
int thermalProtection = 0;
int Thermistor1Pin = 0;
int Thermistor2Pin = 1;
float T1, T2;
float thermalShutdown = 74.00;
float thermalRestart = 65.00;
int thermalCounter = 1;
int thermalTimer = 0;
int lowThermalTimer = 0;
int lowThermalCounter = 30;
float lowThermal = 50;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(PowerPin, INPUT_PULLUP);
  pinMode(PowerLed, OUTPUT);
  powerStatus = 0;

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
  digitalWrite(13, HIGH);
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
  powerStatus = 1;
  digitalWrite(RelayPin, HIGH);
  digitalWrite(PowerLed, HIGH);
}

/*
 * PowerDown Function
 *
 * Power off the equipment
 */
void PowerDown(){
  Serial.println("Powering Down...");
  powerStatus = 0;
  digitalWrite(RelayPin, LOW);
  digitalWrite(PowerLed, LOW);
}
/*
 * FrontPanelButton Function
 *
 * Managing the state of the button and the LED
 */
void FrontPowerButton() {
  //PowerButton
   powerButtonState = digitalRead(PowerPin);
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
    digitalWrite(PowerLed, HIGH);
   } else {
    digitalWrite(PowerLed, LOW);
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
          if(acceptData == true){
             switch (command) {
              Serial.println("Accepting Commands");
              Serial.println(command);
              case POWER_CODE:
                IrReceiver.stop();
                if(powerStatus == 1){
                 PowerDown();
                } else {
                   IrReceiver.stop();
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
 if(T1 > thermalShutdown || T2 > thermalShutdown){
   // Start thermal timer and check thermal counter
   if(thermalTimer > thermalCounter){
      Serial.println("Thermal Protection ON!!!");
      digitalWrite(12, 0);
      thermalProtection = 1;
   }
   thermalTimer++;
 } else {
   thermalTimer = 0;
 }
 if (thermalProtection == 1 && powerStatus == 1){
    digitalWrite(PowerLed, HIGH);
    delay(1000);
    digitalWrite(PowerLed, LOW);
    delay(1000);
    if(T1 < thermalRestart && T2 < thermalRestart){
      Serial.println("Thermal Protection OFF :)");
      digitalWrite(12, 1);
      thermalProtection = 0;
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
 if(T1 < lowThermal || T2 < lowThermal){
   // Start thermal timer and check thermal counter
   if(lowThermalTimer < lowThermalCounter){
      lowThermalTimer++;
   } else {
     Serial.println("Low Temperature...");
      lowThermalTimer = 0;
      digitalWrite(PowerLed, 0);
      delay(50);
      digitalWrite(PowerLed, 1);
      delay(50);
      digitalWrite(PowerLed, 0);
      delay(50);
      digitalWrite(PowerLed, 1);
      delay(50);
      digitalWrite(PowerLed, 0);
      delay(50);
      digitalWrite(PowerLed, 1);
      delay(50);
      digitalWrite(PowerLed, 0);
      delay(50);
      digitalWrite(PowerLed, 1);
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
    resistance = R1 * (1023.0 / adc - 1.0);
  } else if (THERMISTOR_WIRING_NUMBER == 1) {
    // +5V -> R1 (10k) -> analog pin -> thermistor -> GND
    resistance = R1 * (float)adc / (1023.0 - (float)adc);
  } else {
    Serial.println("Wrong thermistor wiring chosen");
  }

  // --- Beta equation for EPCOS B57045K103K ---
  float steinhart = resistance / THERMISTOR_NOMINAL; // R/R0
  steinhart = log(steinhart);                        // ln(R/R0)
  steinhart /= B_COEFFICIENT;                        // 1/B * ln(R/R0)
  steinhart += 1.0 / T0_KELVIN;                      // + 1/T0
  steinhart = 1.0 / steinhart;                       // invert -> Kelvin
  steinhart -= 273.15;                               // Kelvin -> °C

  return steinhart;
}

/*
 * TemperatureCheck Function
 *
 * Thermistor checking function
 */
void TemperatureCheck(){
  T1 = ReadThermistorC(Thermistor1Pin);
  T2 = ReadThermistorC(Thermistor2Pin);

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
  if (powerStatus == 1){
    TemperatureCheck();
  }
  delay(50);
  // Looping some cycles before going to sleep
  loopToSleep++;
  if(loopToSleep == 100){
    loopToSleep = 0;
    // Going to sleep only on power off status
    if (powerStatus == 0){
      GoingToSleep();
    }
  }
}
