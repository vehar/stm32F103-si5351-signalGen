#ifndef CONFIG_H
#define CONFIG_H

// Global variables
extern int frequency;
extern int powerLevel;

// Pin definitions
#define LED_PIN PC13
#define ENCODER_A_PIN PB_0
#define ENCODER_B_PIN PB_1
#define ENCODER_PUSH_PIN PA7

#define UP_BUTTON_PIN PA4
#define DOWN_BUTTON_PIN PA5

// OLED settings
constexpr int OLED_RESET = -1;
#define OLED_ADDR 0x3C

#endif
