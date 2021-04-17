/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
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
unsigned int loopToSleep = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

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

void wakeUp(){
  Serial.println("Interupt fired!");
  sleep_disable();
  detachInterrupt(1);
}

void GoingToSleep(){
  Serial.println("Going to sleep!");
  sleep_enable();
  attachInterrupt(1,wakeUp,LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  digitalWrite(13, HIGH);
  delay(1000);
  sleep_cpu();
  Serial.println("Just woke up!");
  
}

// the loop function runs over and over again forever
void loop() {
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
                  digitalWrite(12, 0);
                  powerStatus = 0;
                } else {
                   IrReceiver.stop();
                  digitalWrite(12, 1);
                  powerStatus = 1;
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
    delay(50);
    loopToSleep++;
    if(loopToSleep == 200){
      loopToSleep = 0;
      GoingToSleep();
    }
}
