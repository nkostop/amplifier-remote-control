/*
 *  PinDefinitionsAndMore.h
 *
 *  Contains pin definitions for IRremote examples for various platforms
 */
 


#if defined(AMPLIFIER)
#define IR_RECEIVE_PIN      3 // To be compatible with interrupt example, pin 2 is chosen here.
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 13 // E.g. used for examples which use LED_BUILDIN for example output.
#define ACCEPT_DATA_CODE 0x44
#define DENY_DATA_CODE_1 0x40
#define DENY_DATA_CODE_2 0x43
#define POWER_CODE 0x46
#endif

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
