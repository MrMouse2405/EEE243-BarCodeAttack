#pragma once
#include "Lab4.h"

/**
 * Sensors
 *
 * Responsible for calibrating and obtaining
 * values from the environment via the IR sensors.
 *
 * Date: 2024-10-31
 *
 */


namespace Sensors {
    /*
     * Calibrates the sensors by reading values as the robot turns,
     * comparing these to previous readings, and setting the highest value
     * to 1000 and the lowest to zero.
     *
     * Takes no parameters and returns no values.
     */

    void calibrateSensors();

    /*
     * Assesses whether the robot's sensors detect the line
     * and calculates the weighted average of the values obtained
     * from the line sensors.
     *
     * Returns Option<int16_t>.
     *
     * If robot's sensors detect the line:
     *   Option<int16_t> will have a value (weighted avg of 3 central IR sensors).
     *
     * If robot's sensors do not detect the line:
     *   Option<int16_t> will be empty.
     */

    Lab4::Option<int> detectLines();

    /*
     * Determines if both Left and Right IR Sensors have detected
     * Barcode
     *
     * Returns bool
     *
     * bool == true if black color is detected
     * bool == false if white color detected
     */
    bool isBarcodeDetected();
}
