/**
 * BarCodeParser
 *
 * Responsible for parsing values obtained from the
 * environment via the IR sensors into code39 character set.
 *
 * Author: OCdt Gratton
 * Date: 2024-11-04
 */
#include "BarCodeParser.h"
#include "code39.h"

namespace BarCodeParser {
    static uint64_t g_narrowValue = 0; // TODO: Rename to standard
    static uint64_t g_wideValue = 0; // TODO: Move to top
}

/*
 * Takes a string of 9 Ns and Ws and returns the corresponding character.
 *
 * Input: barWidths - a string of 9 Ns and Ws
 *
 * Returns: the corresponding character
 */
Lab4::Option<char> BarCodeParser::widthStringToCharacter(const char *barWidths) {
    for (auto i: code39) {
        // Set string length in code39.h is 9 excluding the first character
        if (strncmp(barWidths, i + 1, WIDTH_CHARACTER_SIZE) == 0) {
            return Lab4::Option<char>(i[0]);
        }
    }
    return {}; // Default case (no match found)
}

/*
 * Takes the calibration array of 9 unsigned longs and sets up the calibration values
 * Returns true if calibration was successful, false otherwise.
 *
 * Input: calibrationArray - an array of 9 unsigned longs
 *
 * Returns: true if calibration was successful, false otherwise
 */
bool BarCodeParser::widthCalibrate(const uint64_t calibrationArray[]) {
    g_narrowValue = calibrationArray[0];
    g_wideValue = 0;

    // Find the first significantly different reading for calibration
    for (int i = 1; i < 9; i++) {
        if (calibrationArray[i] > g_narrowValue * (100 + SIZE_BUFFER) / 100 ||
            calibrationArray[i] < g_narrowValue * (100 - SIZE_BUFFER) / 100) {
            g_wideValue = calibrationArray[i];
            break;
        }
    }

    return g_wideValue != 0;
}

/*
 * Takes an array of 9 unsigned longs and converts it to a string of Ns and Ws
 * using the previously set calibration values.
 *
 * Input: timeArray - an array of 9 unsigned longs
 *
 * Returns: a string of Ns and Ws
 */
char *BarCodeParser::convertToBarWidths(const uint64_t timeArray[]) {
    if (g_narrowValue == 0 || g_wideValue == 0) {
        return nullptr; // Calibration hasn't been done
    }

    const auto barWidths = static_cast<char *>(malloc(10));

    for (int i = 0; i < 9; i++) {
        const bool isCloserToNarrow =
                abs((long)(timeArray[i] - g_narrowValue)) < abs((long)(timeArray[i] - g_wideValue));
        barWidths[i] = isCloserToNarrow ? 'N' : 'W';
    }
    barWidths[9] = '\0';

    return barWidths;
}

/*
 * Takes an array of 9 unsigned longs and returns the corresponding character
 * using the previously set calibration values.
 *
 * Input: timeArray - an array of 9 unsigned longs
 *
 * Returns: the corresponding character
 */
Lab4::Option<char> BarCodeParser::processTimeArray(const uint64_t timeArray[]) {
    char *barWidths = convertToBarWidths(timeArray);
    if (barWidths == nullptr) {
        return {};
    }

    int wCount = 0;
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        if (barWidths[i] == 'W') {
            wCount++;
        }
    }
    if (wCount != 3) {
        free(barWidths);
        return {}; // TODO: Put a character that represents a too many or too few Wides error
    }

    char result = 0; {
        Lab4::Option<char> optionalResult = widthStringToCharacter(barWidths);
        free(barWidths);
        switch (optionalResult.checkState()) {
            case Lab4::ResultState::None: {
                return {};
            }
            case Lab4::ResultState::Some: {
                result = optionalResult.getValue();
                break;
            }
        }
    }

    free(barWidths);
    return Lab4::Option<char>(result);
}
