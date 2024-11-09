#include "Lab4.h"
#include "LineFollowing.h"
#include "BarCodeReader.h"
#include "Pololu3piPlus32U4.h"

using namespace LineFollowing;
using namespace Pololu3piPlus32U4;

OLED display;
Buzzer buzzer;
ButtonB buttonB;

// line following algorithm
LineFollower driver;
// scanner with capacity of 20 characters
BarCodeReader scanner;

void playGo();

void playBeep();

void displayCentered(const String &s, const uint8_t line);

void displayResult(Code39Results result);

void displayError(BarCodeReaderStates state);

void setup() {
    // init
    display.setLayout21x8();

    // Welcome Screen
    displayCentered("Abdul Mannan Syed", 0);
    displayCentered("Nathan Gratton", 1);
    displayCentered("Lab 4: Barcode", 4);
    displayCentered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();

    // Now calibrate
    displayCentered("Calibrating...", 4);
    driver.pushAction(Calibrate);
    while (driver.getState() == Calibrating) {
        driver.follow();
    }
    display.clear();
}

void loop() {
    // Ask to start
    displayCentered("Ready", 1);
    displayCentered("<  GO  >", 4);
    buttonB.waitForButton();
    display.clear();

    displayCentered("Scanning", 4);
    playGo();

    // Start Following Line, and scanning barcodes
    driver.pushAction(LineFollowingActions::Start);
    scanner.pushAction(BarCodeReaderActions::Start);
    // BarCodeReaderStates last = scanner.getState();
    while (driver.getState() == Following) {
        driver.follow();
        scanner.read();

        // if (scanner.getState() != last) {
        //     last = scanner.getState();
        //     Motors::setSpeeds(0, 0);
        //     displayCentered(String(scanner.getState()), 5);
        //     buttonB.waitForButton();
        //     display.clear();
        //     Motors::setSpeeds(50, 50);
        // }
        if (scanner.inErrorState() || scanner.getState() == BarCodeReaderStates::Success) {
            driver.pushAction(Stop);
            break;
        }
    }
    // scanner.pushAction(Finish);
    display.clear();
    displayCentered("Stopped", 0);

    const auto scannedResult = scanner.getCode39Results();
    switch (scannedResult.checkState()) {
        case Lab4::ResultState::Some:
            displayCentered("Result:", 1);
            displayCentered(String(scannedResult.getValue().string), 2);
            displayCentered("Len:" + String(scannedResult.getValue().length), 3);
            break;
        case Lab4::ResultState::None: {
            display.clear();
            switch (scanner.getState()) {
                case InvalidBarCodeError: {
                    displayCentered("Bad Code", 0);
                    displayCentered("RS:" + String(scanner.get()) + '\0', 1);
                    displayCentered("RL" + String(scanner.code39ResultsLen), 2);
                    String s;
                    String s1;
                    String s2;

                    for (int i = 0; i < 3; i++) {
                        s += "," + String(static_cast<unsigned long int>(scanner.batch[i]));
                    }
                    for (int i = 3; i < 5; i++) {
                        s1 += "," + String(static_cast<unsigned long int>(scanner.batch[i]));
                    }
                    for (int i = 5; i < 9; i++) {
                        s2 += "," + String(static_cast<unsigned long int>(scanner.batch[i]));
                    }

                    displayCentered(s + '\0', 3);
                    displayCentered(s1 + '\0', 4);
                    displayCentered(s2 + '\0', 5);
                    break;
                }
                case InvalidBarCodeLenError: {
                    displayCentered("Invalid Code Len", 0);
                    displayCentered("RS:" + String(scanner.get()) + '\0', 1);
                    displayCentered("RL" + String(scanner.code39ResultsLen), 3);
                    break;
                }
                case BarCodeCalibrationError: {
                    displayCentered("Scanner Calibration", 0);
                    displayCentered("RS:" + String(scanner.get()) + '\0', 1);
                    displayCentered("Failed", 2);
                    displayCentered("BL:" + String(scanner.batchLen), 3);
                    displayCentered("RL" + String(scanner.code39ResultsLen), 4);
                    break;
                }
                case OutOfMemoryError: {
                    displayCentered("RS:" + String(scanner.get()) + '\0', 0);
                    displayCentered("Scanner Capa", 1);
                    displayCentered("-city Overflow", 2);
                    displayCentered("BL:" + String(scanner.batchLen), 3);
                    displayCentered("RL" + String(scanner.code39ResultsLen), 4);
                    break;
                }
                default:
                    displayCentered("Custom" + String(scanner.getState()), 2);
                    break;
            }
        }
    }

    buttonB.waitForButton();
}

void displayResult(Code39Results result) {
    display.clear();
    displayCentered("Scan Success", 0);
    displayCentered("Result:", 1);
    displayCentered(String(scanner.get()), 2);
    displayCentered(String(scanner.get()[0]), 3);
    displayCentered("Len:" + String(result.length), 5);
    displayCentered(String(scanner.getState()), 6);
}

void displayCentered(const String &s, const uint8_t line) {
    // 10 is half of 21 (see function setup)
    display.gotoXY(10 - (s.length() / 2), line);
    display.print(s.c_str());
}

void playGo() {
    buzzer.play("L16 cdegreg4");
    while (buzzer.isPlaying()) {
    }
}

void playBeep() {
    buzzer.play(">g32>>c32");
    while (buzzer.isPlaying()) {
    }
}
