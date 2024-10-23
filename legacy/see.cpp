/*
* The robot reads values from the three
* central IR sensors and shows the raw data
* on the OLED display.
*
* Author: OCdt Syed and OCdt Libreiro
* Version: 1 October 2024
*/

#include <Pololu3piPlus32U4.h>
using namespace Pololu3piPlus32U4;

OLED display;
ButtonB button_b;
LineSensors line_sensors;

/*
* Sets up the display to be in the proper formatting
*/

void setup() {
    display.setLayout21x8();
}

/*
* Displays a string to the OLED display.
*
* String: the string to be displayed
* line: which number line to be displayed on
* (0 being the first, 7 being the last)
*/

void display_line(const String& s, const uint8_t line) {
    display.gotoXY(0,line);
    display.print(s.c_str());
}

/*
* Displays a string centered along the y-axis of the display.
*
* string: the string to be displayed
* line: which number line to be displayed on. (same as above function)
*/

void display_centered(const String& s, const uint8_t line) {
    // 10 is half of 21 (see line 11)
    display.gotoXY(10 - (s.length()/2),line);
    display.print(s.c_str());
}


void loop() {

     // Welcome Screen
     display_centered("Abdul Mannan Syed",0);
     display_centered("Emma Libreiro",1);
     display_centered("Lab 3-1: See",4);
     display_centered("To start, press B",7);
     button_b.waitForButton();
     display.clear();

    // scan and display values read by sensor.
    while (true) {
        // read sensors
        static uint16_t readings[5];
        line_sensors.read(readings);
        // present readings
        display_line("Reflectance: " +String(readings[3]),4);
        // repeat once requested
        button_b.waitForButton();
        display.clear();
    }
}