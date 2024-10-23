/*
 * The robot will follow black lines on the floor using its
 * sensors while displaying "GO!!!". Once it reaches the end,
 * it will turn around and follow the track again.
 *
 * Author: OCdt Syed and OCdt Gratton
 * Version: 1 October 2024
 */

#include <Pololu3piPlus32U4.h>

using namespace Pololu3piPlus32U4;

OLED display;
Buzzer buzzer;
LineSensors lineSensors;
Motors motors;
ButtonA buttonA;
ButtonB buttonB;
ButtonC buttonC;

/*
 * Magic Numbers
 */

// Sensor Constants
#define NUM_SENSORS 5
#define NUM_SENSORS_START 1 // sensor 2
#define NUM_SENSORS_END 3   // sensor 4

// Encoder Sensors
#define ENCODER_SENSOR_RIGHT 4 // sensor 5
#define ENCODER_SENSOR_LEFT 0  // sensor 1

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
#define CALIBRATION_SPEED 150

// define our cm unit
typedef uint64_t MilliSeconds;

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

#define PLAY_GO(buzzer)          \
    buzzer.play("L16 cdegreg4"); \
    while (buzzer.isPlaying())   \
        ;
#define PLAY_BEEP(buzzer)      \
    buzzer.play(">g32>>c32");  \
    while (buzzer.isPlaying()) \
        ;

/*
 * Displays a string centered along the y-axis of the display.
 *
 * string: the string to be displayed
 * line: which number line to be displayed on. (0 <= line < 8)
 */

void display_centered(const String &s, const uint8_t line)
{
    // 10 is half of 21 (see function setup)
    display.gotoXY(10 - (s.length() / 2), line);
    display.print(s.c_str());
}

/*
 * Calibrates the sensors by reading values as the robot turns,
 * comparing these to previous readings, and setting the highest value
 * to 1000 and the lowest to zero.
 *
 * Takes no parameters and returns no values.
 */
void calibrateSensors()
{
    ledRed(true);
    ledYellow(true);
    display.clear();

    // Wait 1 second and then begin automatic sensor calibration
    // by rotating in place to sweep the sensors over the line
    delay(1000);

    // turn left
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++)
    {
        lineSensors.calibrate();
    }

    // turn all the way to the right
    Motors::setSpeeds(-(int16_t)CALIBRATION_SPEED, CALIBRATION_SPEED);
    for (int i = 0; i <= 80; i++)
    {
        lineSensors.calibrate();
    };

    // turn back to center
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++)
    {
        lineSensors.calibrate();
    }

    // stop
    Motors::setSpeeds(0, 0);
    ledRed(false);
    ledYellow(false);
}

/*
 * Simulates std::Optional
 *
 *  std::optional is a utility in C++17 that represents an object
 *  that may or may not contain a value.
 *
 *  It allows for more expressive code by indicating the presence
 *  or absence of a value without using pointers or special values.
 *
 *  Ideal for handling optional parameters or return values safely.
 */

template <typename T>
class Option
{
    bool result;
    T value;

public:
    // constructor with a valid value
    explicit Option(const T value)
    {
        this->result = true;
        this->value = value;
    }

    // empty value constructor
    explicit Option()
    {
        this->result = false;
    }

    // returns true if the value is absent
    bool is_absent() const { return !(this->result); }

    // return the values
    T get_value() const { return this->value; }
};

