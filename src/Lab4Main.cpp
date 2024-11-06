#include "Lab4.h"
#include "LineFollowing.h"
#include "BarCodeReading.h"

using namespace LineFollowing;
using namespace BarCodeReading;

// line following algorithm
LineFollower driver;
// scanner with capacity of 20 characters
BarCodeReader<20> scanner;

void setup() {
    Lab4::setup();
    Lab4::welcomeScreen();

    // Now calibrate
    driver.pushAction(LineFollowingActions::Calibrate);
}


void loop() {

    // Ask to start
    Lab4::display_centered("Ready", 1);
    Lab4::display_centered("<  GO  >", 4);
    Lab4::buttonB.waitForButton();

    // Start Following Line
    driver.pushAction(LineFollowingActions::Start);
    while (driver.getState() == LineFollowingStates::Following) {
        driver.follow();
    }


}