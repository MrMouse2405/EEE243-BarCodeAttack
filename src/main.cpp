#include "Pololu3piPlus32U4.h"
#include "Lab4.h"
#include "LineFollowing.h"
#include "Parser.h"
#include "Scanner.h"
#include "code39.h"

using namespace LineFollowing;
using namespace Pololu3piPlus32U4;
using namespace Parser;

OLED display;
Buzzer buzzer;
ButtonB buttonB;


// line following algorithm
LineFollower driver;
KNNParser parser;


void playGo();
void playBeep();
void displayCentered(const String &s, const uint8_t line);


void setup() {
    // init
    display.setLayout21x8();

    /*
    
        Welcome Screen
    
    */
    displayCentered("Abdul Mannan Syed", 0);
    displayCentered("Nathan Gratton", 1);
    displayCentered("Lab 4: Barcode", 4);
    displayCentered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();

    /*
    
        Calibrate IR Sensors

    */
    displayCentered("Calibrating...", 4);
    driver.calibrate();
    while (driver.getState() == Calibrating) {
        driver.follow();
    }
    display.clear();

    /*
        
        Train KNN Parser Model
        
        |> Collect First Batch (Data Collection)
        |> Assign HardCoded Values (Data Labelling)
        |> Give it to Parser (Supervised Learning)
    
    */
    Lab4::Batch calibrationBatch;
    Scanner scanner = Scanner();
    
    driver.start();
    while(!calibrationBatch.isFull()) {
        driver.follow();
        auto scannedResult = scanner.scan();
        switch (scannedResult.checkState()) {
            // we found a new value
            case Lab4::ResultState::Some: {
                calibrationBatch.addBar(scannedResult.getPointer());
                break;
            }
            // we didnt get a new value
            case Lab4::ResultState::None: {

                break;
            }
        }
    }
    driver.stop();

    // Label the Values
    // i.e << * >> in code39
    char asterik_pattern[WIDTH_CHARACTER_SIZE] = {'N','W','N','N','W','N','W','N','N'};
    auto calibrationBatchData = calibrationBatch.getBars();
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        calibrationBatchData[i].type = Lab4::BarType(asterik_pattern[i]);
    }

    // parser is calibrated and created, ready for usage    
    parser = KNNParser(&calibrationBatch);

    // testing
    String s;
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        s = s + String((char)calibrationBatchData[i].type);
    }

    displayCentered(s,4);
}

void loop() {
    // Ask to start
    // displayCentered("Ready", 1);
    // displayCentered("<  GO  >", 4);
    // buttonB.waitForButton();
    // display.clear();

    // displayCentered("Scanning", 4);
    // playGo();

    // // Start Following Line, and scanning barcodes
    // driver.start();
    // // BarCodeReaderStates last = scanner.getState();
    // while (driver.getState() == Following) {
    //     driver.follow();
    // }
    // display.clear();
    // displayCentered("Stopped", 0);

    // buttonB.waitForButton();
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
