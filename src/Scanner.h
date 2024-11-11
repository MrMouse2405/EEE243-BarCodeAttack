#pragma once
#include "Lab4.h"
#include "Arduino.h"

class Scanner {
    public:
        Scanner();
        // returns a new value if a new value is detected
        Lab4::Option<Lab4::Bar> scan();
    private:
        typedef enum {
           WHITE,
           BLACK
        } ReadingState;
        ReadingState state;
        uint64_t t0;
};