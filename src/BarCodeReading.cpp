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
#include "BarCodeReading.h"
#include "Sensors.h"

using namespace BarCodeReading;

template<size_t ReaderCapacity>
void BarCodeReader<ReaderCapacity>::startTracking() {
    this->t0 = millis();
}

template<size_t ReaderCapacity>
void BarCodeReader<ReaderCapacity>::stepTracker() {
    uint64_t t1 = millis();
    uint64_t td = t1 - t0;
    this->batch[batchLen] = td;
    this->t0 = t1;
    
    if (this->batchLen + 1 == WIDTH_CHARACTER_SIZE) {
        this->state = BatchEnd;
    }
}
    
/**
 * Returns the current state of Barcode Sensor 
 * 
 * Check definition of BarCodeReaderStates to understand
 * the intent of each state
*/
template<size_t ReaderCapacity>
BarCodeReaderStates BarCodeReader<ReaderCapacity>::getState() {
    return this->state;
}
/**
 * This should be called in a loop while the robot
 * is scanning 
 * 
 */
template<size_t ReaderCapacity>
void BarCodeReader<ReaderCapacity>::read() {
    bool detectedBlack = Sensors::isBarcodeDetected();
    switch (this->state) {
        Initialized: {
            // do nothing
            break;
        }
        BatchStart: {
            // we should see a black
            if (detectedBlack) {
                this->startTracking();
                this->state = SeeingBlack;
            }
            break;
        }
        SeeingBlack: {
            if (!detectedBlack) {
                this->stepTracker();
                this->state = SeeingWhite;
            }
            break;
        }
        SeeingWhite: {
            if (detectedBlack) {
                this->stepTracker();
                this->state = SeeingWhite;
            }
            break;
        }
        BatchEnd: {
            // if first, calibrate it
            // parse the results
            // all the states
            break;
        }
        Success: {break;}
        default: {
            // error cases, ignore.
            break;
        }
    }
}
