/*
* The robot will follow black lines on the floor using its
* sensors while displaying "GO!!!". Once it reaches the end,
* it will turn around and follow the track again.
*
* Author: OCdt Syed and OCdt Libreiro
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
#define NUM_SENSORS_END  3 // sensor 4

// Encoder Sensors
#define ENCODER_SENSOR_RIGHT 4 // sensor 5
#define ENCODER_SENSOR_LEFT  0 // sensor 1

// time between each IR sample
#define LINE_SENSOR_TIMEOUT 2000

// Values below this will be ignored
#define NOISE_THRESHOLD 50

// Values below this will be considered to
// be not on black line.
#define LINE_THRESHOLD  200


// Maximum & Minimum speed the motors will be allowed to turn.
#define MAX_SPEED 125
#define MIN_SPEED 0


// Speed the motors will run when centered on the line.
#define BASE_SPEED MAX_SPEED

// Speed of motors while calibration
#define CALIBRATION_SPEED 150

// Choices for allowing users to choose what distance to follow.
#define RECKON_VALUES_IN_CM {10,30,60,100,200}

// define our cm unit
typedef int counts_per_cm;
typedef int cm;

/*
 * PID Constants
 *
 * This configuration uses a default proportional constant of 1/4
 * and a derivative constant of 1, which appears to perform well at low speeds.
 * Note: Adapted from Pololu3piplus documentation.
*/
#define PROPORTIONAL_CONSTANT 64 // coefficient of the P term * 256
#define DERIVATIVE_CONSTANT 256 // coefficient of the D term * 256

/*
 *
 * Buzzer Constants
 *
 */

#define PLAY_GO(buzzer)   buzzer.play("L16 cdegreg4"); while (buzzer.isPlaying());
#define PLAY_BEEP(buzzer) buzzer.play(">g32>>c32"); while (buzzer.isPlaying());


/*
* Displays a string centered along the y-axis of the display.
*
* string: the string to be displayed
* line: which number line to be displayed on. (0 <= line < 8)
*/

