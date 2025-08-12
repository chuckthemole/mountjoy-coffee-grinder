/*
   Timed contactor for coffee grinder.
 */

#define GRINDER_COIL 2 // Relay for grinder on/off.

#include <Arduino.h>
#include <EEPROM.h>
#include "LCDWrapper.h"

int EEPROM_address = 0; // Storage location for run/rest array.

// Tell LCD panel to use default pins
LcdWrapper lcd;

// Keypresses from LCD panel.
int lcd_key = 0;
int adc_key_in = 0;

#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

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
        return btnNONE;

    if (adc_key_in < 50)
        return btnRIGHT;
    if (adc_key_in < 250)
        return btnUP;
    if (adc_key_in < 450)
        return btnDOWN;
    if (adc_key_in < 650)
        return btnLEFT;
    if (adc_key_in < 850)
        return btnSELECT;

    return btnNONE;
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
        if (read_LCD_buttons() == btnLEFT || btnRIGHT)
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
    lcd.begin();
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

    case btnRIGHT:
    { //  push button "RIGHT" and show the word on the screen

        if (!isRunning)
            grinder_run();
        break;
    }
    case btnLEFT:
    {

        digitalWrite(GRINDER_COIL, LOW); // Stop grinder
        isRunning = false;
        break;
    }
    case btnUP:
    {

        while (read_LCD_buttons() == btnUP)
        {
        }; // Wait for button to be released to continue

        if (grindCycle < 1000)
            grindCycle++; // Increment cycle timer, provided not at limit.

        lcd.setCursor(0, 9); // Update displayed value.
        lcd.print(grindCycle);
        lcd.print(" s   ");

        break;
    }
    case btnDOWN:
    {

        while (read_LCD_buttons() == btnDOWN)
        {
        }; // Wait for button to be released to continue

        if (grindCycle > 0)
            grindCycle--; // Decrement cycle timer.

        lcd.setCursor(0, 9); // Update displayed value.
        lcd.print(grindCycle);
        lcd.print(" s   ");

        break;
    }
    case btnSELECT:
    {

        break;
    }
    case btnNONE:
    {

        break;
    }
    }
}
