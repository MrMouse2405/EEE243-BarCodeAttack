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
#include "Lab4.h"

namespace BarCodeReading {

    typedef enum
    {
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
        // Parsed value does not exist in code39 charset
        InvalidBarCodeError,
        // Could not collect a complete batch
        // Reason 1: Could not get a full batch
        // Reason 2: Too many elements in a batch
        InvalidBarCodeLenError,
        // We have reached the max amount of character the reader can read
        OutOfMemoryError,
        // BarCode has been read succesfully
        Success
    } BarCodeReaderStates;

    typedef enum {
        // start reading
        Start,
        // finish reading
        Finish
    } BarCodeReaderActions;

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
    template<size_t ReaderCapacity>
    class BarCodeReader {
    private:
        BarCodeReaderStates state = Initialized;

        uint64_t t0;

        size_t batchLen = 0;
        uint64_t batch[WIDTH_CHARACTER_SIZE];


        size_t code39ResultsLen = 0;
        char code39Results[ReaderCapacity];

        void startTracking();
        void stepTracker();

    public:
        BarCodeReader();
        ~BarCodeReader();
        /**
         * Returns the current state of Barcode Sensor 
         * 
         * Check definition of BarCodeReaderStates to understand
         * the intent of each state
        */
        BarCodeReaderStates getState();
        /**
         * This should be called in a loop while the robot
         * is scanning 
         * 
         */
        void read();
    };
}