void display_centered(const String &s, const uint8_t line) {
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
void calibrateSensors() {
    ledRed(true);
    ledYellow(true);
    display.clear();

    // Wait 1 second and then begin automatic sensor calibration
    // by rotating in place to sweep the sensors over the line
    delay(1000);

    // turn left
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++) { lineSensors.calibrate(); }

    // turn all the way to the right
    Motors::setSpeeds(-(int16_t) CALIBRATION_SPEED, CALIBRATION_SPEED);
    for (int i = 0; i <= 80; i++) { lineSensors.calibrate(); };

    // turn back to center
    Motors::setSpeeds(CALIBRATION_SPEED, -CALIBRATION_SPEED);
    for (int i = 0; i <= 40; i++) { lineSensors.calibrate(); }

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

template<typename T>
class Option {
private:
    bool result;
    T value;

public:
    // constructor with a valid value
    explicit Option(const T value) {
        this->result = true;
        this->value = value;
    }

    // empty value constructor
    explicit Option() {
        this->result = false;
    }

    // returns true if the value is absent
    bool is_absent() const { return !(this->result); }

    // return the values
    T get_value() const { return this->value; }
};


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

Option<uint16_t> readLineBlack() {
    bool onLine = false;
    uint32_t avg = 0; // this is for the weighted total
    uint16_t sum = 0; // this is for the denominator, which is <= 64000
    static uint16_t lastPosition = 0;

    static uint16_t lineSensorValues[NUM_SENSORS];
    lineSensors.readCalibrated(lineSensorValues);

    for (uint8_t i = NUM_SENSORS_START; i <= NUM_SENSORS_END; i++) {
        const uint16_t value = lineSensorValues[i];

        // keep track of whether we see the line at all
        if (value > LINE_THRESHOLD) { onLine = true; }


        // only average in values that are above a noise threshold
        if (value > NOISE_THRESHOLD) {
            avg += (uint32_t) value * (i * 1000);
            sum += value;
        }
    }


    if (!onLine) {
        // if reached end (None of them see line)
        if (sum == 0) {
            return Option<uint16_t>();
        }


        // If it last read to the left of center, return 0.
        if (lastPosition < (NUM_SENSORS - 3) * 1000 / 2) {
            return Option<uint16_t>{0};
        }
        // If it last read to the right of center, return the max.
        return Option<uint16_t>{(NUM_SENSORS - 1) * 1000};
    }


    lastPosition = avg / sum;
    return Option<uint16_t>{lastPosition};
}


/*
* Checks if at least one of the three IR sensors is on the line
*/

bool isOnLine() {
    static uint16_t lineSensorValues[NUM_SENSORS];
    lineSensors.readCalibrated(lineSensorValues);


    for (uint8_t i = NUM_SENSORS_START; i <= NUM_SENSORS_END; i++) {
        if (lineSensorValues[i] > LINE_THRESHOLD) {
            return true;
        }
    }


    return false;
}

/*
 * Checks if both encoder sensors
 * are currently detecting an encoder line.
 *
 * Returns true if BOTH are currently detecting a line
 * Otherwise returns false
 *
 */

bool isOnEncoderLine() {
    static uint16_t lineSensorValues[NUM_SENSORS];
    lineSensors.readCalibrated(lineSensorValues);

    return (lineSensorValues[ENCODER_SENSOR_LEFT] > LINE_THRESHOLD) && (
               lineSensorValues[ENCODER_SENSOR_RIGHT] > LINE_THRESHOLD);
}

/*
 *
 * Adjusts the motors to move in the direction of line.
 *
 * This function returns:
 *  If robot still detects black line to follow: true
 *  If robot does not detect black line to follow: false
 *
 */

bool follow() {
    static int16_t lastError = 0;
    // Get the position of the line.
    const Option<uint16_t> optionalPosition = readLineBlack();

    if (optionalPosition.is_absent()) {
        Motors::setSpeeds(0, 0);
        return false;
    }

    const int position = static_cast<int>(optionalPosition.get_value());

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

/*
 * This functions uses a guideline to measure distances.
 *
 * This function will return Option<GuideLineResults> value
 * after successfully measuring a guideline.
 *
 * The values are represented in counts_per_cm
 *
 * Option<GuideLineResults> will have an absent value if there
 * was an error occurred during measuring.
 */
struct GuideLineResults {
    counts_per_cm leftResult;
    counts_per_cm rightResult;
};

Option<GuideLineResults> measureGuideLine() {
    enum state {
        ZERO,
        LINE_ONE,
        LINE_TWO
    };

    bool already_on_line = false;
    state state = ZERO;

    // follow line
    for (;;) {
        const bool onLine = follow();
        const bool onEncoderLine = isOnEncoderLine();

        if (onEncoderLine && !already_on_line) {
            already_on_line = true;
            switch (state) {
                // once reached the first edge of encoder line
                // reset encoders, to start tracking.
                case ZERO:
                    state = LINE_ONE;
                // reset our encoders
                    Encoders::getCountsAndResetLeft();
                    Encoders::getCountsAndResetRight();
                    break;
                case LINE_ONE:
                    state = LINE_TWO;
                    break;
                default:
                    break;
            }
        }
        // if we have reached the second edge of first line
        // reset it so that we can detect first edge of second line
        else if (!onEncoderLine && already_on_line) {
            already_on_line = false;
        }

        // if we have reached the first edge of second line
        // we stop
        if (state == LINE_TWO) {
            Motors::setSpeeds(0, 0);
            // this is because we are measuring guideline for 120 cm
            return Option<GuideLineResults>{
                {
                    Encoders::getCountsAndResetLeft() / 120,
                    Encoders::getCountsAndResetRight() / 120,
                }
            };
        }

        // if robot somehow gets off the guideline
        // it will stop and beep.
        if (!onLine) {
            // reached end beep
            PLAY_BEEP(buzzer)
            Motors::setSpeeds(0, 0);
            return Option<GuideLineResults>{};
        }
    }

    return Option<GuideLineResults>{};
}

/*
 * Gives user the choice to select the distance
 * they wish for the robot to travel in cms.
 *
 * This function will return the chosen value in
 * cms.
 *
 * Note, in cms, NOT counts_per_cm
 *
 */

cm getUserDistanceChoice() {
    static const cm cms[] = RECKON_VALUES_IN_CM;
    static constexpr int cmsLen = sizeof(cms) / sizeof (cms[0]);

    int choice = 0;
    // until button B is pressed
    while (!buttonB.isPressed()) {
        // if A is pressed, cycle backward, constrain to zero
        if (buttonA.isPressed()) {
            choice = choice - 1;
            if (choice < 0) {
                choice = 0;
            }
        }

        // if C is pressed, cycle forward, constrain to arraylen - 1
        if (buttonC.isPressed()) {
            choice = choice + 1;
            if (choice >= cmsLen) {
                choice = cmsLen - 1;
            }
        }

        display.clear();
        // display choice
        display_centered("Ready", 0);
        display_centered("<   GO   >", 4);
        display_centered(String(cms[choice]), 7);
        // delay is needed because machines
        // work faster than human reaction time
        delay(1000);
    }

    return cms[choice];
}

/*
* Sets up the display to be in the proper formatting
*/

void setup() {
    display.setLayout21x8();
    lineSensors.setTimeout(LINE_SENSOR_TIMEOUT);
}


void loop() {
    /*
     *
     *  Welcome Screen
     *
     */
    display_centered("Abdul Mannan Syed", 0);
    display_centered("Emma Libreiro", 1);
    display_centered("Lab 3-4: Reckon", 4);
    display_centered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();


    /*
     *
     *  Calibrate
     *
     */

    calibrateSensors();

    /*
     *
     * Display & Wait for user to start the
     * distance tracking algorithm
     *
     */
DISTANCE_TRACKING:
    display_centered("Ready to measure", 4);
    display_centered("To start, press B", 5);
    buttonB.waitForButton();
    display.clear();

    /*
     *
     * Follow guideline And Measure it
     *
     */
    display_centered("GO!!!", 4);
    PLAY_GO(buzzer);

    Option<GuideLineResults> result = measureGuideLine();

    if (result.is_absent()) {
        display.clear();
        display_centered("ERROR!!!", 4);
        display_centered("Press B To Restart", 5);
        buttonB.waitForButton();
        goto DISTANCE_TRACKING;
    }


    /*
     *
     * Now ask user to travel a distance,
     * travel that distance
     *
     */

    // These are our calculated are encoders/cm
    const counts_per_cm leftDistance = result.get_value().leftResult;
    const counts_per_cm rightDistance = result.get_value().rightResult;

    for (;;) {
        const cm distanceToTravel = getUserDistanceChoice();

        // reset encoders
        Encoders::getCountsAndResetLeft();
        Encoders::getCountsAndResetRight();

        const counts_per_cm leftGoal = leftDistance * distanceToTravel;
        const counts_per_cm rightGoal = rightDistance * distanceToTravel;

        // travel that target distance
        Motors::setSpeeds(MAX_SPEED,MAX_SPEED);
        while (leftGoal > Encoders::getCountsLeft() && rightGoal > Encoders::getCountsRight());
        Motors::setSpeeds(0, 0);
    }
}
