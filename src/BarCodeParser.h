/** 
 * BarCodeParser
 * 
 * Responsible for parsing values obtained from the 
 * environment via the IR sensors into code39 character set.
 * 
 * Author: OCdt Gratton
 * Date: 2024-11-04
*/


#pragma once
#include "Lab4.h"
#include <Arduino.h>

namespace BarCodeParser {
    /*
    * Takes a string of 9 Ns and Ws and returns the corresponding character.
    * 
    * Input: barWidths - a string of 9 Ns and Ws
    * 
    * Returns: the corresponding character
    */
    Lab4::Option<char> widthStringToCharacter(const char *barWidths);

    /*
    * Takes the calibration array of 9 unsigned longs and sets up the calibration values
    * Returns true if calibration was successful, false otherwise.
    * 
    * Input: calibrationArray - an array of 9 unsigned longs
    * 
    * Returns: true if calibration was successful, false otherwise
    */
    bool widthCalibrate(const uint64_t calibrationArray[WIDTH_CHARACTER_SIZE]);

    /*
    * Takes an array of 9 unsigned longs and converts it to a string of Ns and Ws
    * using the previously set calibration values.
    * 
    * Input: timeArray - an array of 9 unsigned longs
    * 
    * Returns: a string of Ns and Ws
    */
    char *convertToBarWidths(const uint64_t timeArray[WIDTH_CHARACTER_SIZE]);

    /*
    * Takes an array of 9 unsigned longs and returns the corresponding character
    * using the previously set calibration values.
    * 
    * Input: timeArray - an array of 9 unsigned longs
    * 
    * Returns: the corresponding character
    */
    Lab4::Option<char> processTimeArray(const uint64_t timeArray[WIDTH_CHARACTER_SIZE]);
}
