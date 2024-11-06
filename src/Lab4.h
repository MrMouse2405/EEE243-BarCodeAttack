#pragma once

#include "Pololu3piPlus32U4.h"

// Sensor Constants
#define NUM_SENSORS 5
#define NUM_SENSORS_START 1 // sensor 2
#define NUM_SENSORS_END 3   // sensor 4

// BarCode Sensors
#define BARCODE_SENSOR_RIGHT 4 // sensor 5
#define BARCODE_SENSOR_LEFT 0  // sensor 1

// time between each IR sample
#define LINE_SENSOR_TIMEOUT 2000

// Values below this will be ignored
#define NOISE_THRESHOLD 50

// Values below this will be considered to
// be not on black line.
#define LINE_THRESHOLD 200

// Maximum & Minimum speed the motors will be allowed to turn.
#define MAX_SPEED 200
#define MIN_SPEED 0

// Speed the motors will run when centered on the line.
#define BASE_SPEED MAX_SPEED

// Speed of motors while calibration
#define CALIBRATION_SPEED 150

// Size of the buffer for the barcode in percent.
#define SIZE_BUFFER 10

// Amount of Ws and Ns required to identify a character
#define WIDTH_CHARACTER_SIZE 9

/*
 * PID Constants
 *
 * This configuration uses a default proportional constant of 1/4
 * and a derivative constant of 1, which appears to perform well at low speeds.
 * Note: Adapted from Pololu3piplus documentation.
 */
#define PROPORTIONAL_CONSTANT 64 // coefficient of the P term * 256
#define DERIVATIVE_CONSTANT 256  // coefficient of the D term * 256

/*
 *
 * Buzzer Constants
 *
 */

namespace Lab4 {

    Pololu3piPlus32U4::OLED display;
    Pololu3piPlus32U4::Buzzer buzzer;
    Pololu3piPlus32U4::ButtonB buttonA;
    Pololu3piPlus32U4::ButtonB buttonB;
    Pololu3piPlus32U4::ButtonB buttonC;

    void setup() {
        display.setLayout21x8();
    }

    void playGo() {
        buzzer.play("L16 cdegreg4");
        while (buzzer.isPlaying()){}
    }

    void playBeep(){
        buzzer.play(">g32>>c32");
        while (buzzer.isPlaying());     
    }

    void display_centered(const String &s, const uint8_t line)
    {   
        // 10 is half of 21 (see function setup)
        display.gotoXY(10 - (s.length() / 2), line);
        display.print(s.c_str());
    }

    /*
     *
     *  Welcome Screen
     *
     */
    void welcomeScreen() {
        display_centered("Abdul Mannan Syed", 0);
        display_centered("Nathan Gratton", 1);
        display_centered("Lab 4: Barcode", 4);
        display_centered("To start, press B", 7);
        buttonB.waitForButton();
        display.clear();
    }

    typedef enum OptionType {
        Some,
        None,
    } ResultState;

    template<typename T>
    class Option {
        private:
            const T value;
            const ResultState exists;
        public:
            Option(T value) : value(value), exists(ResultState::Some) {};
            Option() : value(None), exists(ResultState::None) {};
            ~Option() {};
            T getValue() const {
                return value;
            }
            T* getPointer() const {
                return &value;
            }
            ResultState checkState() const {
                return exists;
            }
    };


}