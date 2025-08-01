/*
   Timed contactor for coffee grinder.
 */

#define GRINDER_COIL 2 // Relay for grinder on/off.

#include <Arduino.h>
// #include <LiquidCrystal.h>
#include <EEPROM.h>
#include "CommonArduinoHelper.h"

int EEPROM_address = 0; // Storage location for run/rest array.

// Tell LCD panel which pins it can use.
// LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Keypresses from LCD panel.
int lcd_key = 0;
int adc_key_in = 0;

// #define Button::btnRIGHT 0
// #define Button::btnUP 1
// #define Button::btnDOWN 2
// #define Button::btnLEFT 3
// #define Button::btnSELECT 4
// #define Button::btnNONE 5

// Run time, in seconds, for grinder relay.
unsigned int grindCycle = {360};

unsigned long startTime = 0;
unsigned long elapsedTime = 0;

bool isRunning;

#define LONG_PRESS 100 // The number of miliseconds a button must be held
                       // to be considered a long press.

bool runMode = true; // Automatic mode when true.

int read_LCD_buttons()
{ // Check all buttons for a press.

    adc_key_in = analogRead(0); // Buttons are defined by analog value ranges.

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

/*
 * Turns carbonator solenoid on and off for indicated run time and rest time.
 */

void grinder_run()
{

    lcd.setCursor(0, 0);
    lcd.print("RUN...  ");
    // lcd.setCursor(0,9);
    // lcd.print(grindCycle);
    // lcd.print(" s");

    startTime = millis();

    EEPROM.put(EEPROM_address, grindCycle); // Save current timer setting to permanent memory if
                                            // different from previous value.

    digitalWrite(GRINDER_COIL, HIGH);

    while (millis() < (startTime + (grindCycle * 1000)))
    {

        // Leave grinder on for duration of cycle, or until canceled

        // Abort if left button pressed.
        if (read_LCD_buttons() == Button::btnLEFT || Button::btnRIGHT)
        {
            lcd.setCursor(0, 0);
            lcd.print("IDLE    ");
            digitalWrite(GRINDER_COIL, LOW);
            isRunning = false;
            exit;
        }
        // Else, update display

        lcd.setCursor(9, 1);                      // move cursor to second line "1" and 9 spaces over
        lcd.print((millis() - startTime) / 1000); // display seconds elapsed since start.
        lcd.print(" s");
    }

    lcd.setCursor(0, 0);
    lcd.print("Ready   ");
    digitalWrite(GRINDER_COIL, LOW);
}

void setup()
{

    Serial.begin(9600);

    pinMode(GRINDER_COIL, OUTPUT);

    // EEPROM.get(EEPROM_address, grindCycle);

    // Display Ready message.
    // lcd.begin(16, 2);
    LCD::lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Ready");
    lcd.setCursor(0, 9);
    lcd.print(grindCycle);
    lcd.print(" s");
}

void loop()
{

    lcd_key = read_LCD_buttons(); // read the buttons

    lcd.setCursor(0, 1);

    switch (lcd_key)
    { // depending on which button was pushed, we perform an action

    case Button::btnRIGHT:
    { //  push button "RIGHT" and show the word on the screen

        if (!isRunning)
            grinder_run();
        break;
    }
    case Button::btnLEFT:
    {

        digitalWrite(GRINDER_COIL, LOW); // Stop grinder
        isRunning = false;
        break;
    }
    case Button::btnUP:
    {

        while (read_LCD_buttons() == Button::btnUP)
        {
        }; // Wait for button to be released to continue

        if (grindCycle < 1000)
            grindCycle++; // Increment cycle timer, provided not at limit.

        lcd.setCursor(0, 9); // Update displayed value.
        lcd.print(grindCycle);
        lcd.print(" s   ");

        break;
    }
    case Button::btnDOWN:
    {

        while (read_LCD_buttons() == Button::btnDOWN)
        {
        }; // Wait for button to be released to continue

        if (grindCycle > 0)
            grindCycle--; // Decrement cycle timer.

        lcd.setCursor(0, 9); // Update displayed value.
        lcd.print(grindCycle);
        lcd.print(" s   ");

        break;
    }
    case Button::btnSELECT:
    {

        break;
    }
    case Button::btnNONE:
    {

        break;
    }
    }
}
