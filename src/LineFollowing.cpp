/**
 * LineFollower
 *
 * Responsible for following black
 * line as sensed by the IR sensors.
 *
 * Author: OCdt Syed
 * Date: 2024-11-01
 *
 */
#include <Pololu3piPlus32U4.h>
#include "LineFollowing.h"
#include "Sensors.h"

using namespace LineFollowing;

/**
 *
 * Algorithm for following the line
 *
 * sets state to 'ReachedEnd' if the
 * IR sensors no longer detect the line.
 *
 * used PID controller for moving the robot
 *
 */
void LineFollower::followLine() {
    // Get the position of the line.
    static int16_t lastError = 0;
    // Get IR sensor results
    int64_t position = 0;
    // Check if a line is detected
    {
        // optionalPosition will be None if it's not detected
        Lab4::Option<int> optionalPositon = Sensors::detectLines();
        switch (optionalPositon.checkState()) {
            case Lab4::ResultState::None: {
                Pololu3piPlus32U4::Motors::setSpeeds(0, 0);
                this->state = ReachedEnd;
                return;
            }
            case Lab4::ResultState::Some: {
                // we have a value, continue
                position = optionalPositon.getValue();
            }
        }
    }
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
    Pololu3piPlus32U4::Motors::setSpeeds(leftSpeed, rightSpeed);
}

/**
 * This should be called in a loop while the robot
 * is allowed to move
 *
 */
void LineFollower::follow() {
    switch (this->state) {
        case Initialized: {
            // do nothing
            break;
        }
        case Calibrating: {
            Sensors::calibrateSensors();
            this->state = Ready;
            break;
        }
        case Ready: {
            // do nothing
            break;
        }
        case Following: {
            this->followLine();
            break;
        }
        case ForcedStop:
        case ReachedEnd: {
            Pololu3piPlus32U4::Motors::setSpeeds(0, 0);
            break;
        }
    }
}

/**
 *
 *  Push user defined actions
 *
 *  i.e give commands to start / stop
 *  the algorithm
 *
 */
void LineFollower::start() {
    switch (this->state) {
        case Initialized:
        case Calibrating: {
            break;
        }
        default: {
            this->state = Following;
            break;
        }
    };
}

void LineFollower::stop() {
    Pololu3piPlus32U4::Motors::setSpeeds(0, 0);
    switch (this->state) {
        case Calibrating:
        case ReachedEnd: {
            break;
        }
        default: {
            this->state = ForcedStop;
            break;
        }
    }
}

void LineFollower::calibrate() {
    switch (this->state) {
        case Initialized:
        case ReachedEnd: {
            this->state = Calibrating;
            break;
        }
        default: {
            break;
        }
    }
}

/**
 *
 *  Get the current state of algorithm
 *
 */
LineFollowingStates LineFollower::getState() const {
    return this->state;
}
