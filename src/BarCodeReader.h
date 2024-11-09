/**
 * BarCodeReader
 *
 * Responsible for taking raw values from the IR
 * Sensors in batches and parsing them into
 * code39 character set.
 *
 *
 * Author: OCdt Syed
 * Date: 2024-11-01
 *
 */

#pragma once
#include <Arduino.h>
#include "Lab4.h"


typedef enum {
    // Sensor has just started, awaiting input
    Initialized,
    // Start of Batch of 10 values
    BatchStart,
    // Calculating Width for White Token
    SeeingWhite,
    // Calculating Width for Black Token
    SeeingBlack,
    // End of Batch
    BatchEnd,
    // Spacer
    DiscardWhite,
    DiscardBlack,
    // Errored by while calibration
    BarCodeCalibrationError,
    // Parsed value does not exist in code39 charset
    InvalidBarCodeError,
    // Could not collect a complete batch
    // Reason 1: Could not get a full batch
    // Reason 2: Too many elements in a batch
    InvalidBarCodeLenError,
    // We have reached the max amount of character the reader can read
    OutOfMemoryError,
    // BarCode has been read successfully
    Success
} BarCodeReaderStates;

typedef enum {
    // start reading
    Start,
    // finish reading
    Finish
} BarCodeReaderActions;

typedef struct C39Results {
    size_t length{};
    char *string{};
} Code39Results;

/**
 *
 * class BarCodeReader
 *
 * Responsible for reading values while
 * robot is scanning over the barcode
 *
 * ReaderCapacity is the maximum
 * number of code39 character it can read.
 *
 */
class BarCodeReader {
    BarCodeReaderStates state = Initialized;

    uint64_t t0{};

    void startTracking();

    void stepTracker();

public:
    size_t batchLen = 0;
    size_t code39ResultsLen = 0;
    uint64_t batch[WIDTH_CHARACTER_SIZE]{};
    char *barWidths{};
    char code39Results[BARCODE_READER_CAPACITY]{};

    /**
     * Returns the current state of Barcode Sensor
     *
     * Check definition of BarCodeReaderStates to understand
     * the intent of each state
     */
    BarCodeReaderStates getState() const;

    /**
     * Returns true if an error has occurred
     *
     * Error type can be received from method getState();
     */
    bool inErrorState() const;

    /**
     * This should be called in a loop while the robot
     * is scanning
     *
     */
    void read();

    /**
     * Returns the parsed results
     *
     * Will be empty if an error has occurred.
     */
    Lab4::Option<Code39Results> getCode39Results();
    char * get() {
        return this->code39Results;
    }

    /**
     *
     *  Push user defined actions
     *
     *  i.e. give commands to start / stop
     *  the algorithm
     *
     */
    void pushAction(BarCodeReaderActions action);
};
