/*
* The robot will play a note based on the darkness
* of the surface beneath it—producing higher notes for
* lighter colors and lower notes for darker ones.
* Surface brightness changes are detected every 30 ms.
*
* Authors: OCdt Syed and OCdt Libreiro
* Version: 1 October 2024
*/

#include <Pololu3piPlus32U4.h>

using namespace Pololu3piPlus32U4;

OLED display;
ButtonB button_b;
LineSensors line_sensors;

/*
* Threshold for values.
*
* The following values are determined by calculating
* the median of two adjacent brightness level readings
* obtained from the IR sensor in Part 1 of the lab.
*
*/
#define THRESHOLD_0 2090
#define THRESHOLD_1 1920
#define THRESHOLD_2 1682
#define THRESHOLD_3 1364
#define THRESHOLD_4 1090
#define THRESHOLD_5 924
#define THRESHOLD_6 812
#define THRESHOLD_7 714
#define THRESHOLD_8 606

/*
* Sets up the display to be in the proper formatting
*/

void setup() {
    display.setLayout21x8();
}

/*
* Displays a string to the OLED display.
*
* string: the string to be displayed
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
* line: which number line to be displayed on
*/

void display_centered(const String& s, const uint8_t line) {
    // 10 is half of 21 (see line 11)
    display.gotoXY(10 - (s.length()/2),line);
    display.print(s.c_str());
}

/*
* Determines which frequency of note to be played, depending on
* the value obtained from IR sensors.
*
* Value: value obtained from IR sensors.
*/

 void play_note(const uint16_t value) {

   unsigned char note = 0;

   if (value >= THRESHOLD_0) {
        note = 20;
     } else if (value >= THRESHOLD_1) {
         note = 40;
     } else if (value >= THRESHOLD_2) {
         note = 60;
     } else if (value >= THRESHOLD_3) {
         note = 80;
     } else if (value >= THRESHOLD_4) {
         note = 100;
     } else if (value >= THRESHOLD_5) {
         note = 120;
     } else if (value >= THRESHOLD_6) {
         note = 140;
     } else if (value >= THRESHOLD_7) {
         note = 160;
     } else if (value >= THRESHOLD_8) {
         note = 180;
     } else {
         note = 200;
     }

    Buzzer::playNote(note,30,10);
 }


void loop() {

    // Welcome Screen
    display_centered("Abdul Mannan Syed",0);
    display_centered("Emma Libreiro",1);
    display_centered("Lab 3-1: See",4);
    display_centered("To start, press B",7);
    button_b.waitForButton();
    display.clear();

    for(;;) {
        // read sensors
        static uint16_t readings[5];
        line_sensors.read(readings);
        // show reflectance
        display_line("Reflectance: " +String(readings[3]),4);
        // play note
        play_note(readings[3]);
        delay(30);

        // repeat.
        display.clear();
    }
}


