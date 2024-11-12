#pragma once
#include "Lab4.h"

/**
 * Parser
 *
 * Defines the KNNParser class for parsing and classifying barcode values
 * using the K-Nearest Neighbor algorithm. Supports training with calibration
 * data and batch predictions as code39 characters.
 *
 * Date: 2024-11-11
 *
 */

namespace Parser {
    /*
     * KNNParser
     *
     * Class for parsing barcode values using the K-Nearest Neighbor classifier.
     * It trains the model with a calibration batch to accurately classify
     * barcode values as Narrow or Wide. Once calibrated, it can also predict
     * decode batches of values as code39 characters.
     */

    class KNNParser {
    public:
        /*
         * Trains the KNN model using 9 labeled barcode width values from a calibration batch.
         * Assumes the batch to be code39 character set
         */
        void train(const Lab4::Buffer<Lab4::Bar, WIDTH_CHARACTER_SIZE> *calibrationBatch);

        /*
         * Predicts whether a given barcode width is Narrow or Wide based on calibration data.
         */
        Lab4::BarType getBarType(const Lab4::Bar *bar);

        /*
         * Decodes a Code39 character from a sequence of Narrow/Wide values.
         * Returns an Option<char>; empty if the sequence does not conform to Code39 specifications.
         */
        static Lab4::Option<char> lex(const Lab4::Buffer<Lab4::BarType, WIDTH_CHARACTER_SIZE> &code);

    private:
        /*
         * Structure representing a point in the KNN algorithm.
         * Contains the Euclidean distance and a pointer to the corresponding Bar.
         */
        struct KNNPoint {
            int64_t distance;
            const Lab4::Bar *bar;
        };

        /*
         * Array to hold training data bars for KNN classification.
         * Initialized to a default state.
         */

        Lab4::Bar trainingData[WIDTH_CHARACTER_SIZE] = {};

        /*
         * Array to store KNNPoint instances for distance calculations
         * and neighbor classification during the KNN algorithm.
         */

        KNNPoint points[WIDTH_CHARACTER_SIZE] = {};

        /*
         * Classifies a bar using the k-nearest neighbors algorithm.
         * Assumes two groups: Narrow and Wide. Returns Narrow if
         * the majority of the k nearest neighbors belong to Narrow;
         * otherwise, returns Wide.
         */
        static Lab4::BarType KNearestClassifier(const Lab4::Bar *bar, const int k,
                                                KNNPoint points[WIDTH_CHARACTER_SIZE]);

        /*
         * Sorts an array of KNNPoint using the quicksort algorithm.
         * This function organizes the points based on their distances
         * to facilitate efficient neighbor classification.
         */
        static void quickSort(KNNPoint arr[], int low, int high);

        /*
         * Partitions the array of KNNPoint elements for quicksort.
         * Rearranges the elements so that those less than a pivot
         * come before it, and those greater come after.
         */
        static int partition(KNNPoint arr[], int low, int high);

        /*
         * Swaps two elements of type T.
         * This function is used in the quicksort algorithm to exchange
         * positions of elements in the array.
         */
        template<typename T>
        static void swap(T *a, T *b);
    };
}
