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
#include "BarCodeReader.h"

#include <Pololu3piPlus32U4Buttons.h>
#include <Pololu3piPlus32U4Motors.h>
#include <Pololu3piPlus32U4OLED.h>

#include "BarCodeParser.h"
#include "Sensors.h"

void BarCodeReader::startTracking() {
    this->t0 = millis();
    this->batchLen = 0;
}

void BarCodeReader::stepTracker() {
    const uint64_t t1 = millis();
    const uint64_t td = t1 - t0;
    this->batch[batchLen] = td;
    this->batchLen += 1;
    this->t0 = t1;

    if (this->batchLen == WIDTH_CHARACTER_SIZE) {
        this->state = BatchEnd;
    }
}

/**
 * Returns the current state of Barcode Sensor
 *
 * Check definition of BarCodeReaderStates to understand
 * the intent of each state
 */
BarCodeReaderStates BarCodeReader::getState() const {
    return this->state;
}

/**
 * This should be called in a loop while the robot
 * is scanning
 *
 */
void BarCodeReader::read() {
    const bool detectedBlack = Sensors::isBarcodeDetected();
    switch (this->state) {
        case Initialized:
        case Success:
        default: {
            // do nothing
            break;
        }
        // Scanning States
        case BatchStart: {
            // we should see a black
            if (detectedBlack) {
                this->startTracking();
                this->state = SeeingBlack;
            }
            break;
        }
        case SeeingBlack: {
            if (detectedBlack) {
                return;
            }
            this->state = SeeingWhite;
            this->stepTracker();
            break;
        }
        case SeeingWhite: {
            if (!detectedBlack) {
                return;
            }
            this->state = SeeingBlack;
            this->stepTracker();
            break;
        }
        case DiscardBlack: {
            if (!detectedBlack) { this->state = BatchStart; }
            break;
        }
        case DiscardWhite: {
            if (detectedBlack) { this->state = BatchStart; }
            break;
        }
        case BatchEnd: {
            // if first time, calibrate it
            if (this->code39ResultsLen == 0) {
                const bool errored = !BarCodeParser::widthCalibrate(this->batch);
                if (errored) {
                    this->state = BarCodeCalibrationError;
                    return;
                }
            }

            // check memory capacity
            if (this->code39ResultsLen + 2 == BARCODE_READER_CAPACITY) {
                this->state = OutOfMemoryError;
                return;
            }

            // parse the results
            char result = 0; {
                auto parseResult = BarCodeParser::processTimeArray(this->batch);
                switch (parseResult.checkState()) {
                    case Lab4::ResultState::None: {
                        this->state = InvalidBarCodeError;
                        return;
                    }
                    case Lab4::ResultState::Some: {
                        result = parseResult.getValue();
                        break;
                    }
                }
            }

            // store the result
            this->code39Results[this->code39ResultsLen] = result;
            this->code39ResultsLen += 1;


            this->state = BatchStart;

            // discard the next token
            // if (detectedBlack) {
            //     this->state = DiscardBlack;
            // } else {
            //     this->state = DiscardWhite;
            // }
            break;
        }
    }
}

bool BarCodeReader::inErrorState() const {
    switch (this->state) {
        case InvalidBarCodeError:
        case InvalidBarCodeLenError:
        case BarCodeCalibrationError:
        case OutOfMemoryError:
            return true;
        default:
            return false;
    }
}


Lab4::Option<Code39Results> BarCodeReader::getCode39Results() {
    if (this->inErrorState()) {
        return Lab4::Option<Code39Results>{};
    }

    this->code39Results[this->code39ResultsLen] = '\0';

    Code39Results result;
    result.length = this->code39ResultsLen;
    result.string = this->code39Results;
    return Lab4::Option<Code39Results>{result};
}

void BarCodeReader::pushAction(BarCodeReaderActions action) {
    switch (action) {
        case Start: {
            switch (this->state) {
                case Initialized: // start fresh
                {
                    this->state = BatchStart;
                    break;
                }
                case Success: // start a new scan
                {
                    this->state = BatchStart;
                    this->code39ResultsLen = 0;
                    break;
                }
                default: {
                    // ignore
                    break;
                }
            }
            break;
        }

        case Finish: {
            switch (this->state) {
                // finish before we can record a batch
                case BatchStart:
                case SeeingWhite:
                case SeeingBlack: {
                    this->state = InvalidBarCodeLenError;
                    break;
                }
                case Initialized:
                case BatchEnd: {
                    this->state = Success;
                    break;
                }
                default: {
                    // ignore
                    break;
                }
            }
            break;
        }
    }
}
