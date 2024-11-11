#pragma once
#include "Lab4.h"

namespace Parser {

    struct KNNPoint {
        Lab4::BarType klass;
        int value;
        int distance;
    };

    class KNNParser {
        struct KNNPoint trainingData[WIDTH_CHARACTER_SIZE];
        public:
            KNNParser(Lab4::Batch* calibrationBatch);
            Lab4::BarType getBarType(Lab4::Bar* bar);
    };

}