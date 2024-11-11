#pragma once
#include "Lab4.h"

namespace Parser {
    struct KNNPoint {
        int64_t distance;
        const Lab4::Bar* bar;
    };

    class KNNParser {
        Lab4::Bar trainingData[WIDTH_CHARACTER_SIZE] = {};
        KNNPoint points[WIDTH_CHARACTER_SIZE] = {};
    public:

        void train(const Lab4::Buffer<Lab4::Bar,WIDTH_CHARACTER_SIZE> *calibrationBatch);
        Lab4::BarType getBarType(const Lab4::Bar *bar);

        static Lab4::Option<char> lex(const Lab4::Buffer<Lab4::BarType,WIDTH_CHARACTER_SIZE>& code);
    };
}
