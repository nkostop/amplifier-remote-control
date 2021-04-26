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
bool acceptData = false;
int powerStatus = 0;
int thermalProtection = 0;
unsigned int loopToSleep = 0;

//Variables for front side button
int PowerPin = 2;
int PowerLed = 8;
int powerButtonState;
int prevPowerButtonState = HIGH;

// Variables for thermistors
int Thermistor1Pin = 0;
int Thermistor2Pin = 1;
int Vo1;
int Vo2;
float R1 = 10000;
float logR2, R2, T1, T2;
float c1 = 1.306013916e-03, c2 = 2.136446243e-04, c3 = 1.035851727e-07;
float thermalShutdown = 70.00;
float thermalRestart = 65.00;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(PowerPin, INPUT_PULLUP);
  pinMode(PowerLed, OUTPUT);

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
  sleep_disable();
  detachInterrupt(0);
  detachInterrupt(1);
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
  attachInterrupt(0,PowerUp,LOW);
  attachInterrupt(1,wakeUp,LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  digitalWrite(13, HIGH);
  delay(1000);
  sleep_cpu();
  Serial.println("Just woke up!");
  
}

/*
 * PowerUp Function
 *
 * Power on the equipment
 */
void PowerUp(){
  Serial.println("Powering Up...");
  digitalWrite(12, 1);
  powerStatus = 1;
  delay(1000);
}

/*
 * PowerDown Function
 *
 * Power off the equipment
 */
void PowerDown(){
  Serial.println("Powering Down...");
   digitalWrite(12, 0);
   powerStatus = 0;
   thermalProtection = 0;
   delay(1000);
}

/*
 * FrontPanelButton Function
 *
 * Managing the state of the button and the LED
 */
void FrontPowerButton() {
  //PowerButton
   digitalWrite(PowerLed, LOW);
   powerButtonState = digitalRead(PowerPin);
   if(powerButtonState != prevPowerButtonState){
    if (powerStatus) {
      PowerDown();
     } else {
      PowerUp();
     }
   }

  //  LED
   if (powerStatus) {
    digitalWrite(PowerLed, HIGH);
   } else {
    digitalWrite(PowerLed, LOW);
   }  
}

/*
 * TemperatureCheck Function
 *
 * Thermistor checking function
 */
void ThermalProtection(){
 if(T1 > thermalShutdown || T2 > thermalShutdown){
   Serial.println("Thermal Protection ON!!!");
   digitalWrite(12, 0);
   thermalProtection = 1;
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
    }
 }
}

/*
 * TemperatureCheck Function
 *
 * Thermistor checking function
 */
void TemperatureCheck(){
  Vo1 = analogRead(Thermistor1Pin);
  Vo2 = analogRead(Thermistor2Pin);
  //Calculating Thermistor 1
  R2 = R1 * (1023.0 / (float)Vo1 - 1.0);
  logR2 = log(R2);
  T1 = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T1 = T1 - 273.15;

  Serial.print("Temperature 1: "); 
  Serial.print(T1);
  Serial.println(" C"); 

  //Calculating Thermistor 2
  R2 = R1 * (1023.0 / (float)Vo2 - 1.0);
  logR2 = log(R2);
  T2 = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T2 = T2 - 273.15;

  Serial.print("Temperature 2: "); 
  Serial.print(T2);
  Serial.println(" C"); 

  //RUN thermal protection routine
  ThermalProtection();
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
        if (IrReceiver.decodedIRData.address == 0) {
          int command = IrReceiver.decodedIRData.command;
          switch (command) {
            case ACCEPT_DATA_CODE:
              acceptData = true;
              // statements
              break;
            case DENY_DATA_CODE_1:
              // Deny all data
              // statements
              acceptData = false;
              break;
            case DENY_DATA_CODE_2:
              // Deny all data
              // statements
              acceptData = false;
              break;
            default:
              // statements
              break;
          }
          if(acceptData == true){
             switch (command) {
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
  if(loopToSleep == 10){
    loopToSleep = 0;
    // Going to sleep only on power off status
    if (powerStatus == 0){
      GoingToSleep();
    }
  }
}
