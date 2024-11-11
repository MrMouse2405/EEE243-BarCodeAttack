#pragma once
#include "Lab4.h"

/**
 * Scanner
 *
 * Responsible for bar code widths as scanned from
 * the IR sensors
 *
 * Date: 2024-11-11
 *
 */

class Scanner {
public:
    /**
     *
     * Scans the barcode width from the time of
     * initialization of this object
     *
     */
    Scanner();

    /**
     *
     * Scans the barcode width from the provided
     * startTime of initialization of this object
     *
     */
    explicit Scanner(uint64_t startTime);

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
    Lab4::Option<Lab4::Bar> scan();

private:
    typedef enum {
        // currently seeing white
        WHITE,
        // currently seeing black
        BLACK
    } ReadingState;

    // what are we currently seeing?
    ReadingState state;
    // since when did we start seeing our ReadingState
    uint64_t t0;
};
