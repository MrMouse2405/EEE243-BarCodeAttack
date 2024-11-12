#include "Pololu3piPlus32U4.h"
#include "Lab4.h"
#include "LineFollowing.h"
#include "Parser.h"
#include "Scanner.h"

using namespace LineFollowing;
using namespace Pololu3piPlus32U4;
using namespace Parser;

OLED display;
ButtonB buttonB;

LineFollower driver;
KNNParser parser;

void playGo();

void playBeep();

void displayCentered(const String &s, uint8_t line);

void collectCalibrationBatch();

bool skipAScan(Scanner &scanner, LineFollower &driver);

void setup()
{
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

        Calibrating Robot

        |> Calibrate IR Sensors
    */

    displayCentered("Calibrating...", 4);
    driver.calibrate();
    while (driver.getState() == Calibrating)
    {
        driver.follow();
    }
    display.clear();
}

void loop()
{
    // Ask to start
    displayCentered("Ready", 1);
    displayCentered("<  GO  >", 4);
    buttonB.waitForButton();
    display.clear();

    displayCentered("Scanning", 4);
    playGo();

    /*

        |> Step 1: Collect data for supervised learning
        ... train the KNN Model

        Assumption: First One is ALWAYS '*'

     */

    collectCalibrationBatch();
    display.clear();

    /*

        |> Step 2: Parse BarCode

    */

    displayCentered("Scanning", 2);

    Lab4::Buffer<char, 20> resultBuffer;

    driver.start();
    while (driver.getState() != ReachedEnd && !resultBuffer.isFull())
    {
        /*

            Collecting a batch

         */

        Lab4::Buffer<Lab4::BarType, WIDTH_CHARACTER_SIZE> buffer;
        auto scanner = Scanner();

        // skip first scan (separator white space)
        while (scanner.scan().checkState() != Lab4::ResultState::Some)
        {
            if (driver.getState() == ReachedEnd)
            {
                // TODO: reached end while skipping a value ERORR!!!
                // TODO: get rid of this SHOW_RESULT, make the loops exit nicely
                // or else greg will be mad
                goto SHOW_RESULT;
            }
            driver.follow();
        }

        // collect our 9 values
        while (!buffer.isFull())
        {
            driver.follow();
            if (driver.getState() == ReachedEnd)
            {
                // TODO: reached end while reading a batch ERROR!!!
                // TODO: get rid of this SHOW_RESULT, make the loops exit nicely
                // or else greg will be mad
                goto SHOW_RESULT;
            }

            // check if we found a new value
            Lab4::Option<Lab4::Bar> scannedResult = scanner.scan();
            switch (scannedResult.checkState())
            {
            // we found a new value either Narrow or Wide
            case Lab4::ResultState::Some:
            {
                Lab4::BarType result = parser.getBarType(scannedResult.getPointer());
                buffer.add(&result);
                if (result == Lab4::BarType::Wide)
                {
                    // TODO: Play Music
                    // TODO: Too many wide bars error
                }
                break;
            }
            case Lab4::ResultState::None:
            {
                // we didn't get a new value yet
                break;
            }
            }
        }

        /*

            Decode our value

         */
        Lab4::Option<char> parsedResult = KNNParser::lex(buffer);
        switch (parsedResult.checkState())
        {
        case Lab4::ResultState::Some:
        {
            if (parsedResult.getValue() == '*')
            {
                // TODO: Implement Nice Exit, too many loops AAA
                // TODO: this sections sucks
                // TODO: this section works
                // TODO: 4 am coding is fun
                // TODO: Make this work  nicely pls
                driver.stop();
                goto SHOW_RESULT;
            }
            resultBuffer.add(parsedResult.getValue());
            break;
        }
        case Lab4::ResultState::None:
        {
            // TODO: Wrong Value Error
            break;
        }
        }
    }

    if (resultBuffer.isFull())
    {
        // TODO: we read too many values,
        // Realistically, we are never hitting the limit
        // but the condition still exists
    }

SHOW_RESULT:
    display.clear();
    displayCentered("Result:", 3);
    resultBuffer.add('\0');
    displayCentered(String(resultBuffer.buffer), 5);
    buttonB.waitForButton();
    display.clear();
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

bool skipAScan(Scanner &scanner, LineFollower &driver)
{
    driver.start();
    while (scanner.scan().checkState() != Lab4::ResultState::Some)
    {
        if (driver.getState() == ReachedEnd)
        {
            return false;
        }
        driver.follow();
    }

    return true;
}

/**
 *
 * Collects the first set of barcode values
 * and uses it for calibating the parser.
 *
 * Assumes the first set of barcode values correspond
 * to the character '*' in code39 character specification
 *
 * Step:
 *  |> Collect First Batch (Data Collection)
 *  |> Assign HardCoded Values (Data Labelling)   // i.e << * >> in code39
 *  |> Data is now ready for supervised learning.
 *
 */

void collectCalibrationBatch()
{

    // Step 1: Collect calibration data
    char starPatternLabel[WIDTH_CHARACTER_SIZE] = {'N', 'W', 'N', 'N', 'W', 'N', 'W', 'N', 'N'};
    Lab4::Buffer<Lab4::Bar, WIDTH_CHARACTER_SIZE> trainingBatch;

    auto scanner = Scanner();
    unsigned int count = 0;

    // skip first scan
    skipAScan(&scanner, &driver);

    // collect data
    while (!trainingBatch.isFull())
    {
        driver.follow();
        auto scannedResult = scanner.scan();
        switch (scannedResult.checkState())
        {
        // we found a new value
        case Lab4::ResultState::Some:
        {
            Lab4::Bar result = scannedResult.getValue();
            // Step 2: label the value
            result.type = static_cast<Lab4::BarType>(starPatternLabel[count]);
            trainingBatch.add(&result);
            count++;
            break;
        }
        // we didn't get a new value
        case Lab4::ResultState::None:
        {
            break;
        }
        }
    }

    // Step 3: Perform Supervised Learning
    parser.train(&trainingBatch);
}

void displayCentered(const String &s, const uint8_t line)
{
    // 10 is half of 21 (see function setup)
    display.gotoXY(10 - s.length() / 2, line);
    display.print(s.c_str());
}

void playGo()
{
    Buzzer::play("L16 cdegreg4");
    while (Buzzer::isPlaying())
    {
    }
}

void playBeep()
{
    Buzzer::play(">g32>>c32");
    while (Buzzer::isPlaying())
    {
    }
}
