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

#pragma once

namespace LineFollowing
{
    typedef enum LFStates
    {
        // Robot is ready for action
        Initialized,
        // Robot is calibrating IR sensors
        // No actions will be accepted
        Calibrating,
        // Robot is calibrated and ready
        Ready,
        // Currently following the line
        Following,
        // Stopped by caller
        ForcedStop,
        // Robot has reached the end of line
        ReachedEnd,
    } LineFollowingStates;

    class LineFollower
    {
        // Tracks the state of Line Follower
        LineFollowingStates state = Initialized;
        void followLine();

    public:
        /**
         * This should be called in a loop while the robot
         * is allowed to move
         *
         */
        void follow();
        /**
         *
         *  Push user defined actions
         *
         *  i.e give commands to start / stop
         *  the algorithm
         *
         */
        void start();
        void stop();
        void calibrate();
        /**
         *
         *  Get the current state of algorithm
         *
         */
        LineFollowingStates getState() const;
    };
}