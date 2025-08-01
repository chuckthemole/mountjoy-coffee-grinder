/*
  GrinderController.ino
  ----------------------
  Timed relay control for coffee grinder using LCD + button interface.
  Stores settings in EEPROM.

  Dependencies:
  - CommonArduinoHelper.h (includes LCD and Button enum helpers)
*/

#include <Arduino.h>
#include <EEPROM.h>
#include "CommonArduinoHelper.h"

// ==================== CONFIG ==================== //
#define GRINDER_COIL 2   // Digital pin controlling grinder relay
#define EEPROM_ADDRESS 0 // EEPROM location for saved grind time
#define LONG_PRESS 100   // Long-press threshold (ms)

// ==================== STATE ==================== //
unsigned int grindCycle = 360; // Grinder ON duration in seconds
unsigned long startTime = 0;   // Millis at start of grind cycle
bool isRunning = false;        // Grinder active state

// ==================== FUNCTIONS ==================== //

/**
 * Reads the analog button input and returns the button enum.
 * Based on the key voltage ranges from common LCD keypads.
 */
int read_LCD_buttons()
{
    int adc_key_in = analogRead(A0); // Read analog input

    if (adc_key_in > 1000)
        return Button::btnNONE;
    if (adc_key_in < 50)
        return Button::btnRIGHT;
    if (adc_key_in < 250)
        return Button::btnUP;
    if (adc_key_in < 450)
        return Button::btnDOWN;
    if (adc_key_in < 650)
        return Button::btnLEFT;
    if (adc_key_in < 850)
        return Button::btnSELECT;

    return Button::btnNONE;
}

/**
 * Runs the grinder for a set duration unless interrupted.
 */
void grinder_run()
{
    Serial.println("Starting grinder cycle...");
    isRunning = true;

    // Update LCD
    LCD::lcd.setCursor(0, 0);
    LCD::lcd.print("RUN...       ");

    // Save grind time to EEPROM
    EEPROM.put(EEPROM_ADDRESS, grindCycle);

    // Start the grinder
    digitalWrite(GRINDER_COIL, HIGH);
    startTime = millis();

    while (millis() - startTime < grindCycle * 1000UL)
    {
        int btn = read_LCD_buttons();

        // Check for abort (LEFT or RIGHT button)
        if (btn == Button::btnLEFT || btn == Button::btnRIGHT)
        {
            Serial.println("Cycle aborted by user.");
            LCD::lcd.setCursor(0, 0);
            LCD::lcd.print("ABORTED      ");
            digitalWrite(GRINDER_COIL, LOW);
            isRunning = false;
            delay(500); // Allow time for LCD update
            return;
        }

        // Show elapsed time
        LCD::lcd.setCursor(9, 1);
        LCD::lcd.print((millis() - startTime) / 1000);
        LCD::lcd.print(" s  ");
    }

    // Finish cycle
    Serial.println("Grinder cycle complete.");
    LCD::lcd.setCursor(0, 0);
    LCD::lcd.print("Ready         ");
    digitalWrite(GRINDER_COIL, LOW);
    isRunning = false;
}

/**
 * Initializes display, I/O pins, and EEPROM state.
 */
void setup()
{
    Serial.begin(9600);
    Serial.println("Grinder controller booting...");

    pinMode(GRINDER_COIL, OUTPUT);
    digitalWrite(GRINDER_COIL, LOW);

    // Setup LCD
    LCD::lcd.begin(16, 2);
    LCD::lcd.setCursor(0, 0);
    LCD::lcd.print("Ready");

    // Load previous grindCycle from EEPROM
    EEPROM.get(EEPROM_ADDRESS, grindCycle);
    if (grindCycle == 0xFFFF || grindCycle == 0)
    {
        grindCycle = 360; // Use default if EEPROM is uninitialized
    }

    // Show default time
    LCD::lcd.setCursor(0, 9);
    LCD::lcd.print(grindCycle);
    LCD::lcd.print(" s");

    Serial.print("Initial grind time: ");
    Serial.print(grindCycle);
    Serial.println(" seconds");
}

/**
 * Main loop to handle user interaction.
 */
void loop()
{
    int lcd_key = read_LCD_buttons(); // Read current button press

    LCD::lcd.setCursor(0, 1); // Move to second row

    switch (lcd_key)
    {
    case Button::btnRIGHT:
        if (!isRunning)
            grinder_run();
        break;

    case Button::btnLEFT:
        Serial.println("Manual STOP triggered.");
        digitalWrite(GRINDER_COIL, LOW);
        isRunning = false;
        break;

    case Button::btnUP:
        while (read_LCD_buttons() == Button::btnUP)
            ; // Wait for release

        if (grindCycle < 1000)
        {
            grindCycle++;
            Serial.print("Grind time increased to: ");
            Serial.println(grindCycle);
        }

        LCD::lcd.setCursor(0, 9);
        LCD::lcd.print(grindCycle);
        LCD::lcd.print(" s  ");
        break;

    case Button::btnDOWN:
        while (read_LCD_buttons() == Button::btnDOWN)
            ; // Wait for release

        if (grindCycle > 1)
        {
            grindCycle--;
            Serial.print("Grind time decreased to: ");
            Serial.println(grindCycle);
        }

        LCD::lcd.setCursor(0, 9);
        LCD::lcd.print(grindCycle);
        LCD::lcd.print(" s  ");
        break;

    case Button::btnSELECT:
        // Reserved for future use
        break;

    case Button::btnNONE:
    default:
        // Do nothing
        break;
    }

    delay(100); // Polling delay to avoid bouncing
}
