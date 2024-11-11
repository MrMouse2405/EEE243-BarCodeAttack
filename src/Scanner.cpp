#include "Scanner.h"
#include "Sensors.h"
#include "Parser.h"

Scanner::Scanner() {
    this->state = WHITE;
    this->t0 = millis();
}

Scanner::Scanner(const uint64_t startTime) {
    this->state = WHITE;
    this->t0 = startTime;
}

Lab4::Option<Lab4::Bar> Scanner::scan() {
    const bool blackDetected = Sensors::isBarcodeDetected();
    switch (this->state) {
        case WHITE: {
            if (blackDetected) {
                this->state = BLACK;
                const uint64_t t1 = millis();
                const uint64_t delta = t1 - t0;
                this->t0 = t1;
                return Lab4::Option<Lab4::Bar>({delta, Lab4::BarType::Null});
            }
            break;
        }
        case BLACK: {
            if (!blackDetected) {
                this->state = WHITE;
                const uint64_t t1 = millis();
                const uint64_t delta = t1 - t0;
                this->t0 = t1;
                return Lab4::Option<Lab4::Bar>({delta, Lab4::BarType::Null});
            }
            break;
        }
    }

    return {};
}