bool isBarcodeDetected(uint16_t lineSensorValues[NUM_SENSORS])
{
    return (lineSensorValues[ENCODER_SENSOR_LEFT] > LINE_THRESHOLD) && (lineSensorValues[ENCODER_SENSOR_RIGHT] > LINE_THRESHOLD);
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

typedef struct
{
    uint16_t LineSensorDetectionResult;
    bool IsBarcodeSensorDetected;

} SensorResults;

Option<SensorResults> detectLines()
{
    bool onLine = false;
    uint32_t avg = 0; // this is for the weighted total
    uint16_t sum = 0; // this is for the denominator, which is <= 64000
    static uint16_t lastPosition = 0;

    static uint16_t lineSensorValues[NUM_SENSORS];
    lineSensors.readCalibrated(lineSensorValues);

    for (uint8_t i = NUM_SENSORS_START; i <= NUM_SENSORS_END; i++)
    {
        const uint16_t value = lineSensorValues[i];

        // keep track of whether we see the line at all
        if (value > LINE_THRESHOLD)
        {
            onLine = true;
        }

        // only average in values that are above a noise threshold
        if (value > NOISE_THRESHOLD)
        {
            avg += static_cast<uint32_t>(value) * (i * 1000);
            sum += value;
        }
    }

    if (!onLine)
    {
        // if reached end (None of them see line)
        if (sum == 0)
        {
            return Option<SensorResults>();
        }

        // If it last read to the left of center, return 0.
        if (lastPosition < (NUM_SENSORS - 3) * 1000 / 2)
        {
            return Option<SensorResults>{{0, isBarcodeDetected(lineSensorValues)}};
        }
        // If it last read to the right of center, return the max.
        return Option<SensorResults>{{(NUM_SENSORS - 1) * 1000, isBarcodeDetected(lineSensorValues)}};
    }

    lastPosition = avg / sum;
    return Option<SensorResults>{{lastPosition, isBarcodeDetected(lineSensorValues)}};
}

bool follow(const Option<SensorResults> *optionalPosition)
{
    static int16_t lastError = 0;
    // Get the position of the line.

    if (optionalPosition->is_absent())
    {
        Motors::setSpeeds(0, 0);
        return false;
    }

    const int position = static_cast<int>(optionalPosition->get_value().LineSensorDetectionResult);

    // Our "error" is how far we are away from the center of the
    // line, which corresponds to position 2000.
    const int error = position - 2000;

    // Get motor speed difference using PROPORTIONAL_CONSTANT and derivative
    // PID terms (the integral term is generally not very useful
    // for line following).
    const int speedDifference = error * PROPORTIONAL_CONSTANT / 256 + (error - lastError) * DERIVATIVE_CONSTANT / 256;

    lastError = error;

    // Get individual motor speeds.  The sign of speedDifference
    // determines if the robot turns left or right.
    int leftSpeed = BASE_SPEED + speedDifference;
    int rightSpeed = BASE_SPEED - speedDifference;

    // Constrain our motor speeds to be between 0 and MAX_SPEED.
    // One motor will always be turning at MAX_SPEED, and the other
    // will be at MAX_SPEED-|speedDifference| if that is positive,
    // else it will be stationary.  For some applications, you
    // might want to allow the motor speed to go negative so that
    // it can spin in reverse.
    leftSpeed = constrain(leftSpeed, MIN_SPEED, (int16_t)MAX_SPEED);
    rightSpeed = constrain(rightSpeed, MIN_SPEED, (int16_t)MAX_SPEED);

    Motors::setSpeeds(leftSpeed, rightSpeed);

    return true;
}

typedef enum States
{
    START,
    NOT_DETECTED,
    DETECTED,
} State;

typedef class StateMachine
{

    State state = START;
    MilliSeconds t0;

    int barcodeResults[100];
    int len = 0;

public:
    StateMachine()
    {
    }

    State getState()
    {
        return state;
    }

    int *getResults()
    {
        return barcodeResults;
    }

    int getResultLen()
    {
        return len;
    }

    void push(State newState)
    {

        if (newState == state)
        {
            return;
        }

        switch (state)
        {
        case START:
            state = DETECTED;
            t0 = millis();
            break;
        case NOT_DETECTED:
            state = DETECTED;
            barcodeResults[len] = millis() - t0;
            len++;
            t0 = millis();
            break;
        case DETECTED:
            state = NOT_DETECTED;
            barcodeResults[len] = millis() - t0;
            len++;
            t0 = millis();
            break;
        }
    }

} BarCodeStateMachine;

void followAndReadBarCode(BarCodeStateMachine *stateMachine)
{
    for (;;)
    {
        const Option<SensorResults> results = detectLines();
        const bool onLine = follow(&results);
        if (!onLine)
        {
            return;
        }
        if (results.get_value().IsBarcodeSensorDetected)
        {
            stateMachine->push(DETECTED);
        }
        else
        {
            stateMachine->push(NOT_DETECTED);
        }
    }
}

/*
 * Sets up the display to be in the proper formatting
 */

void setup()
{
    display.setLayout21x8();
    lineSensors.setTimeout(LINE_SENSOR_TIMEOUT);
}

void loop()
{
    /*
     *
     *  Welcome Screen
     *
     */
    display_centered("Abdul Mannan Syed", 0);
    display_centered("Nathan Gratton", 1);
    display_centered("Lab 4: Barcode", 4);
    display_centered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();

    /*
     *
     *  Calibrate
     *
     */

    calibrateSensors();
    display_centered("Ready", 1);
    display_centered("<  GO  >", 4);
    buttonB.waitForButton();

    /*
     *
     * Follow guideline And Measure it
     *
     */
    display_centered("GO!!!", 4);
    PLAY_GO(buzzer);

    /*
     *
     * Follow Line
     *
     * Read & Store Detections
     *
     */

    BarCodeStateMachine stateMachine = BarCodeStateMachine();
    followAndReadBarCode(&stateMachine);
    PLAY_BEEP(buzzer);
    display_centered("Values:" + String(stateMachine.getResultLen()), 4);
    buttonB.waitForButton();
}
