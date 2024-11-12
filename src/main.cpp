/**
 * EEE243 Lab 4: When BarCode Attacks
 * Professor Greg Philips
 *
 * This file controls the robot's operation.
 *
 * The robot navigates along the center line while reading
 * and parsing values from the IR sensor as it scans the
 * stripes on either side. An error will be displayed if
 * the stripes do not  conform to Code39 specifications.
 * The robot must display result of the stripes at the end
 * decoded according to the Code39 specification.
 *
 * @authors OCdt Syed, OCdt Gratton
 * @date 2024-11-11
 */

#include "Pololu3piPlus32U4.h"
#include "Lab4.h"
#include "LineFollowing.h"
#include "Parser.h"
#include "Scanner.h"

using namespace LineFollowing;
using namespace Pololu3piPlus32U4;
using namespace Parser;
using namespace Lab4;

OLED display;
ButtonB buttonB;

LineFollower driver;
KNNParser parser;


bool collectCalibrationBatch();

bool skipAScan(Scanner &scanner, LineFollower &driver);

void playNote(const String &sequence, bool yield = false);

void displayCentered(const String &message = "EMPTY", uint8_t line = 0);

void displayError(const String &message = "EMPTY");


void setup() {
    display.setLayout21x8();

    // Welcome screen
    displayCentered("Abdul Mannan Syed", 0);
    displayCentered("Nathan Gratton", 1);
    displayCentered("Lab 4: Barcode", 4);
    displayCentered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();

    // Calibrate Robot
    displayCentered("Calibrating...", 4);
    driver.calibrate();
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

    // Start Scanning Robot
    displayCentered("Scanning", 4);
    playNote(GO_SEQUENCE, true);
    driver.start();

    // Collect first batch
    if (!collectCalibrationBatch()) {
        displayError("Line Too Short");
        return;
    }

    // Parse Remaining Characters
    Buffer<char, 20> resultBuffer;
    while (resultBuffer.getLast() != CODE39_DELIMITER) {
        if (resultBuffer.isFull()) {
            displayError("Max Capacity Reached");
            return;
        }

        Buffer<BarType, WIDTH_CHARACTER_SIZE> buffer;
        Scanner scanner;
        uint8_t wideBarCount = 0;

        // skip first scan (separator white space)
        if (!skipAScan(scanner, driver)) {
            // we ran off-line before we could have started a batch
            displayError("Missing End Delimiter");
            return;
        }

        // collect our 9 values
        while (!buffer.isFull()) {
            driver.follow();
            if (driver.getState() == ReachedEnd) {
                displayError("Line Too Short");
                return;
            }

            Option<Bar> scannedResult = scanner.scan();
            // add new value we found to buffer
            if (scannedResult.checkState() == Some) {
                BarType result = parser.getBarType(scannedResult.getPointer());
                if (result == Wide) {
                    playNote(HIGH_SEQUENCE);
                    wideBarCount++;
                    if (wideBarCount == 4) {
                        displayError("Too many wide bars");
                        return;
                    }
                }
                buffer.add(&result);
            }
        }

        // decode our buffer
        const Option<char> parsedResult = KNNParser::lex(buffer);
        switch (parsedResult.checkState()) {
            case Some: {
                // we found a value!
                playNote(LOW_SEQUENCE);
                resultBuffer.add(parsedResult.getValue());
                break;
            }
            case None: {
                displayError("Invalid Value");
                return;
            }
        }
    }
    driver.stop();
    display.clear();
    playNote(BEEP_SEQUENCE, true);
    // Remove last delimiter, and make it a valid c-string
    resultBuffer.setLast('\0');

    // Display Result
    displayCentered("Result:", 0);
    if (resultBuffer.count == 1) {
        displayCentered("[EMPTY]", 1);
    } else {
        displayCentered(String(resultBuffer.buffer), 4);
    }
    buttonB.waitForButton();
    display.clear();
}

