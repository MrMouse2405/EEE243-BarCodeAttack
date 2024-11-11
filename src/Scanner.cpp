#include "Scanner.h"
#include "Sensors.h"
#include "Parser.h"

Scanner::Scanner() {
    this->state = WHITE;
    this->t0 = millis();
}

Lab4::Option<Lab4::Bar> Scanner::scan() {

    uint64_t t1;
    uint64_t delta;

    switch(this->state) {
        case WHITE: {
            if (Sensors::isBarcodeDetected()) {
                this->state = BLACK;
            }
            uint64_t t1 = millis();
            uint64_t delta = t1 - t0;
            this->t0 = t1;
            return Lab4::Option<Lab4::Bar>({delta,Lab4::BarType::Null});
        }
        case BLACK: {
            if (!Sensors::isBarcodeDetected()) {
                this->state = BLACK;
            }
            uint64_t t1 = millis();
            uint64_t delta = t1 - t0;
            this->t0 = t1;
            return Lab4::Option<Lab4::Bar>({delta,Lab4::BarType::Null});
        }
    }

    return Lab4::Option<Lab4::Bar>();
}
