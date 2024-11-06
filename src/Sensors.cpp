/**
 * Sensors
 *
 * Responsible for calibrating and obtaining
 * values from the environment via the IR sensors.
 *
 * Author: OCdt Syed
 * Date: 2024-10-31
 *
 */

#include "Sensors.h"
#include "Pololu3piPlus32U4.h"

namespace Sensors {
    Pololu3piPlus32U4::LineSensors lineSensors;
    // values read from the sensors will be stored here
    static uint16_t lineSensorValues[NUM_SENSORS];
}

/*
 * Calibrates the sensors by reading values as the robot turns,
 * comparing these to previous readings, and setting the highest value
 * to 1000 and the lowest to zero.
 *
 * Takes no parameters and returns no values.
 */
void Sensors::calibrateSensors() {
    using namespace Pololu3piPlus32U4;

    ledRed(true);
    ledYellow(true);

    // Wait 1 second and then begin automatic sensor calibration
    // by rotating in place to sweep the sensors over the line
    delay(1000);

    // turn left
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++) {
        lineSensors.calibrate();
    }

    // turn all the way to the right
    Motors::setSpeeds(-(int16_t) CALIBRATION_SPEED, CALIBRATION_SPEED);
    for (int i = 0; i <= 80; i++) {
        lineSensors.calibrate();
    };

    // turn back to center
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++) {
        lineSensors.calibrate();
    }

    // stop
    Motors::setSpeeds(0, 0);
    ledRed(false);
    ledYellow(false);
}

/*
 * Determines if both Left and Right IR Sensors have detected
 * Barcode
 *
 * Returns bool
 *
 * bool == true if black color is detected
 * bool == false if white color detected
 */
bool Sensors::isBarcodeDetected() {
    return (lineSensorValues[BARCODE_SENSOR_LEFT] > LINE_THRESHOLD) && (
               lineSensorValues[BARCODE_SENSOR_RIGHT] > LINE_THRESHOLD);
}

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

Lab4::Option<int> Sensors::detectLines() {
    bool onLine = false;
    uint32_t avg = 0; // this is for the weighted total
    uint16_t sum = 0; // this is for the denominator, which is <= 64000
    static uint16_t lastPosition = 0;

    lineSensors.readCalibrated(lineSensorValues);

    for (uint8_t i = NUM_SENSORS_START; i <= NUM_SENSORS_END; i++) {
        const uint16_t value = lineSensorValues[i];

        // keep track of whether we see the line at all
        if (value > LINE_THRESHOLD) {
            onLine = true;
        }

        // only average in values that are above a noise threshold
        if (value > NOISE_THRESHOLD) {
            avg += static_cast<uint32_t>(value) * (i * 1000);
            sum += value;
        }
    }

    if (!onLine) {
        // if reached end (None of them see line)
        if (sum == 0) {
            return {};
        }

        // If it last read to the left of center, return 0.
        if (lastPosition < (NUM_SENSORS - 3) * 1000 / 2) {
            return Lab4::Option<int>{0};
        }
        // If it last read to the right of center, return the max.
        return Lab4::Option<int>{(NUM_SENSORS - 1) * 1000};
    }

    lastPosition = avg / sum;
    return Lab4::Option<int>{static_cast<int>(lastPosition)};
}