/**
 * Skips a scan value while controlling the driver.
 *
 * This function interacts with the scanner and driver to skip
 * the current scanning process. It may adjust the driver's behavior
 * accordingly.
 *
 * @param scanner Reference to the Scanner object.
 * @param driver Reference to the LineFollower object.
 * @return true if the scan was successfully skipped; false otherwise.
 */
bool skipAScan(Scanner &scanner, LineFollower &driver) {
    driver.start();
    while (scanner.scan().checkState() != Some) {
        if (driver.getState() == ReachedEnd) {
            driver.stop();
            return false;
        }
        driver.follow();
    }

    return true;
}

/**
 * Collects the first set of barcode values for parser calibration.
 *
 * This function assumes that the initial set of barcode values corresponds
 * to the character '*' in the Code39 character specification.
 *
 * Steps:
 *  1. Collect the first batch of barcode values (Data Collection).
 *  2. Assign hardcoded labels (Data Labelling) to each barcode value,
 *     associating them with the character '*'.
 *  3. Prepare the labeled data for supervised learning.
 *
 * @returns true if the operation succeeded; false otherwise.
 */
bool collectCalibrationBatch() {
    char starPatternLabel[WIDTH_CHARACTER_SIZE] = CODE39_DELIMITER_PATTERN;
    Lab4::Buffer<Bar, WIDTH_CHARACTER_SIZE> trainingBatch;
    auto scanner = Scanner();
    unsigned int count = 0;

    // Step 1: Collect calibration data
    // skip first scan
    if (!skipAScan(scanner, driver)) {
        return false; // Error
    }

    // collect data
    while (!trainingBatch.isFull()) {
        driver.follow();
        if (driver.getState() == ReachedEnd) {
            return false; // Error
        }

        Option<Bar> scannedResult = scanner.scan();
        switch (scannedResult.checkState()) {
            // we found a new value
            case Some: {
                Bar result = scannedResult.getValue();
                // Step 2: label the value
                const auto type = static_cast<BarType>(starPatternLabel[count]);
                result.type = type;
                if (type == Wide) {
                    playNote(HIGH_SEQUENCE);
                }
                trainingBatch.add(&result);
                count++;
                break;
            }
            // we didn't get a new value
            case None: {
                break;
            }
        }
    }

    // Step 3: Perform Supervised Learning
    parser.train(&trainingBatch);
    return true;
}

/**
 * Displays a string centered on the specified line of a display.
 *
 * This function takes a string and positions it in the center of a given line
 * on the display. The string is adjusted based on the display's width to ensure
 * that it is evenly spaced on both sides.
 *
 * @param message The string to be displayed.
 * @param line The line number on which the string should be centered (0-based).
 *             Ensure that the line number corresponds to a valid line on the display.
 */
void displayCentered(const String &message, const uint8_t line) {
    // 10 is half of 21 (see function setup)
    display.gotoXY(10 - message.length() / 2, line);
    display.print(message.c_str());
}

/**
 * Displays an error message on the screen.
 *
 * This function takes a string as an input and displays it as an error message.
 * It may format the message with a specific prefix or styling to indicate that
 * it is an error, ensuring that the user can easily identify it.
 *
 * @param message The error message to be displayed. It should provide clear information
 *          about the nature of the error.
 */
void displayError(const String &message) {
    display.clear();
    driver.stop();
    playNote(BEEP_SEQUENCE, true);
    displayCentered("[ ERROR ]", 0);
    displayCentered(message, 1);
    buttonB.waitForButton();
    display.clear();
}

/**
 * Plays a specified musical note sequence.
 *
 * This function takes a string representing a note sequence and
 * plays it using the buzzer. It blocks execution until the note
 * sequence has finished playing.
 *
 * @param sequence The note sequence to be played.
 * @param yield The program will be yielded until buzzer has
 *              finished playing if set to true
 */
void playNote(const String &sequence, const bool yield) {
    Buzzer::stopPlaying(); // stop all previous
    Buzzer::play(sequence.c_str());
    if (yield) {
        while (Buzzer::isPlaying()) {
        }
    }
}
