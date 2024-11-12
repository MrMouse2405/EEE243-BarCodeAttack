#include "Scanner.h"
#include "Sensors.h"

/**
 * Scanner
 *
 * Responsible for bar code widths as scanned from
 * the IR sensors
 *
 * Date: 2024-11-11
 *
 */

/**
 *
 * Scans the barcode width from the time of
 * initialization of this object
 *
 */
Scanner::Scanner() {
    this->state = WHITE;
    this->t0 = millis();
}

/**
 *
 * Scans the barcode width from the provided
 * startTime of initialization of this object
 *
 */

Scanner::Scanner(const uint64_t startTime) {
    this->state = WHITE;
    this->t0 = startTime;
}

/**
 *
 * Scans and returns everytime a new value is detected.
 * returns Bar {
 *  time = how long is the width of that bar
 *  type = NULL
 *}
 * Option<Bar> will be empty if a new value is not detected.
 * It will contain a value if a new value is detected.
 *
 */
Lab4::Option<Lab4::Bar> Scanner::scan() {
    /*

        Basically everytime we are seeing a new color,
        i.e white if we were seeing black and vice versa,
        we track the time for how long we saw black or white
        and return it.

        if we are still seeing the same color, we return
        nothing.


     */
    const bool blackDetected = Sensors::isBarcodeDetected();
    switch (this->state) {
        case WHITE: {
            if (blackDetected) {
                this->state = BLACK;
                const uint64_t t1 = millis();
                const uint64_t delta = t1 - t0;
                this->t0 = t1;
                return Lab4::Option<Lab4::Bar>({delta, Lab4::BarType::Null});
            }
            break;
        }
        case BLACK: {
            if (!blackDetected) {
                this->state = WHITE;
                const uint64_t t1 = millis();
                const uint64_t delta = t1 - t0;
                this->t0 = t1;
                return Lab4::Option<Lab4::Bar>({delta, Lab4::BarType::Null});
            }
            break;
        }
    }

    return {};
}
