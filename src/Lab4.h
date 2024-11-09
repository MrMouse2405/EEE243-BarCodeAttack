#pragma once

// Sensor Constants
#define NUM_SENSORS 5
#define NUM_SENSORS_START 1 // sensor 2
#define NUM_SENSORS_END 3   // sensor 4

// BarCode Sensors
#define BARCODE_SENSOR_RIGHT 4 // sensor 5
#define BARCODE_SENSOR_LEFT 0  // sensor 1

// time between each IR sample
#define LINE_SENSOR_TIMEOUT 2000

// Values below this will be ignored
#define NOISE_THRESHOLD 50

// Values below this will be considered to
// be not on black line.
#define LINE_THRESHOLD 200

// Maximum & Minimum speed the motors will be allowed to turn.
#define MAX_SPEED 200
#define MIN_SPEED 0

// Speed the motors will run when centered on the line.
#define BASE_SPEED MAX_SPEED

// Speed of motors while calibration
#define CALIBRATION_SPEED 50

// Size of the buffer for the barcode in percent.
#define SIZE_BUFFER 10

// Amount of Ws and Ns required to identify a character
#define WIDTH_CHARACTER_SIZE 9

// Amount of character barcode reader can store
#define BARCODE_READER_CAPACITY 20

/*
 * PID Constants
 *
 * This configuration uses a default proportional constant of 1/4
 * and a derivative constant of 1, which appears to perform well at low speeds.
 * Note: Adapted from Pololu3piplus documentation.
 */
#define PROPORTIONAL_CONSTANT 64 // coefficient of the P term * 256
#define DERIVATIVE_CONSTANT 256  // coefficient of the D term * 256

/*
 *
 * Buzzer Constants
 *
 */

namespace Lab4 {
    typedef enum OptionType {
        Some,
        None,
    } ResultState;

    template<typename T>
    class Option {
        T value;
        const ResultState exists;

    public:
        explicit Option(T value) : value(value), exists(Some) {
        }

        Option() : exists(None) {
        }

        ~Option() = default;

        T getValue() const {
            return value;
        }

        T *getPointer() const {
            return &value;
        }

        ResultState checkState() const {
            return exists;
        }
    };
}
