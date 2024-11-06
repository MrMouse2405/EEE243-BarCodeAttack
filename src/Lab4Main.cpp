#include "Lab4.h"
#include "LineFollowing.h"
#include "BarCodeReading.h"

using namespace LineFollowing;
using namespace BarCodeReading;
using namespace Pololu3piPlus32U4;

OLED display;
Buzzer buzzer;
ButtonB buttonB;

// line following algorithm
LineFollower driver;
// scanner with capacity of 20 characters
BarCodeReader<20> scanner;

void playGo()
{
    buzzer.play("L16 cdegreg4");
    while (buzzer.isPlaying())
    {
    }
}

void playBeep()
{
    buzzer.play(">g32>>c32");
    while (buzzer.isPlaying())
    {
    }
}

void display_centered(const String &s, const uint8_t line)
{
    // 10 is half of 21 (see function setup)
    display.gotoXY(10 - (s.length() / 2), line);
    display.print(s.c_str());
}

void setup()
{
    // init
    display.setLayout21x8();

    // Welcome Screen
    display_centered("Abdul Mannan Syed", 0);
    display_centered("Nathan Gratton", 1);
    display_centered("Lab 4: Barcode", 4);
    display_centered("To start, press B", 7);
    buttonB.waitForButton();
    display.clear();

    // Now calibrate
    driver.pushAction(LineFollowingActions::Calibrate);
}

void loop()
{

    // Ask to start
    display_centered("Ready", 1);
    display_centered("<  GO  >", 4);
    buttonB.waitForButton();
    display.clear();

    // Start Following Line
    driver.pushAction(LineFollowingActions::Start);
    while (driver.getState() == LineFollowingStates::Following)
    {
        driver.follow();
    }
}