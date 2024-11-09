#ifndef CONFIG_H
#define CONFIG_H

// Global variables
extern int frequency;
extern int powerLevel;

// Pin definitions
#define LED_PIN PC13
#define ENCODER_A_PIN PA1
#define ENCODER_B_PIN PA2
#define BUTTON_CONFIRM_PIN PA3

// OLED settings
#define OLED_RESET -1
#define OLED_ADDR 0x3C

#endif
