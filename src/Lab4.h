#pragma once
#include <Arduino.h>

/*
 * Lab4.h
 *
 * Contains Several Hardcoded Magic Constants used by the robot.
 *
 * This file also defines essential types and structures for barcode processing
 * in the Lab4 namespace. It includes:
 *
 * - BarType: An enumeration for barcode classification (Narrow, Wide, Null).
 * - Bar: A structure representing a barcode with a timestamp and type.
 * - ResultState: An enumeration to represent the presence or absence of a value.
 * - Buffer: A templated class implementing a fixed-size buffer to store
 *   elements of any type.
 * - Option: A templated class for encapsulating an optional value, allowing
 *   for safe handling of potentially absent values.
 */

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
#define LINE_THRESHOLD 250

// Maximum & Minimum speed the motors will be allowed to turn.
#define MAX_SPEED 50
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
    // Enum representing the type of barcode: Narrow, Wide, or Null.
    typedef enum BT : char {
        Narrow = 'N',
        Wide = 'W',
        Null = 'X' // In this context means it's not processed
    } BarType;

    // Struct representing a barcode with a timestamp and its type.
    typedef struct {
        uint64_t time;        // Timestamp of the barcode
        mutable BarType type; // Type of the barcode (Narrow or Wide)
    } Bar;

    // Enum representing the state of an Option: Some (has value) or None (no value).
    typedef enum OptionType {
        Some,
        None,
    } ResultState;

    // Template class for a fixed-size buffer to hold elements of type T.
    template <typename T, size_t SIZE>
    class Buffer {
    public:
        int count = 0;       // Current number of elements in the buffer
        T buffer[SIZE]{};    // Array to hold the elements

        // Checks if the buffer is full.
        bool isFull() const {
            return count == SIZE;
        }

        // Adds a pointer to an element to the buffer if it's not full.
        void add(const T *val) {
            if (this->isFull()) { return; }
            memcpy(&buffer[this->count], val, sizeof(T));
            this->count += 1;
        }

        // Adds an element to the buffer if it's not full.
        void add(const T val) {
            if (this->isFull()) { return; }
            this->buffer[this->count] = val;
            this->count += 1;
        }
    };

    // Template class representing an optional value of type T.
    template<typename T>
    class Option {
        T value;                // The value, if it exists
        const ResultState exists; // State indicating if the value exists

    public:
        // Constructor for a present value.
        explicit Option(T value) : value(value), exists(Some) {
        }

        // Constructor for an absent value.
        Option() : exists(None) {
        }

        ~Option() = default;

        // Returns the value.
        T getValue() const {
            return value;
        }

        // Returns a pointer to the value.
        const T *getPointer() const {
            return &value;
        }

        // Checks if the value exists.
        ResultState checkState() const {
            return exists;
        }
    };
}
